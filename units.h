#ifndef UNITS_H_INCLUDED
#define UNITS_H_INCLUDED
#include "globals.h"
#include "dijkstra.h"

using namespace std;

enum AOE_TYPE{
    RANGE,
    LINE,
    CROSS,
    AOE_TYPE_COUNT
};

enum UnitType{
    GENERIC_UNIT,
    UNIT_TYPE_COUNT
};

struct UnitStats
{
    int health, maxHealth, speed, att, def, movRange, maxMovRange, actionTokens, maxActionTokens, miningCapability;
    bool flying = false;
    bool isCommander = false;
    bool isBuilding = false;
    effect effects[EFFECTS_COUNT];
    int targetFaction;
    float properHeight = 1.607f;
    array<int,RESOURCE_COUNT> cost;
    bool yourTurn = false;
};

struct AOE{
    int radius = 1;
    AOE_TYPE type = RANGE;
};

unordered_map<glm::vec3, int> getCellsUpToDist(unordered_map<glm::vec3, int> pathable, int dist);

class HexObject : public Object, public Selectable
{
public:
    glm::vec3 hexPos;
    int owner;
};

class Unit : public HexObject
{
public:
    glm::mat4 transformMat;
    glm::vec3 pos;
    glm::vec3 scaleOutline = glm::vec3(1.05f);
    Model model;

    int selectedAbil = -1;
    array<int, ABILITIES_COUNT> abilitiesList;
    array<int, ABILITIES_COUNT> abilitiesRanges;
    array<AOE, ABILITIES_COUNT> abilitiesAOE;
    array<unordered_map<int,effect>, ABILITIES_COUNT> abilityEffects;

    UnitStats stats;
    UnitGui guiHotBar;
    unordered_map<glm::vec3, int> reachableHexes;
    unordered_map<glm::vec3, int> inAbilityRange;
    unordered_map<glm::vec3, HexCell> *hexGrid;
    UnitBar guiHealthBar;

    Unit();
    Unit(glm::vec3 hexCellCords, Model model, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID, bool flying);
    void commandLC(Selectable *target, abilityCall *orderInfo);
    void commandRC(Selectable *target, abilityCall *orderInfo);
    void onSelect();
    void onHover();
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
    void updateMovRange(int malus = 0);
    void updateAbilRange(int malus = 0);
    void resolveStartOfTurn();
    void resolveEndOfTurn(endTurnEffects effects);
    void takeDamage(int damage, bool ignoreArmor = false, EFFECTS effect = DAMAGE);
    void rotateTowardsHex(glm::vec3 theHex);
private:
    void startOfTurnCore();
    void endOfTurnCore(endTurnEffects effects);
    void useAbilOnUnit(Selectable *target, abilityCall *orderInfo);
    void useAbilOnHex(Selectable *target, abilityCall *orderInfo);
    void tryMoving(Selectable *target, abilityCall *orderInfo);
    void resolveEffects();
};

#endif // UNITS_H_INCLUDED
