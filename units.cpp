#include "customgui.h"
#include "units.h"

vector<HexCell*> getHexesFromAOE(glm::vec3 culpritPos,AOE aoeInfo, glm::vec3 origin, unordered_map<glm::vec3, HexCell> *grid){
    vector<HexCell*> result;

    result.push_back(&(*grid)[origin]);

    switch(aoeInfo.type){
        case LINE:{
            glm::vec3 direction = get_direction(culpritPos, origin);

            int effective_radius = aoeInfo.radius;
            if (abs(direction.x) > 0 && abs(direction.y) > 0 && abs(direction.z) > 0) {
                effective_radius = static_cast<int>(floor(aoeInfo.radius * 0.5));
            }

            for (int i = 1; i < effective_radius; ++i) {
                glm::vec3 next_hex = origin + direction * glm::vec3(i);
                if ((*grid).find(next_hex) != (*grid).end()) {
                    result.push_back(&(*grid)[next_hex]);
                }
            }
            break;
        }
        case RANGE:{
                result.clear();
                unordered_map<glm::vec3, int> cells = getPathableCellsRange(*grid,origin);

                for(auto it : cells)
                {
                    if(it.second <= aoeInfo.radius)
                        result.push_back(&(*grid)[it.first]);
                }

            break;
        }
        case CROSS: {
            int x = 2;
            for (glm::vec3 direction : cube_direction_vectors) {
                if(x%2 == 0){
                    for (int i = 1; i <= aoeInfo.radius; ++i) {
                        glm::vec3 next_hex = origin + direction * glm::vec3(i);
                        if ((*grid).find(next_hex) != (*grid).end()) {
                            result.push_back(&(*grid)[next_hex]);
                        }
                    }
                }
                x++;
            }
            break;
        }
        default:
            break;
    }
    return result;
}

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

Unit::Unit(glm::vec3 hexCellCords, ModelWithPath mod, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID, bool flying)
    : stats{5, 5, 3, 2, 1, 0, 4, 0, 2, 0},
      guiHotBar(ImVec2(Globals::windowW, Globals::windowH), &stats, &abilitiesList, &selectedAbil, &availableToBuild, &availableToMorph), hexGrid(HexGrid),
      guiHealthBar(ImVec2(Globals::windowW, Globals::windowH), &stats, &pos, objID)
{
    for(auto &it : abilitiesList)
    {
        it = -1;
    }
    stats.flying = flying;
    abilitiesList[0] = 0;
    abilitiesRanges[0] = 1;
    this->ID = objID;
    this->owner = factionID;
    stats.ownerID = factionID;
    hexPos = hexCellCords;

    ///model i animacja
    this->model = mod.model;
    animations = Animation(mod.path,&model);
    animator = Animator(&animations);

    for(int i = 0; i < ABILITIES_COUNT; i++){
        abilitiesList[i] = ABILITIES_COUNT;
    }

    for(int i = 0; i < 10; i++){
        availableToBuild[i] = UNIT_TYPE_COUNT;
    }

    for(int i = 0; i < 10; i++){
        availableToMorph[i] = UNIT_TYPE_COUNT;
    }

    pos = getWorldPosFromHex(hexPos);
    pos.y = 0;

    if(stats.flying){
        (*hexGrid)[hexPos].airID = this->ID;
        (*hexGrid)[hexPos].occupiedAir = true;
    }else{
        (*hexGrid)[hexPos].groundID = this->ID;
        (*hexGrid)[hexPos].occupiedGround = true;
    }

    transformMat = glm::translate(glm::mat4(1.f), pos);
    boundingBox = Cube(0.21f, 0.21f, 0.21f, pos);
}

/** \brief Metoda wykonujaca siê przy kliknieciu lewego przycisku myszy gdy Unit jest selected
 *
 * \param target Selectable* To w co sie kliknelo
 * \param orderInfo abilityCall* Uzywane do obslugi umiejetnosci, jesli takowa jest uzyta
 * \return void
 *
 */
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

/** \brief Metoda wykonujaca siê przy kliknieciu prawego przycisku myszy gdy Unit jest selected
 *
 * \param target Selectable* To w co sie kliknelo
 * \param orderInfo abilityCall* Uzywane do obslugi umiejetnosci, jesli takowa jest uzyta
 * \return void
 *
 */
