#ifndef CUSTOMGUI_H_INCLUDED
#define CUSTOMGUI_H_INCLUDED

#include "abilities.h"
#include "shaders.h"

extern ImGuiWindowFlags invisPreset;

enum ResourceType
{
    ORE,
    GAS,
    GEMS,
    OIL,
    WATER,
    FIRE,
    EARTH,
    AIR,
    RESOURCE_COUNT
};

extern const std::string abilityNames[ABILITIES_COUNT];

struct UnitStats;

class Unit;

class GuiElement : public Object
{
public:
    ImVec2 windowSpan;
};

class TextParticle : public GuiElement
{
public:
    glm::vec3 worldPos;
    double lifeTimer;
    std::string message;
    ImVec4 color;

    TextParticle();
    TextParticle(ImVec2 windowSize, int damage, EFFECTS type, glm::vec3 worldPos, int objID);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

class UnitGui : public GuiElement
{
public:
    float windowWi, windowHe;
    float windowX, windowY;
    float statswindowWi, statswindowHe;
    float statswindowX, statswindowY;
    UnitStats *stats;
    std::array<int, ABILITIES_COUNT> *abilityList;
    int* selectedAbil;

    GLuint activeTokenTexture;
    GLuint inactiveTokenTexture;

    GLuint godForgiveMe[5];

    GLuint effectText[EFFECTS_COUNT];

    UnitGui();
    UnitGui(ImVec2 windowSize, UnitStats *stats,  std::array<int, ABILITIES_COUNT> *abilityList, int* selectedAbil);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

class PlayerGui : public GuiElement
{
public:
    float windowWi, windowHe;
    float windowX, windowY;
    std::array<int, RESOURCE_COUNT> *resources;
    unsigned int resTextures[RESOURCE_COUNT];

    PlayerGui();
    PlayerGui(ImVec2 windowSize, std::array<int, RESOURCE_COUNT> *resourcesRef);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

class InitiativeTrackerGui : public GuiElement
{
public:
    float windowWi, windowHe;
    float windowX, windowY;
    std::deque<Unit> *queueRef;
    int64_t *highlightRef;
    bool *hoveredRef;

    InitiativeTrackerGui();
    InitiativeTrackerGui(ImVec2 windowSize, std::deque<Unit> *qu, int64_t *highlight, bool *hovered);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};

class UnitBar : public GuiElement
{
public:
    float windowWi, windowHe;
    float windowX, windowY;
    UnitStats *stats;
    unsigned int marker_text;

    int64_t unitID;
    glm::vec3 *unitPos;

    GLuint effectText[EFFECTS_COUNT];

    UnitBar();
    UnitBar(ImVec2 windowSize, UnitStats *stats, glm::vec3 *unitPos, int64_t ID);
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);
};
#endif // CUSTOMGUI_H_INCLUDED
