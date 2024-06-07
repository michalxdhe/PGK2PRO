#ifndef ABILITIES_H_INCLUDED
#define ABILITIES_H_INCLUDED

enum ABILITIES
{
    ATTACK,
    CREATE,
    MISSILE,
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
    GENERIC_UNIT,
    UNIT_TYPE_COUNT
};

#endif // ABILITIES_H_INCLUDED