void Unit::commandRC(Selectable *target, abilityCall *orderInfo)
{
    tryMoving(target,orderInfo);
    glm::vec3 targetPos = getWorldPosFromHex(hexPos);
    rotateTowardsHex(targetPos);
    pos = targetPos;
    pos.y = 0;
    transformMat[3] = glm::vec4(pos,1.0f);
}

void Unit::onSelectSound(){

}

void Unit::onSelect()
{
    isSelected = 1;
    onSelectSound();
    updateMovRange();
    updateAbilRange();
}

void Unit::onHover()
{
    hovering = 1;
}

void Unit::update(double deltaTime)
{
    animator.UpdateAnimation(deltaTime);
    boundingBox.model = glm::translate(glm::mat4(1.f), glm::vec3(pos.x, stats.properHeight, pos.z));
    if(isSelected){
        updateMovRange();
        updateAbilRange();
    }
    guiHealthBar.update(deltaTime);
    if(!isSelected)
        selectedAbil = -1;
}

void Unit::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{

    auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i){
            std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }


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

    glUniform1i(glGetUniformLocation(shaderProgram, "factionID"), owner-1);

    if((hovering || isSelected) && shaderProgram != shaderPrograms[2]){
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
    }
    ///aha
    if(stats.flying){
        glm::mat4 transformedMat = translate(transformMat,glm::vec3(0.f,stats.properHeight/5, 0.f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transformedMat));
    }else{
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transformMat));
    }


    glUniform1i(glGetUniformLocation(shaderProgram, "unitRender"), true);
    model.Draw(shaderProgram,true);

    //DEBUG
    //boundingBox.Draw(shaderProgram, hovering || isSelected);
    glUniform1i(glGetUniformLocation(shaderProgram, "unitRender"), false);

    if((hovering || isSelected) && shaderProgram != shaderPrograms[2]){
        glUniform1i(glGetUniformLocation(shaderProgram, "factionHighlight"), true);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glm::mat4 modelScaled = glm::scale(transformMat,scaleOutline);
        ///aha
        if(stats.flying){
            glm::mat4 transformedMat = translate(modelScaled,glm::vec3(0.f,stats.properHeight/5, 0.f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transformedMat));
        }else{
          glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelScaled));
        }
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

    glm::mat3 newRotation;

    if(!stats.fixRotate)
        newRotation = glm::mat3(glm::inverse(glm::lookAt(ourPos,hexPos,glm::vec3(0.f,1.f,0.f))));
    else
        newRotation = glm::mat3(glm::inverse(glm::lookAt(hexPos,ourPos,glm::vec3(0.f,1.f,0.f))));


    if(ourPos != hexPos)
        this->transformMat = glm::translate(glm::mat4(1.f),translation) * glm::mat4(newRotation) * glm::scale(glm::mat4(1.f),scale);
}

/** \brief Kalkuluje i store'uje zasieg umiejetnosci aktualnie wybranej
 *
 * \param malus int
 * \return void
 *
 */
void Unit::updateAbilRange(int malus)
{
    inAbilityRange.clear();
    if(selectedAbil != -1)
        inAbilityRange = getCellsUpToDist(getPathableCellsRange(*hexGrid, hexPos), std::max(0, abilitiesRanges[selectedAbil]));
}

/** \brief Jesli zostal klikniety unit z wybrana umiejetnoscia to uzywa jej na niego
 *
 * \param target Selectable* To w co sie kliknelo
 * \param orderInfo abilityCall* Uzywane do obslugi umiejetnosci, jesli takowa jest uzyta
 * \return void
 *
 */
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
        orderInfo->offSpring = stats.selectedToBuild;

        orderInfo->target = getHexesFromAOE(this->hexPos,abilitiesAOE[selectedAbil],isUnit->hexPos,hexGrid);
    }
}

/** \brief Jesli zostal klikniety hex z wybrana umiejetnoscia to uzywa jej na niego
 *
 * \param target Selectable* To w co sie kliknelo
 * \param orderInfo abilityCall* Uzywane do obslugi umiejetnosci, jesli takowa jest uzyta
 * \return void
 *
 */
