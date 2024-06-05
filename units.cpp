#include "customgui.h"
#include "units.h"


unordered_map<glm::vec3, int> getCellsUpToDist(unordered_map<glm::vec3, int> pathable, int dist)
{
    unordered_map<glm::vec3, int> cells;
    for (const auto& pair : pathable)
    {
        if (pair.second <= dist)
        {
            cells.insert(pair);
        }
    }
    return cells;
}

Unit::Unit() = default;

Unit::Unit(glm::vec3 hexCellCords, Model model, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
    : stats{5, 5, 3, 2, 1, 0, 4, 0, 2, 0},
      guiHotBar(ImVec2(Globals::windowW, Globals::windowH), &stats, &abilitiesList, &selectedAbil), hexGrid(HexGrid),
      guiHealthBar(ImVec2(Globals::windowW, Globals::windowH), &stats, &pos, objID)
{
    for(auto &it : abilitiesList)
    {
        it = -1;
    }
    properHeight = 1.507f;
    abilitiesList[0] = 0;
    abilitiesRanges[0] = 1;
    this->ID = objID;
    this->owner = factionID;
    hexPos = hexCellCords;
    this->model = model;
    pos = getWorldPosFromHex(hexPos);
    pos.y = properHeight;
    if(stats.flying){
        (*HexGrid)[hexPos].airID = this->ID;
        (*HexGrid)[hexPos].occupiedAir = true;
    }else{
        (*HexGrid)[hexPos].groundID = this->ID;
        (*HexGrid)[hexPos].occupiedGround = true;
    }
    transformMat = glm::translate(glm::mat4(1.f), pos);
    boundingBox = Cube(0.21f, 0.21f, 0.21f, pos);
}

void Unit::commandLC(Selectable *target, abilityCall *orderInfo)
{
    useAbilOnUnit(target,orderInfo);
    useAbilOnHex(target,orderInfo);
    Hexagon* isHex = dynamic_cast<Hexagon*>(target);
    if(isHex != nullptr)
    {
        glm::vec3 targetPos = getWorldPosFromHex(isHex->cell.LogicPos);
        rotateTowardsHex(targetPos);
    }

    Unit* isUnit = dynamic_cast<Unit*>(target);
    if(isUnit != nullptr)
    {
        glm::vec3 targetPos = getWorldPosFromHex(isUnit->hexPos);
        rotateTowardsHex(targetPos);
    }
    //lol
}

void Unit::commandRC(Selectable *target, abilityCall *orderInfo)
{
    tryMoving(target,orderInfo);
    glm::vec3 targetPos = getWorldPosFromHex(hexPos);
    rotateTowardsHex(targetPos);
    pos = targetPos;
    pos.y = properHeight;
    transformMat[3] = glm::vec4(pos,1.0f);
}

void Unit::onSelect()
{
    isSelected = 1;
    updateMovRange();
    updateAbilRange();
}

void Unit::onHover()
{
    hovering = 1;
}

void Unit::update(double deltaTime)
{
    boundingBox.model = glm::translate(glm::mat4(1.f), pos);
    updateMovRange();
    updateAbilRange();
    guiHealthBar.update(deltaTime);
    if(!isSelected)
        selectedAbil = -1;
}

void Unit::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    if(isSelected && selectedAbil == -1)
    {
        for(auto& pair : reachableHexes)
        {
            (*hexGrid)[pair.first].moveRangeView = true;
        }
    }

    if(isSelected && selectedAbil != -1)
    {
        for(auto& pair : inAbilityRange)
        {
            (*hexGrid)[pair.first].abilityRangeView = true;
        }
    }

    if(isSelected && shaderProgram != shaderPrograms[2])
        guiHotBar.render(shaderProgram, shaderPrograms);
    if(shaderProgram != shaderPrograms[2])
        guiHealthBar.render(shaderProgram, shaderPrograms);

    if((hovering || isSelected) && shaderProgram != shaderPrograms[2]){
        glUniform1i(glGetUniformLocation(shaderProgram, "factionID"), owner-1);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
    }

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transformMat));

    model.Draw(shaderProgram,true);
    //boundingBox.Draw(shaderProgram, hovering || isSelected);

    if((hovering || isSelected) && shaderProgram != shaderPrograms[2]){
        glUniform1i(glGetUniformLocation(shaderProgram, "factionHighlight"), true);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glm::mat4 modelScaled = glm::scale(transformMat,glm::vec3(1.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelScaled));
        model.Draw(shaderProgram,false);

        glDepthFunc(GL_LESS);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);


        glUniform1i(glGetUniformLocation(shaderProgram, "factionHighlight"), false);
    }
}

void Unit::updateMovRange(int malus)
{
    reachableHexes.clear();
    if(stats.flying)
        reachableHexes = getCellsUpToDist(getPathableCellsAir(*hexGrid, hexPos), std::max(0, stats.movRange - malus - stats.effects[SLOW].intensity));
    else
        reachableHexes = getCellsUpToDist(getPathableCellsGround(*hexGrid, hexPos), std::max(0, stats.movRange - malus - stats.effects[SLOW].intensity));
}

