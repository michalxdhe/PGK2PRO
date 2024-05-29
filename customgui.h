#ifndef CUSTOMGUI_H_INCLUDED
#define CUSTOMGUI_H_INCLUDED

#include "shaders.h"

extern ImGuiWindowFlags invisPreset;

enum ResourceType {
    ORE,
    GEMS,
    GAS,
    OIL,
    WATER,
    FIRE,
    EARTH,
    AIR,
    RESOURCE_COUNT
};

class Unit;

class GuiElement : public Object {
public:
    ImVec2 windowSpan;
};

class UnitGui : public GuiElement {
public:
    float windowWi, windowHe;
    float windowX, windowY;
    float statswindowWi, statswindowHe;
    float statswindowX, statswindowY;
    int *hp, *spd, *att, *def, *movRange;

    UnitGui();
    UnitGui(ImVec2 windowSize, int *health, int *speed, int *attack, int *defense, int *movementRange);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

class PlayerGui : public GuiElement {
public:
    float windowWi, windowHe;
    float windowX, windowY;
    int *resources[RESOURCE_COUNT];
    unsigned int resTextures[RESOURCE_COUNT];

    PlayerGui();
    PlayerGui(ImVec2 windowSize, int *r1, int *r2, int *r3, int *r4, int *r5, int *r6, int *r7, int *r8);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

class InitiativeTrackerGui : public GuiElement {
public:
    float windowWi, windowHe;
    float windowX, windowY;
    std::deque<Unit> *queueRef;

    InitiativeTrackerGui();
    InitiativeTrackerGui(ImVec2 windowSize, std::deque<Unit> *qu);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

#endif // CUSTOMGUI_H_INCLUDED
