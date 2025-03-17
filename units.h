#ifndef UNITS_H_INCLUDED
#define UNITS_H_INCLUDED

#include "globals.h"
#include "dijkstra.h"
#include "abilities.h"

#include "soundbullshit.h"

using namespace std;

enum AOE_TYPE{
    RANGE,
    LINE,
    CROSS,
    AOE_TYPE_COUNT
};

struct ModelWithPath{
    string path;
    Model model;
};

enum SOUNDS{
    WORM_GRUNT,
    SOUND_COUNT
};

static AudioPlayer audioJungle;

static ModelWithPath unitModels[UNIT_TYPE_COUNT];

/** \brief Struktura uzywana do przetwarzania umiejetnosci jednostek
 */
struct abilityCall
{
    Unit *culprit;
    int abilityID;
    /// Efekty sa stored wszystkie na raz, + jest taki ze nie trzeba potem szukac czy unit ma jakies efekty, - jest taki ze zawsze przesylamy wszystkie nawet jesli maja 0 statystyk
    effect effects[EFFECTS_COUNT];
    vector<HexCell*> target;
    UnitType offSpring;
};

/** \brief Struktura dla Unit'a i jego general statystyk, lecz nie wszystkich
 */
struct UnitStats
{
    int health, maxHealth, speed, att, def, movRange, maxMovRange, actionTokens, maxActionTokens, miningCapability;
    bool flying = false;
    bool isCommander = false;
    bool isBuilding = false;
    effect effects[EFFECTS_COUNT];
    int ownerID;
    float properHeight = 1.607f;
    array<int,RESOURCE_COUNT> cost;
    int buildActionPointCost = 1;
    bool yourTurn = false;
    bool fixRotate = false;
    UnitType selectedToBuild = UNIT_TYPE_COUNT;
};

void setSomeStats(UnitStats* stats, int health, int maxHealth, int speed, int att, int def, int movRange, int maxMovRange, int actionTokens, int maxActionTokens, int miningCapability) {
    stats->health = health;
    stats->maxHealth = maxHealth;
    stats->speed = speed;
    stats->att = att;
    stats->def = def;
    stats->movRange = movRange;
    stats->maxMovRange = maxMovRange;
    stats->actionTokens = actionTokens;
    stats->maxActionTokens = maxActionTokens;
    stats->miningCapability = miningCapability;
}


struct AOE{
    int radius = 0;
    AOE_TYPE type = LINE;
};

unordered_map<glm::vec3, int> getCellsUpToDist(unordered_map<glm::vec3, int> pathable, int dist);

/** \brief
 */
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
    Animation animations;
	Animator animator;

    int selectedAbil = -1;
    array<int, ABILITIES_COUNT> abilitiesList;
    array<int, ABILITIES_COUNT> abilitiesRanges;
    array<AOE, ABILITIES_COUNT> abilitiesAOE;
    array<UnitType, 10> availableToBuild;
    array<UnitType, 10> availableToMorph;
    array<unordered_map<int,effect>, ABILITIES_COUNT> abilityEffects;

    UnitStats stats;
    UnitGui guiHotBar;
    unordered_map<glm::vec3, int> reachableHexes;
    unordered_map<glm::vec3, int> inAbilityRange;
    unordered_map<glm::vec3, HexCell> *hexGrid;
    UnitBar guiHealthBar;

    Unit();
    Unit(glm::vec3 hexCellCords, ModelWithPath mod, unordered_map<glm::vec3, HexCell> *HexGrid, int factionID, int64_t objID, bool flying);
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
    virtual void onSelectSound();
private:
    virtual void startOfTurnCore();
    virtual void endOfTurnCore(endTurnEffects effects);
    void useAbilOnUnit(Selectable *target, abilityCall *orderInfo);
    void useAbilOnHex(Selectable *target, abilityCall *orderInfo);
    void tryMoving(Selectable *target, abilityCall *orderInfo);
    void resolveEffects();
};

#endif // UNITS_H_INCLUDED