void Unit::useAbilOnHex(Selectable *target, abilityCall *orderInfo)
{
    Hexagon* isHex = dynamic_cast<Hexagon*>(target);
    if(isHex != nullptr && inAbilityRange.find(isHex->cell.LogicPos) != inAbilityRange.end())
    {
        orderInfo->abilityID = selectedAbil;
        orderInfo->culprit = this;
        for(auto it : abilityEffects[selectedAbil]){
            orderInfo->effects[it.first] = it.second;
        }
        orderInfo->target = getHexesFromAOE(this->hexPos,abilitiesAOE[selectedAbil],isHex->cell.LogicPos,hexGrid);
        orderInfo->offSpring = stats.selectedToBuild;
    }
}

/** \brief Jesli zostal klikniety hex bez wybranej umiejetnosci to porusza sie do niego
 *
 * \param target Selectable* To w co sie kliknelo
 * \param orderInfo abilityCall* Uzywane do obslugi umiejetnosci, jesli takowa jest uzyta
 * \return void
 *
 */
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
    int finaldamage;
    if(stats.def > 0 && effect != HEAL)
        finaldamage = max(0, damage - (ignoreArmor ? 0 : stats.def));
    else if(effect != HEAL)
        finaldamage = damage - stats.def;
    else
        finaldamage = damage;
        if(effect == HEAL)
            stats.health = min(stats.maxHealth,stats.health - finaldamage);
        else
            stats.health = max(0,stats.health - finaldamage);
    (*obiektyRef)[Globals::numberOfEntities++] = make_unique<TextParticle>(ImVec2(Globals::windowW,Globals::windowH), -finaldamage ,effect, glm::vec3(pos.x,pos.y+stats.properHeight,pos.z),Globals::numberOfEntities);
}

/** \brief Obsluga efektow na jednostce
 *
 * \return void
 *
 */
void Unit::resolveEffects()
{
    for(int i = 0; i < EFFECTS_COUNT; i++){
        if(stats.effects[i].duration > 0 || stats.effects[i].intensity > 0){
         switch(i){
        case DAMAGE:
            takeDamage(stats.effects[i].intensity, false, DAMAGE);
            stats.effects[i].duration = 1;
            break;
        case POISON:
            if(!stats.isBuilding)
                takeDamage(stats.effects[i].intensity/2,true, POISON);
            stats.def = max(0, stats.def - stats.effects[i].intensity);
            break;
        case BURNING:
            takeDamage(stats.effects[i].intensity,false, BURNING);
            break;
        case SLOW:
            stats.effects[i].intensity /= 2;
            break;
        case HEAL:
            takeDamage(-stats.effects[i].intensity,true, HEAL);
            break;
         }
         stats.effects[i].duration -= 1;
         if(stats.effects[i].duration < 1)
            stats.effects[i].intensity = 0;
        }
    }
}

/** \brief Pomocnicza metoda,
 *  jakby ktos chcial aby jego jednostka robila co innego na poczatku tury, np. wzrastal jej atak, wystarczy zoverloadowac to
 * \return void
 *
 */
void Unit::startOfTurnCore()
{
    resolveEffects();
}

/** \brief Pomocnicza metoda,
 * jakby ktos chcial aby jego jednostka robila co innego na koniec tury, wystarczy zoverloadowac to
 * \return void
 *
 */
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

class Larve : public Unit {
public:
    Larve(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[LARVE], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,2, 2, 8, 1, 0, 2, 2, 0, 3, 1);
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[MORPH] = MORPH;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 1;
        abilitiesRanges[MORPH] = 0;

        ///AbilityAOEtypes
        abilitiesAOE[ATTACK] = {0,RANGE};

        ///AbilityEffectList
        (abilityEffects[ATTACK])[POISON] = effect{1,1};

        ///BuildList IF CREATE ABIL IS PRESENT
        availableToMorph[0] = EGG;

        ///Cost Adjustment
        stats.cost[ORE] = 2;
        stats.buildActionPointCost = 1;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.15f, 0.07, 0.15f, pos);
        scaleOutline = glm::vec3(1.003);
    }

    void onSelectSound(){
        audioJungle.play(WORM_GRUNT);
    }

};

class Egg : public Unit {
public:
    Egg(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[EGG], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,1, 5, 1, 0, 2, 0, 0, 0, 3, 0);
        ///AbilityDec
        abilitiesList[MORPH] = MORPH;

        ///AbilityRangeSet
        abilitiesRanges[MORPH] = 0;

        ///AbilityAOEtypes
        abilitiesAOE[MORPH] = {0,RANGE};

        ///AbilityEffectList

        ///BuildList IF CREATE ABIL IS PRESENT
        availableToMorph[0] = BIRD;
        availableToMorph[1] = CENTI;
        availableToMorph[2] = MORTARBUG;

