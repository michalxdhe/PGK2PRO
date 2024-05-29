#include "customgui.h"
#include "units.h"


unordered_map<glm::vec3, int> getCellsUpToDist(unordered_map<glm::vec3, int> pathable, int dist) {
    unordered_map<glm::vec3, int> cells;
    for (const auto& pair : pathable) {
        if (pair.second <= dist) {
            cells.insert(pair);
        }
    }
    return cells;
}

Unit::Unit() = default;

Unit::Unit(glm::vec3 hexCellCords, Model model, unordered_map<glm::vec3, HexCell> *HexGrid)
    : health(5), speed(1), att(1), def(1), movRange(4),
    hexGrid(HexGrid), guiHotBar(ImVec2(Globals::windowW, Globals::windowH), &health, &speed, &att, &def, &movRange) {
    hexPos = hexCellCords;
    this->model = model;
    pos = glm::vec3(0.f);
    pos.x = hexPos.x;
    pos.y = 3.f;
    pos.z = hexPos.z;
    transformMat = glm::translate(glm::mat4(1.f), pos);
    boundingBox = Cube(0.21f, 0.21f, 0.21f, pos);
}

void Unit::commandLC(Selectable *target) {
}

void Unit::commandRC(Selectable *target) {
    Hexagon* isHex = dynamic_cast<Hexagon*>(target);
    if(isHex != nullptr && reachableHexes.find(isHex->cell.LogicPos) != reachableHexes.end()) {
        this->hexPos = isHex->cell.LogicPos;
        this->movRange -= reachableHexes[isHex->cell.LogicPos];
    }
    if(this->movRange == 0)
        this->movRange = 4;
    updateMovRange();
}

void Unit::onSelect() {
    isSelected = 1;
    updateMovRange();
}

void Unit::onHover() {
    hovering = 1;
}

void Unit::update(double deltaTime) {
    pos = getWorldPosFromHex(hexPos);
    pos.y = 2.f;
    transformMat = glm::translate(glm::mat4(1.f), pos);
    boundingBox.model = glm::translate(glm::mat4(1.f), pos);
    updateMovRange();
}

void Unit::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transformMat));
    boundingBox.Draw(shaderProgram, hovering || isSelected);
    if(isSelected && shaderProgram != shaderPrograms[2])
        guiHotBar.render(shaderProgram, shaderPrograms);
}

void Unit::updateMovRange(int malus) {
    reachableHexes.clear();
    reachableHexes = getCellsUpToDist(getPathableCells(*hexGrid, hexPos), std::max(0, movRange - malus));
}
