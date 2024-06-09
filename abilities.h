#ifndef ABILITIES_H_INCLUDED
#define ABILITIES_H_INCLUDED

enum ABILITIES
{
    ATTACK,
    CREATE,
    MORPH,
    SPIT,
    HEX,
    DECIMATE,
    CLEANSE,
    RALLY,
    FORTIFY,
    SPEEDBOON,
    HEALTHBOON,
    ABILITIES_COUNT
};

enum EFFECTS{
    DAMAGE,
    POISON,
    BURNING,
    SLOW,
    HEAL,
    EFFECTS_COUNT
};

struct effect{
    int intensity = 0;
    int duration = 0;
};

enum UnitType : int{
    LARVE,
    EGG,
    CENTI,
    BIRD,
    MORTARBUG,
    COLLECTOR,
    BALLER,
    COMM,
    UNIT_TYPE_COUNT
};

static std::string abilityDesc[ABILITIES_COUNT] = {
    "1AP - Basic Attack of a unit",
    "Build buildings or create units",
    "Morph into something else",
    "2AP - Spit, low damage, high status effect density",
    "4AP - Hex, lower stats of an enemy",
    "3AP - Decimate, heavy blow, aimed at the armor of a foe",
    "2AP - Cleanse, decrease status effects from a unit",
    "3AP - Rally, increase att of allies",
    "3AP - Fortify, increase armor of allies",
    "2AP - SpeedBoon, increase speed and movRange of allies",
    "2AP - HealthBoon, increase the maxHealth of allies"
};

static std::string unitDesc[UNIT_TYPE_COUNT] = {
    "2 Ore, 1 AP - Larve, Small nimble Worker\n capable of applying Poison\n can morph into an egg",
    "2 Ore, 1 Gas, 3AP - an EggSac, fairly durable,\ncan morph into various units once matured",
    "4 Ore, 1 Gas, 3AP - Centipede, Formidable warrior with a high health pool\n capable of dealing high damage negating armor",
    "3 Ore, 3 Gas, 3AP - BirdOfPrey, Fast flying Harrasment unit",
    "2 Ore, 2 Gas, 2AP - MortarBug, Long Range Harrasment unit",
    "2 Ore, 2 Gas, 1AP - Polyp, Stationary resource extraction bioorganism,\n something sleeps within",
    "5 Ore, 5 Gas, 6AP - Baller, Powerful caster unit",
    "LazyComm"
};

#endif // ABILITIES_H_INCLUDED