void Unit::rotateTowardsHex(glm::vec3 theHex){
    if(stats.isBuilding)
        return;
    glm::vec3 ourPos = glm::vec3(pos.x,pos.y,pos.z);
    glm::vec3 hexPos = glm::vec3(theHex.x,pos.y,theHex.z);
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transformMat, scale, rotation, translation, skew, perspective);

    glm::mat3 newRotation = glm::mat3(glm::inverse(glm::lookAt(ourPos,hexPos,glm::vec3(0.f,1.f,0.f))));
    if(ourPos != hexPos)
        this->transformMat = glm::translate(glm::mat4(1.f),translation) * glm::mat4(newRotation) * glm::scale(glm::mat4(1.f),scale);
}

void Unit::updateAbilRange(int malus)
{
    inAbilityRange.clear();
    if(selectedAbil != -1)
        inAbilityRange = getCellsUpToDist(getPathableCellsRange(*hexGrid, hexPos), std::max(0, abilitiesRanges[selectedAbil]));
}

void Unit::useAbilOnUnit(Selectable *target, abilityCall *orderInfo)
{
    Unit* isUnit = dynamic_cast<Unit*>(target);
    if(isUnit != nullptr && inAbilityRange.find(isUnit->hexPos) != inAbilityRange.end())
    {
        orderInfo->abilityID = selectedAbil;
        orderInfo->culprit = this;
        ///powinienes sprawdzic raczej czy hexpos w tym unitcie jest poprawny
        for(auto it : abilityEffects[selectedAbil]){
            orderInfo->effects[it.first] = it.second;
        }
        orderInfo->target.push_back(&(*isUnit->hexGrid)[isUnit->hexPos]);
    }
}

void Unit::useAbilOnHex(Selectable *target, abilityCall *orderInfo)
{
    Hexagon* isHex = dynamic_cast<Hexagon*>(target);
    if(isHex != nullptr && inAbilityRange.find(isHex->cell.LogicPos) != inAbilityRange.end())
    {
        ///prawdopodobnie bedziesz chcial ztargetowac terrain/building jesli klikasz na hex'a
        orderInfo->abilityID = selectedAbil;
        orderInfo->culprit = this;
        orderInfo->target.push_back(&isHex->cell);
    }
}

void Unit::tryMoving(Selectable *target, abilityCall *orderInfo)
{

    Hexagon* isHex = dynamic_cast<Hexagon*>(target);
    if(isHex != nullptr && reachableHexes.find(isHex->cell.LogicPos) != reachableHexes.end() && selectedAbil == -1)
    {
        if(stats.flying){
            (*hexGrid)[this->hexPos].airID = -1;
            (*hexGrid)[this->hexPos].occupiedAir = false;
        }else{
            (*hexGrid)[this->hexPos].groundID = -1;
            (*hexGrid)[this->hexPos].occupiedGround = false;
        }
        this->hexPos = isHex->cell.LogicPos;
        this->stats.movRange -= reachableHexes[isHex->cell.LogicPos];
        if(stats.flying){
            isHex->cell.airID = this->ID;
            isHex->cell.occupiedAir = true;
        }else{
            isHex->cell.groundID = this->ID;
            isHex->cell.occupiedGround = true;
        }
    }
    selectedAbil = -1;
    updateMovRange();
}

void Unit::takeDamage(int damage, bool ignoreArmor, EFFECTS effect){
    int finaldamage = max(0,damage - (ignoreArmor ? 0 : stats.def));
    stats.health = max(0,stats.health - finaldamage);
    (*obiektyRef)[Globals::numberOfEntities++] = make_unique<TextParticle>(ImVec2(Globals::windowW,Globals::windowH), -finaldamage ,effect, this->pos,Globals::numberOfEntities);
}

void Unit::resolveEffects()
{
    for(int i = 0; i < EFFECTS_COUNT; i++){
        if(stats.effects[i].duration > 0){
         switch(i){
        case DAMAGE:
            takeDamage(stats.effects[i].intensity, false, DAMAGE);
            stats.effects[i].duration = 0;
            break;
        case POISON:
            takeDamage(stats.effects[i].intensity/2,true, POISON);
            stats.def = max(0, stats.def - stats.effects[i].intensity);
            break;
        case BURNING:
            takeDamage(stats.effects[i].intensity,false, BURNING);
            break;
        case SLOW:
            stats.effects[i].intensity /= 2;
            break;
         }
         stats.effects[i].duration -= 1;
         if(stats.effects[i].duration < 1)
            stats.effects[i].intensity = 0;
        }
    }
}

void Unit::startOfTurnCore()
{
    resolveEffects();
}

void Unit::endOfTurnCore(endTurnEffects effects)
{

}

void Unit::resolveStartOfTurn()
{
    startOfTurnCore();
}

void Unit::resolveEndOfTurn(endTurnEffects effects)
{
    endOfTurnCore(effects);
}

class GenericUnit : public Unit {
public:
    GenericUnit(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, Model("Model/GenericTest/bullshit.obj"), HexGrid, factionID, objID) {
        stats = {5, 5, 7, 1, 1, 7, 7, 0, 2, 1, true, false};
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[MISSILE] = MISSILE;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 3;
        abilitiesRanges[MISSILE] = 4;

        ///AbilityEffectList
        (abilityEffects[0])[BURNING] = effect{2,1};

        properHeight = 1.609f;
        pos.y = properHeight;
        transformMat = glm::translate(glm::mat4(1.f), pos);
        transformMat = glm::scale(transformMat,glm::vec3(0.1f));

    }
};
