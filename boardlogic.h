#ifndef BOARDLOGIC_H_INCLUDED
#define BOARDLOGIC_H_INCLUDED

using namespace std;

static Model resModels[RESOURCE_COUNT];

struct abilityCall
{
    Unit *culprit;
    int abilityID;
    effect effects[EFFECTS_COUNT];
    vector<HexCell*> target;
    Unit *offSpring;
};

struct endTurnEffects
{
    vector<effect> effects;
};

static glm::vec3 cube_direction_vectors[] =
{
    glm::vec3(1,0,-1), glm::vec3(1,-1,0), glm::vec3(0,-1,1),
    glm::vec3(-1,0,1), glm::vec3(-1,1,0), glm::vec3(0,1,-1)
};

glm::vec3 getRandomHex(int boardSize){
    int coPos = rand()% boardSize + 1;
    int coNeg = -(rand()% boardSize + 1);
    int coNet = -(coPos + coNeg);

    std::array<int, 3> co = {coPos, coNeg, coNet};

    std::random_shuffle(co.begin(), co.end());

    return glm::vec3(co[0], co[1], co[2]);
}

glm::vec3 getWorldPosFromHex(glm::vec3 LogicPos, glm::vec3 scale = glm::vec3(1.f))
{
    return glm::vec3(glm::vec3(LogicPos.x*0.45f * scale.x,0.f,LogicPos.z * 0.52f * scale.z + (-LogicPos.x * -0.26f * scale.z)));
}

unordered_map<glm::vec3, HexCell> GenerateHexGrid(int gridSize)
{

    HexCell bottom = HexCell(glm::vec3(0,-gridSize, gridSize));
    HexCell top = HexCell(glm::vec3(0,gridSize, -gridSize));

    HexCell temp;

    unordered_map<glm::vec3, HexCell> grid;

    while(bottom.LogicPos.y < 1)
    {
        grid[bottom.LogicPos] = bottom;
        temp = bottom;
        bottom.LogicPos += cube_direction_vectors[4];
        temp.LogicPos += cube_direction_vectors[0];
        while(temp.LogicPos.x <= gridSize)
        {
            grid[temp.LogicPos] = temp;
            temp.LogicPos += cube_direction_vectors[0];
        }
    }

    while(top.LogicPos.y > 0)
    {
        grid[top.LogicPos] = top;
        temp = top;
        top.LogicPos += cube_direction_vectors[1];
        temp.LogicPos += cube_direction_vectors[3];
        while(temp.LogicPos.x >= -gridSize)
        {
            grid[temp.LogicPos] = temp;
            temp.LogicPos += cube_direction_vectors[3];
        }
    }

    return grid;
};

vector<glm::vec3> getNeighbors(glm::vec3 cellPos)
{
    vector<glm::vec3> neighbors;
    for (int i = 0; i < 6; ++i)
    {
        neighbors.push_back(cellPos + cube_direction_vectors[i]);
    }
    return neighbors;
}

class Hexagon : public Object, public Selectable
{

public:
    Model model;
    glm::mat4 view;
    HexCell &cell;
    glm::vec3 scale;

    Hexagon() = default;

    Hexagon(Model model, HexCell &komorka): cell(komorka)
    {
        this->model = model;
        scale = glm::vec3(1.f);
        glm::vec3 pos = getWorldPosFromHex(cell.LogicPos,scale);
        view = glm::translate(glm::mat4(1.f),pos);
        view = glm::scale(view,scale);
        boundingBox = Cube(0.21f,0.21f,0.21f,pos + glm::vec3(0.f,1.25f,0.f));
    }

    void update(double deltaTime)
    {
        // view = translate(view, glm::vec3(0.f,0.002f,0.f));
    }

    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
    {
        if(isSelected || hovering){
            glUniform1i(glGetUniformLocation(shaderProgram, "hoveredHex"), true);
        }
        if(cell.moveRangeView)
            glUniform1i(glGetUniformLocation(shaderProgram, "moveRange"), true);
        if(cell.abilityRangeView)
            glUniform1i(glGetUniformLocation(shaderProgram, "abilityRange"), true);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(view));
        if(cell.passable)
        {
            this->model.Draw(shaderProgram);
        }
        if(isSelected || hovering){
            glUniform1i(glGetUniformLocation(shaderProgram, "hoveredHex"), false);
        }

        if(cell.moveRangeView){
            glUniform1i(glGetUniformLocation(shaderProgram, "moveRange"), false);
            cell.moveRangeView = false;
        }
        if(cell.abilityRangeView){
            glUniform1i(glGetUniformLocation(shaderProgram, "abilityRange"), false);
            cell.abilityRangeView = false;
        }

        if(cell.passable && cell.presentResource != -1)
            resModels[cell.presentResource].Draw(shaderProgram);
    }

    void onSelect()
    {
        isSelected = 1;
        //miejsce na call'e do np. pokazania gui dla tego konkretnego obiektu np.
    }

    void onHover()
    {
        hovering = 1;
        //miejsce na call'e do np. pokazania gui dla tego konkretnego obiektu np. jakis brief description
    }

    void commandRC(Selectable *target, abilityCall *orderInfo)
    {

    }

    void commandLC(Selectable *target, abilityCall *orderInfo)
    {

    }
};

#endif // BOARDLOGIC_H_INCLUDED
