#ifndef ABILITIES_H_INCLUDED
#define ABILITIES_H_INCLUDED

enum Ability
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
    int effectID;
    int intensity;
};

#endif // ABILITIES_H_INCLUDED
