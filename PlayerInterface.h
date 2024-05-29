#ifndef PLAYERINTERFACE_H_INCLUDED
#define PLAYERINTERFACE_H_INCLUDED

#include "globals.h"
#include "customgui.h"

class PlayerInterface
{

public:
    uint16_t ID;

    int ore, gems, gas, oil, water, fire, earth, air;

    int selectedID = -1;

    PlayerGui resourceOverlay;

    PlayerInterface() = delete;

     PlayerInterface(uint16_t ID)
        : ID(ID), ore(0), gems(0), gas(0), oil(0), water(0), fire(0), earth(0), air(0),
          resourceOverlay(ImVec2(Globals::windowW, Globals::windowH), &ore, &gems, &gas, &oil, &water, &fire, &earth, &air)
    {
    }

    void renderGui(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms){
    resourceOverlay.render(shaderProgram, shaderPrograms);
    }
};

#endif // PLAYERINTERFACE_H_INCLUDED