        ///Cost Adjustment
        stats.cost[ORE] = 2;
        stats.cost[GAS] = 1;
        stats.buildActionPointCost = 3;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.005);

        stats.effects[HEAL].duration = 4;
        stats.effects[HEAL].intensity = 1;
    }
};

class Centipede : public Unit {
public:
    Centipede(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[CENTI], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,10, 10, 3, 2, 2, 2, 2, 1, 3, 0);
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[DECIMATE] = DECIMATE;
        abilitiesList[RALLY] = RALLY;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 1;
        abilitiesRanges[DECIMATE] = 1;
        abilitiesRanges[RALLY] = 0;

        ///AbilityAOEtypes
        abilitiesAOE[RALLY] = {1,RANGE};

        ///AbilityEffectList
        (abilityEffects[ATTACK])[DAMAGE] = effect{3,1};
        (abilityEffects[ATTACK])[SLOW] = effect{4,4};
        (abilityEffects[DECIMATE])[SLOW] = effect{6,4};

        ///BuildList IF CREATE ABIL IS PRESENT

        ///Cost Adjustment
        stats.cost[ORE] = 4;
        stats.cost[GAS] = 1;
        stats.buildActionPointCost = 3;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.009);
    }
};

class Birdie : public Unit {
public:
    Birdie(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[BIRD], HexGrid, factionID, objID, true) {
        setSomeStats(&stats,3, 3, 9, 1, 0, 5, 5, 2, 2, 0);
        stats.flying = true;

        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[SPIT] = SPIT;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 1;
        abilitiesRanges[SPIT] = 4;
        ///AbilityAOEtypes

        ///AbilityEffectList
        (abilityEffects[ATTACK])[DAMAGE] = effect{2,1};
        (abilityEffects[SPIT])[POISON] = effect{3,3};
        (abilityEffects[SPIT])[BURNING] = effect{1,1};

        ///BuildList IF CREATE ABIL IS PRESENT

        ///Cost Adjustment
        stats.cost[ORE] = 3;
        stats.cost[GAS] = 3;
        stats.buildActionPointCost = 3;

        ///adjustments
        stats.properHeight = 2.0f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.005);

        stats.fixRotate = true;
    }
};


class MortarBug : public Unit {
public:
    MortarBug(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[MORTARBUG], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,4, 4, 1, 3, 1, 1, 1, 1, 1, 0);
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 4;

        ///AbilityAOEtypes
        abilitiesAOE[ATTACK] = {1,RANGE};

        ///AbilityEffectList
        (abilityEffects[ATTACK])[POISON] = effect{1,3};

        ///BuildList IF CREATE ABIL IS PRESENT

        ///Cost Adjustment
        stats.cost[ORE] = 2;
        stats.cost[GAS] = 2;
        stats.buildActionPointCost = 2;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.008);
    }
};


class Polyp : public Unit {
public:
    Polyp(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[COLLECTOR], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,6, 6, 0, 0, 2, 0, 0, 0, 6, 2);
        stats.isBuilding = true;
        ///AbilityDec
        abilitiesList[MORPH] = MORPH;
        abilitiesList[HEALTHBOON] = HEALTHBOON;

        ///AbilityRangeSet
        abilitiesRanges[MORPH] = 0;
        abilitiesRanges[HEALTHBOON] = 0;

        ///AbilityAOEtypes
        abilitiesAOE[HEALTHBOON] = {1,RANGE};

        ///AbilityEffectList
        (abilityEffects[HEALTHBOON])[HEAL] = effect{3,2};

        ///BuildList IF CREATE ABIL IS PRESENT
        availableToMorph[0] = BALLER;

        ///Cost Adjustment
        stats.cost[ORE] = 2;
        stats.cost[GAS] = 2;
        stats.buildActionPointCost = 1;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.009);
    }
};


class BallSpider : public Unit {
public:
    BallSpider(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[BALLER], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,15, 15, 1, 1, 0, 2, 2, 0, 5, 0);
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[HEX] = HEX;
        abilitiesList[SPEEDBOON] = SPEEDBOON;
        abilitiesList[FORTIFY] = FORTIFY;
        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 3;
        abilitiesRanges[HEX] = 5;
        abilitiesRanges[SPEEDBOON] = 2;
        abilitiesRanges[FORTIFY] = 3;
        ///AbilityAOEtypes
        abilitiesAOE[FORTIFY] = {2,RANGE};

