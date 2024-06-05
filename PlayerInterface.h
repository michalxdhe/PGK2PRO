#ifndef PLAYERINTERFACE_H_INCLUDED
#define PLAYERINTERFACE_H_INCLUDED

#include "globals.h"
#include "customgui.h"

class PlayerInterface
{

public:

    std::array<int, RESOURCE_COUNT> resources;

    int selectedID = -1;
    int commanderID = -1;

    PlayerGui resourceOverlay;

    PlayerInterface()
        : resources{0, 0, 0, 0, 0, 0, 0, 0},
          resourceOverlay(ImVec2(Globals::windowW, Globals::windowH), &resources)
    {
    }

    void renderGui(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
    {
        resourceOverlay.render(shaderProgram, shaderPrograms);
    }
};

#endif // PLAYERINTERFACE_H_INCLUDED
