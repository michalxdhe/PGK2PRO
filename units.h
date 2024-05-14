#ifndef UNITS_H_INCLUDED
#define UNITS_H_INCLUDED

#include "metaphysical.h"
#include "model.h"
#include "boardlogic.h"

using namespace std;

unordered_map<glm::vec3, int> getCellsUpToDist(unordered_map<glm::vec3, int> pathable, int dist)
{
    unordered_map<glm::vec3, int> cells;

    for (const auto& pair : pathable)
    {
        if (pair.second <= dist)
        {
            cells.insert(pair);
        }
    }

    return cells;
}

class HexObject : public Object, public Selectable
{

public:
    glm::vec3 hexPos;
};

class Unit : public HexObject
{

public:
    glm::mat4 transformMat;
    glm::vec3 pos;
    Model model;
    int health = 1, speed = 0, att = 0, def = 0, movRange = 4;

    unordered_map<glm::vec3, int> reachableHexes;
    unordered_map<glm::vec3, HexCell> &hexGrid;

    Unit() = default;

    Unit(glm::vec3 hexCellCords, Model model, unordered_map<glm::vec3, HexCell> &HexGrid) : hexGrid(HexGrid)
    {
        hexPos = hexCellCords;
        model = model;
        pos = glm::vec3(0.f);
        pos.x = hexPos.x;
        pos.y = 3.f;
        pos.z = hexPos.z;
        transformMat = glm::translate(glm::mat4(1.f), pos);


        boundingBox = Cube(0.21f, 0.21f, 0.21f, pos);
    };

    void commandLC(Selectable *target)
    {

    }

    void commandRC(Selectable *target)
    {
        Hexagon* isHex = dynamic_cast<Hexagon*>(target);
        if(isHex != nullptr && reachableHexes.find(isHex->cell.LogicPos) != reachableHexes.end())
        {
            this->hexPos = isHex->cell.LogicPos;
            this->movRange -= reachableHexes[isHex->cell.LogicPos];
        }
        if(this->movRange == 0)
            this->movRange = 4;
        updateMovRange();
    }

    void onSelect()
    {
        isSelected = 1;
        //tutaj szczegolnie zrob update tego gdzie jest wstanie pojsc z swoim zasiegiem unit uzywajac dijkstra algorytmu
        updateMovRange();
        //miejsce na call'e do np. pokazania gui dla tego konkretnego obiektu np.
    }

    void onHover()
    {
        hovering = 1;
        //miejsce na call'e do np. pokazania gui dla tego konkretnego obiektu np. jakis brief description
    }

    void update(double deltaTime)
    {
        pos = getWorldPosFromHex(hexPos);
        pos.y = 2.f;
        transformMat = glm::translate(glm::mat4(1.f),pos);
        boundingBox.model = glm::translate(glm::mat4(1.f),pos);
    }

    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
    {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transformMat));
        //model.Draw(shaderProgram);
        boundingBox.Draw(shaderProgram, hovering || isSelected);
    }
private:

    void updateMovRange(int malus = 0){
        reachableHexes.clear();
        reachableHexes = getCellsUpToDist(getPathableCells(hexGrid,hexPos),std::max(0,movRange - malus));
    }

};

#endif // UNITS_H_INCLUDED
