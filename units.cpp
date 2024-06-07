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
    : stats{5, 5, 3, 2, 1, 0, 4, 0, 2, 0, flying},
      guiHotBar(ImVec2(Globals::windowW, Globals::windowH), &stats, &abilitiesList, &selectedAbil, &availableToBuild, &availableToMorph), hexGrid(HexGrid),
      guiHealthBar(ImVec2(Globals::windowW, Globals::windowH), &stats, &pos, objID)
{
    for(auto &it : abilitiesList)
    {
        it = -1;
    }
    abilitiesList[0] = 0;
    abilitiesRanges[0] = 1;
    this->ID = objID;
    this->owner = factionID;
    hexPos = hexCellCords;

    ///model i animacja
    this->model = mod.model;
    animations = Animation(mod.path,&model);
    animator = Animator(&animations);

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

        glm::mat4 modelScaled = glm::scale(transformMat,scaleOutline);
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
    int finaldamage = max(0,damage - (ignoreArmor ? 0 : stats.def));
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
        if(stats.effects[i].duration > 0){
         switch(i){
        case DAMAGE:
            takeDamage(stats.effects[i].intensity, false, DAMAGE);
            stats.effects[i].duration = 0;
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
         }
         stats.effects[i].duration -= 1;
         if(stats.effects[i].duration < 1)
            stats.effects[i].intensity = 0;
        }
    }
}

/** \brief Pomocnicza metoda, jakby ktos chcial aby jego jednostka robila co innego na poczatku tury, np. wzrastal jej atak
 *
 * \return void
 *
 */
void Unit::startOfTurnCore()
{
    resolveEffects();
}

/** \brief Pomocnicza metoda, jakby ktos chcial aby jego jednostka robila co innego na koniec tury
 *
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

class GenericUnit : public Unit {
public:
    GenericUnit(glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID)
        : Unit(hexCellCords, unitModels[GENERIC_UNIT], HexGrid, factionID, objID, false) {
        stats = {5, 5, 7, 1, 1, 7, 7, 0, 2, 1, .isCommander = false};
        ///AbilityDec
        abilitiesList[ATTACK] = ATTACK;
        abilitiesList[CREATE] = CREATE;

        ///AbilityRangeSet
        abilitiesRanges[ATTACK] = 1;
        abilitiesRanges[CREATE] = 1;

        ///AbilityAOEtypes
        abilitiesAOE[ATTACK] = {1,RANGE};
        abilitiesAOE[CREATE] = {0,RANGE};

        ///AbilityEffectList
        (abilityEffects[ATTACK])[POISON] = effect{2,1};

        ///BuildList IF CREATE ABIL IS PRESENT
        availableToBuild[0] = GENERIC_UNIT;

        ///Cost Adjustment
        stats.cost[ORE] = 1;
        stats.buildActionPointCost = 1;

        ///adjustments
        stats.properHeight = 1.50f;
        boundingBox = Cube(0.15f, 0.07, 0.15f, pos);
        scaleOutline = glm::vec3(1.003);
    }
};


struct UnitFactory{
        inline static deque<unique_ptr<Unit>> toBeCreated;
        inline static unordered_map<glm::vec3, HexCell> *HexGridRef;
        inline static unordered_map<UnitType, function<unique_ptr<Unit>(glm::vec3, unordered_map<glm::vec3, HexCell>*, int, int)>> unitCreationMap;

        static void initialize(unordered_map<glm::vec3, HexCell> *HexGrid) {
            HexGridRef = HexGrid;

            unitCreationMap[GENERIC_UNIT] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID, int objID) {
                return std::make_unique<GenericUnit>(hexCellCords, HexGrid, factionID, objID);
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
                (*obiekty)[toBeCreated.front()->ID] = move(toBeCreated.front());
                toBeCreated.pop_front();
            }
        }
};
