#ifndef BOARDLOGIC_H_INCLUDED
#define BOARDLOGIC_H_INCLUDED

#include "model.h"

using namespace std;

static glm::vec3 cube_direction_vectors[] = {
    glm::vec3(1,0,-1), glm::vec3(1,-1,0), glm::vec3(0,-1,1),
    glm::vec3(-1,0,1), glm::vec3(-1,1,0), glm::vec3(0,1,-1)
};

class HexCell {

public:
    glm::vec3 LogicPos;
    int g, h;

    HexCell() = default;

    HexCell(glm::vec3 pos){
    LogicPos = pos;
    }
};

unordered_map<glm::vec3, HexCell> GenerateHexGrid(int gridSize){

HexCell bottom = HexCell(glm::vec3(0,-gridSize, gridSize));
HexCell top = HexCell(glm::vec3(0,gridSize, -gridSize));

HexCell temp;

unordered_map<glm::vec3, HexCell> grid;

while(bottom.LogicPos.y < 1){
    grid[bottom.LogicPos] = bottom;
    temp = bottom;
    bottom.LogicPos += cube_direction_vectors[4];
    temp.LogicPos += cube_direction_vectors[0];
    while(temp.LogicPos.x <= gridSize){
        grid[temp.LogicPos] = temp;
        temp.LogicPos += cube_direction_vectors[0];
    }
}

while(top.LogicPos.y > 0){
    grid[top.LogicPos] = top;
    temp = top;
    top.LogicPos += cube_direction_vectors[1];
    temp.LogicPos += cube_direction_vectors[3];
    while(temp.LogicPos.x >= -gridSize){
        grid[temp.LogicPos] = temp;
        temp.LogicPos += cube_direction_vectors[3];
    }
}

return grid;
}

#endif // BOARDLOGIC_H_INCLUDED
