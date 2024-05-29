#ifndef UNITS_H_INCLUDED
#define UNITS_H_INCLUDED
#include "globals.h"
#include "dijkstra.h"

using namespace std;

unordered_map<glm::vec3, int> getCellsUpToDist(unordered_map<glm::vec3, int> pathable, int dist);

class HexObject : public Object, public Selectable {
public:
    glm::vec3 hexPos;
};

class UnitGui;

class Unit : public HexObject {
public:
    glm::mat4 transformMat;
    glm::vec3 pos;
    Model model;
    int health, speed, att, def, movRange;
    UnitGui guiHotBar;
    unordered_map<glm::vec3, int> reachableHexes;
    unordered_map<glm::vec3, HexCell> *hexGrid;

    Unit();
    Unit(glm::vec3 hexCellCords, Model model, unordered_map<glm::vec3, HexCell> *HexGrid);
    void commandLC(Selectable *target);
    void commandRC(Selectable *target);
    void onSelect();
    void onHover();
    void update(double deltaTime);
    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms);

private:
    void updateMovRange(int malus = 0);
};

#endif // UNITS_H_INCLUDED
