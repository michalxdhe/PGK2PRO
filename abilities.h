#ifndef ABILITIES_H_INCLUDED
#define ABILITIES_H_INCLUDED

enum ABILITIES
{
    ATTACK,
    CREATE,
    MORPH,
    ABILITIES_COUNT
};

enum EFFECTS{
    DAMAGE,
    POISON,
    BURNING,
    SLOW,
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
    "Basic Attack of a unit",
    "Build buildings or create units",
    "Morph into something else"
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