        ///AbilityEffectList
        (abilityEffects[HEX])[POISON] = effect{1,2};
        (abilityEffects[HEX])[DAMAGE] = effect{1,2};
        (abilityEffects[HEX])[SLOW] = effect{1,2};
        (abilityEffects[HEX])[BURNING] = effect{1,2};

        ///BuildList IF CREATE ABIL IS PRESENT

        ///Cost Adjustment
        stats.cost[ORE] = 5;
        stats.cost[GAS] = 5;
        stats.buildActionPointCost = 6;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.009);
    }
};


class LazyComm : public Unit {
public:
    LazyComm(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[COMM], HexGrid, factionID, objID, false) {
        setSomeStats(&stats,25, 25, 2, 1, 2, 2, 2, 4, 4, 1);
        stats.isCommander = true;
        stats.ownerID = factionID;
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[CREATE] = CREATE;
        abilitiesList[CLEANSE] = CLEANSE;
        abilitiesList[SPIT] = SPIT;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 1;
        abilitiesRanges[CREATE] = 1;
        abilitiesRanges[CLEANSE] = 2;
        abilitiesRanges[SPIT] = 3;

        ///AbilityAOEtypes
        abilitiesAOE[ATTACK] = {3,LINE};
        abilitiesAOE[SPIT] = {1,RANGE};
        abilitiesAOE[CLEANSE] = {1,RANGE};

        ///AbilityEffectList
        (abilityEffects[ATTACK])[BURNING] = effect{1,2};
        (abilityEffects[CLEANSE])[HEAL] = effect{1,2};
        (abilityEffects[SPIT])[SLOW] = effect{6,2};

        ///BuildList IF CREATE ABIL IS PRESENT
        availableToBuild[0] = LARVE;
        availableToBuild[1] = COLLECTOR;

        ///Cost Adjustment
        stats.cost[ORE] = 10;
        stats.cost[GAS] = 10;
        stats.buildActionPointCost = 30;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.13f, 0.1, 0.13f, pos);
        scaleOutline = glm::vec3(1.009);
    }
};

struct UnitFactory{
        inline static deque<unique_ptr<Unit>> toBeCreated;
        inline static unordered_map<glm::vec3, HexCell> *HexGridRef;
        inline static unordered_map<UnitType, function<unique_ptr<Unit>(glm::vec3, unordered_map<glm::vec3, HexCell>*, int, int)>> unitCreationMap;

        static void initialize(unordered_map<glm::vec3, HexCell> *HexGrid) {
            HexGridRef = HexGrid;

            unitCreationMap[LARVE] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<Larve>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[EGG] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<Egg>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[CENTI] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<Centipede>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[BIRD] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<Birdie>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[MORTARBUG] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<MortarBug>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[COLLECTOR] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<Polyp>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[BALLER] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<BallSpider>(hexCellCords, HexGrid, factionID, objID);
            };

            unitCreationMap[COMM] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<LazyComm>(hexCellCords, HexGrid, factionID, objID);
            };
        }

        static void createUnit(UnitType unitType, glm::vec3 hexCellCords, int factionID){
            auto it = unitCreationMap.find(unitType);
            if (it != unitCreationMap.end()) {
                toBeCreated.push_back(it->second(hexCellCords, HexGridRef, factionID,Globals::numberOfEntities++));
            } else {
                std::cerr << "Error: Unknown unit type." << std::endl;
            }
        }

        static void resolveCreation(unordered_map<int, unique_ptr<Object>> *obiekty, deque<Unit> *initiativeQueue){
            while(!toBeCreated.empty()){
                //(*initiativeQueue).push_back(*toBeCreated.front());
                if(toBeCreated.front()->stats.flying){
                    (*HexGridRef)[toBeCreated.front()->hexPos].airID = toBeCreated.front()->ID;
                    (*HexGridRef)[toBeCreated.front()->hexPos].occupiedAir = true;
                }
                else{
                    (*HexGridRef)[toBeCreated.front()->hexPos].groundID = toBeCreated.front()->ID;
                    (*HexGridRef)[toBeCreated.front()->hexPos].occupiedGround = true;
                }
                (*obiekty)[toBeCreated.front()->ID] = move(toBeCreated.front());
                toBeCreated.pop_front();
            }
        }
};
