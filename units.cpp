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
    (*HexGrid)[hexPos].groundID = this->ID;
    (*HexGrid)[hexPos].occupiedGround = true;
    transformMat = glm::translate(glm::mat4(1.f), pos);
    boundingBox = Cube(0.21f, 0.21f, 0.21f, pos);
}

void Unit::commandLC(Selectable *target, abilityCall *orderInfo)
{
    useAbilOnUnit(target,orderInfo);
    useAbilOnHex(target,orderInfo);
}

void Unit::commandRC(Selectable *target, abilityCall *orderInfo)
{
    tryMoving(target,orderInfo);
    pos = getWorldPosFromHex(hexPos);
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
    reachableHexes = getCellsUpToDist(getPathableCellsGround(*hexGrid, hexPos), std::max(0, stats.movRange - malus));
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
    if(isUnit != nullptr)
    {
        orderInfo->abilityID = selectedAbil;
        orderInfo->culprit = this;
        ///powinienes sprawdzic raczej czy hexpos w tym unitcie jest poprawny
        orderInfo->target.push_back(&(*isUnit->hexGrid)[isUnit->hexPos]);
    }
}

void Unit::useAbilOnHex(Selectable *target, abilityCall *orderInfo)
{
    Hexagon* isHex = dynamic_cast<Hexagon*>(target);
    if(isHex != nullptr)
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
        (*hexGrid)[this->hexPos].groundID = -1;
        (*hexGrid)[this->hexPos].occupiedGround = false;
        this->hexPos = isHex->cell.LogicPos;
        this->stats.movRange -= reachableHexes[isHex->cell.LogicPos];
        isHex->cell.groundID = this->ID;
        isHex->cell.occupiedGround = true;
    }
    selectedAbil = -1;
    updateMovRange();
}

void Unit::resolveStartOfTurn()
{
    cout << "nowa tura dla unita: " << this->ID << endl;
}

void Unit::resolveEndOfTurn(endTurnEffects effects)
{

}

class GenericUnit : public Unit {
public:
    GenericUnit(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, Model("Model/GenericTest/bullshit.obj"), HexGrid, factionID, objID) {
        stats = {5, 5, 7, 4, 1, 7, 7, 0, 1, 1};
        abilitiesList[0] = ATTACK;
        abilitiesRanges[0] = 3;
        properHeight = 1.609f;
        pos.y = properHeight;
        transformMat = glm::translate(glm::mat4(1.f), pos);
        transformMat = glm::scale(transformMat,glm::vec3(0.1f));
    }
};
