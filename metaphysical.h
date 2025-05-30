#ifndef METAPHYSICAL_H_INCLUDED
#define METAPHYSICAL_H_INCLUDED

#include "common.h"

enum EnviromentObjects
{
    PEDESTAL,
    ENV_COUNT
};

static Model envModels[ENV_COUNT];

/** \brief deprecated,
 *
 * \param lightPos glm::vec3
 * \param up glm::vec3
 * \return glm::mat4 lightspacematrix dla shadowmapy
 *
 */
glm::mat4 calculateLightSpaceMatrix(glm::vec3 lightPos, glm::vec3 up)
{
    glm::mat4 viewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), up);

    float near_plane = 0.5f;
    float far_plane = 5.0f;

    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    glm::mat4 lightSpaceMatrix = projectionMatrix * viewMatrix;

    return lightSpaceMatrix;
}

/** \brief Obiekt komorki hexagonalnej
 */
class HexCell
{

public:
    ///Pozycja logiczna Cube coordynatow komorki
    glm::vec3 LogicPos;

    bool passable = 1;
    bool moveRangeView = false;
    bool abilityRangeView = false;
    int64_t groundID = -1;
    int64_t airID = -1;
    int presentResource = -1;

    bool occupiedGround = false;
    bool occupiedAir = false;

    HexCell() = default;

    HexCell(glm::vec3 pos)
    {
        LogicPos = pos;
    }
};

/** \brief Obiekt vertic�w u�o�onych w kostk�, raczej deprecated, uzywane tylko do debugowania i skybox'a
 */
class Cube
{

public:
    glm::mat4 model;
    GLuint VBO, VAO;
    std::vector<float> vertices;

    Cube() = default;

    Cube(float w, float h, float l, glm::vec3 pos = glm::vec3(0.f,0.f,0.f))
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model,pos);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        remodel(w,h,l);
    }

    void remodel(float w, float h, float l)
    {
        vertices =
        {
            -w, -h, -l,  0.0f,  0.0f, -1.0f,
                w, -h, -l,   0.0f,  0.0f, -1.0f,
                w,  h, -l,   0.0f,  0.0f, -1.0f,
                w,  h, -l,   0.0f,  0.0f, -1.0f,
                -w,  h, -l,  0.0f,  0.0f, -1.0f,
                -w, -h, -l,  0.0f,  0.0f, -1.0f,

                -w, -h,  l,  0.0f,  0.0f,  1.0f,
                w, -h,  l,   0.0f,  0.0f,  1.0f,
                w,  h,  l,   0.0f,  0.0f,  1.0f,
                w,  h,  l,   0.0f,  0.0f,  1.0f,
                -w,  h,  l,  0.0f,  0.0f,  1.0f,
                -w, -h,  l,  0.0f,  0.0f,  1.0f,

                -w,  h,  l, -1.0f,  0.0f,  0.0f,
                -w,  h, -l, -1.0f,  0.0f,  0.0f,
                -w, -h, -l, -1.0f,  0.0f,  0.0f,
                -w, -h, -l, -1.0f,  0.0f,  0.0f,
                -w, -h,  l, -1.0f,  0.0f,  0.0f,
                -w,  h,  l, -1.0f,  0.0f,  0.0f,

                w,  h,  l,   1.0f,  0.0f,  0.0f,
                w,  h, -l,   1.0f,  0.0f,  0.0f,
                w, -h, -l,   1.0f,  0.0f,  0.0f,
                w, -h, -l,   1.0f,  0.0f,  0.0f,
                w, -h,  l,   1.0f,  0.0f,  0.0f,
                w,  h,  l,   1.0f,  0.0f,  0.0f,

                -w,  h, -l,  0.0f,  1.0f,  0.0f,
                w,  h, -l,   0.0f,  1.0f,  0.0f,
                w,  h,  l,   0.0f,  1.0f,  0.0f,
                w,  h,  l,   0.0f,  1.0f,  0.0f,
                -w,  h,  l,  0.0f,  1.0f,  0.0f,
                -w,  h, -l,  0.0f,  1.0f,  0.0f,

                -w, -h, -l,  0.0f, -1.0f,  0.0f,
                w, -h, -l,   0.0f, -1.0f,  0.0f,
                w, -h,  l,   0.0f, -1.0f,  0.0f,
                w, -h,  l,   0.0f, -1.0f,  0.0f,
                -w, -h,  l,  0.0f, -1.0f,  0.0f,
                -w, -h, -l,  0.0f, -1.0f,  0.0f
            };

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Draw(unsigned int shaderProgram, bool filled)
    {
        glUseProgram(shaderProgram);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        if(!filled)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        if(!filled)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);
    }

};


/** \brief Funkcja do obliczenia pozycji ekranowej na bazie pozycji w worldSpace, to samo co w mousepickerze ale na odwrot
 *
 * \param pos const glm::vec3& pozycja w worldspace
 * \param viewMatrix const glm::mat4&
 * \param projectionMatrix const glm::mat4&
 * \param screenWidth int
 * \param screenHeight int
 * \return glm::vec2 2D pozycja ekranu
 *
 */
glm::vec2 calculateScreenPosition(glm::vec3 pos,  glm::mat4 viewMatrix, glm::mat4 projectionMatrix, int screenWidth, int screenHeight) {
    glm::vec4 clipSpacePos = projectionMatrix * viewMatrix * glm::vec4(pos, 1.0);

    if (clipSpacePos.w <= 0.0f) {
        return glm::vec2(-1.0f, -1.0f);
    }

    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;

    if (ndcSpacePos.x < -1.0f || ndcSpacePos.x > 1.0f || ndcSpacePos.y < -1.0f || ndcSpacePos.y > 1.0f || ndcSpacePos.z < -1.0f || ndcSpacePos.z > 1.0f) {
        return glm::vec2(-1.0f, -1.0f);
    }

    glm::vec2 screenSpacePos;
    screenSpacePos.x = (ndcSpacePos.x + 1.0f) * 0.5f * screenWidth;
    screenSpacePos.y = (1.0f - ndcSpacePos.y) * 0.5f * screenHeight;

    return screenSpacePos;
}


/** \brief Glowny abstrakt obiektu gry
 */
class Object
{

public:
    static glm::mat4 *viewRef;
    static glm::mat4 *projectionRef;
    /// Jakis hack, ref do mapy wszystkich obiektow, chyba uzywany tylko do damageparticles, jak narazie
    static std::unordered_map<int, std::unique_ptr<Object>> *obiektyRef;
    int64_t ID;
    virtual ~Object() = default;

    virtual void update(double deltaTime) = 0;
    virtual void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms) = 0;
};

glm::mat4* Object::viewRef = nullptr;
glm::mat4* Object::projectionRef = nullptr;
std::unordered_map<int, std::unique_ptr<Object>>* Object::obiektyRef = nullptr;

/** \brief Abstrakt klasa obiektu zdolnego do kolizji
 */
class BoundingColider
{

public:
    Cube boundingBox;

    virtual ~BoundingColider() = default;

};

struct abilityCall;

/** \brief Abstrakt klasa obiektu ktory da sie kliknac i wybrac, reaguje na najechanie
 */
class Selectable : public BoundingColider
{

public:
    bool hovering = 0;
    bool isSelected = 0;

    virtual ~Selectable() = default;

    void refresh()
    {
        hovering = 0;
    }

    virtual void onSelect() = 0;
    virtual void onHover() = 0;

    virtual void commandRC(Selectable *target, abilityCall *orderInfo) = 0;
    virtual void commandLC(Selectable *target, abilityCall *orderInfo) = 0;
};

/** \brief Bazowa klasa obiektow swiatla, mozna rozszerzyc o wiecej parametrow jak np. kolor, intensity etc.
 */
class LightSource : public Object
{

public:
    glm::mat4 lightSpaceMatrix;
    glm::vec3 lightTarget;
    glm::vec3 lightPos = glm::vec3(0.001f,3.f,0.001f);
};
/** \brief Klasa obiektu fizycznego swiatla
 */
class LightCube : public LightSource
{

public:
    glm::mat4 model = glm::mat4(1.0f);

    GLuint VBO, VAO;
    std::vector<float> vertices;

    LightCube() = default;

    LightCube(float w, float h, float l)
    {
        remodel(w,h,l);
        model = glm::translate(model,lightPos);
        model = glm::scale(model,glm::vec3(0.1f));
    }

    void remodel(float w, float h, float l)
    {
        vertices =
        {
            -w, -h, -l,  0.0f,  0.0f, -1.0f,
                w, -h, -l,   0.0f,  0.0f, -1.0f,
                w,  h, -l,   0.0f,  0.0f, -1.0f,
                w,  h, -l,   0.0f,  0.0f, -1.0f,
                -w,  h, -l,  0.0f,  0.0f, -1.0f,
                -w, -h, -l,  0.0f,  0.0f, -1.0f,

                -w, -h,  l,  0.0f,  0.0f,  1.0f,
                w, -h,  l,   0.0f,  0.0f,  1.0f,
                w,  h,  l,   0.0f,  0.0f,  1.0f,
                w,  h,  l,   0.0f,  0.0f,  1.0f,
                -w,  h,  l,  0.0f,  0.0f,  1.0f,
                -w, -h,  l,  0.0f,  0.0f,  1.0f,

                -w,  h,  l, -1.0f,  0.0f,  0.0f,
                -w,  h, -l, -1.0f,  0.0f,  0.0f,
                -w, -h, -l, -1.0f,  0.0f,  0.0f,
                -w, -h, -l, -1.0f,  0.0f,  0.0f,
                -w, -h,  l, -1.0f,  0.0f,  0.0f,
                -w,  h,  l, -1.0f,  0.0f,  0.0f,

                w,  h,  l,   1.0f,  0.0f,  0.0f,
                w,  h, -l,   1.0f,  0.0f,  0.0f,
                w, -h, -l,   1.0f,  0.0f,  0.0f,
                w, -h, -l,   1.0f,  0.0f,  0.0f,
                w, -h,  l,   1.0f,  0.0f,  0.0f,
                w,  h,  l,   1.0f,  0.0f,  0.0f,

                -w, -h, -l,  0.0f, -1.0f,  0.0f,
                w, -h, -l,   0.0f, -1.0f,  0.0f,
                w, -h,  l,   0.0f, -1.0f,  0.0f,
                w, -h,  l,   0.0f, -1.0f,  0.0f,
                -w, -h,  l,  0.0f, -1.0f,  0.0f,
                -w, -h, -l,  0.0f, -1.0f,  0.0f,

                -w,  h, -l,  0.0f,  1.0f,  0.0f,
                w,  h, -l,   0.0f,  1.0f,  0.0f,
                w,  h,  l,   0.0f,  1.0f,  0.0f,
                w,  h,  l,   0.0f,  1.0f,  0.0f,
                -w,  h,  l,  0.0f,  1.0f,  0.0f,
                -w,  h, -l,  0.0f,  1.0f,  0.0f
            };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void update(double deltaTime)
    {
        glm::mat3 rotationMatrix = glm::mat3(model);
        lightPos = glm::vec3(model * glm::vec4(glm::vec3(0.f,0.f,0.f), 1.0f));
        lightTarget = rotationMatrix * glm::vec3(0.f,-1.f,0.f); //ten drugi vector w mnozeniu to tam gdzie celuje swiatlo
        //model = glm::translate(model,glm::vec3(0.f, 0.f, 0.5f));
        //model = glm::rotate(model, 0.05f, glm::vec3(1.0f, 0.f, 0.0f));
    }

    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
    {
        glUseProgram(shaderProgram);

        glUniform3f(glGetUniformLocation(shaderProgram,"lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram,"lightPos"),1, glm::value_ptr(lightPos));

        glUseProgram(shaderPrograms[1]);
        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[1], "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

    }
};

/** \brief Obiekt wiazki, glownie uzywany do mousePickera, zoptymalizowany tak aby inverse dzielenie dzialo sie tylko raz na klatke
 */
class optimizedRay
{

public:
    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 directionInv;

    bool first = true;
    double tmin;
    int64_t closestID;

    optimizedRay() = default;

    optimizedRay(glm::vec3 org, glm::vec3 dir)
    {
        origin = org;
        direction = dir;
        directionInv = glm::vec3(1.f/dir.x,1.f/dir.y,1.f/dir.z);
        closestID = -1;
        tmin = 1000;
    }

};

/** \brief Check kolizji miedzy Ray'em a AABB
 *
 * \param optray optimizedRay& Ray
 * \param box Cube AABB
 * \param objID int64_t ID podawane w ramach determinowania najblizszego obiektu
 * \return bool Czy jest kolizja
 *
 */
bool collisonCubeRay(optimizedRay &optray, Cube box, int64_t objID)
{
    glm::vec3 cubePos = glm::vec3(box.model * glm::vec4(glm::vec3(0.f,0.f,0.f), 1.0f));
    double h,w,l;
    w = -box.vertices[0];
    h = -box.vertices[1];
    l = -box.vertices[2];

    double tx1 = (cubePos.x - w - optray.origin.x)*optray.directionInv.x;
    double tx2 = (cubePos.x + w - optray.origin.x)*optray.directionInv.x;

    double tmin = std::min(tx1, tx2);
    double tmax = std::max(tx1, tx2);

    double ty1 = (cubePos.y - h - optray.origin.y)*optray.directionInv.y;
    double ty2 = (cubePos.y + h - optray.origin.y)*optray.directionInv.y;

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    double tz1 = (cubePos.z - l - optray.origin.z)*optray.directionInv.z;
    double tz2 = (cubePos.z + l - optray.origin.z)*optray.directionInv.z;

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    bool collision = tmax >= tmin;

    if(collision)
    {
        if(optray.first)
        {
            optray.first = false;
            optray.closestID = objID;
            optray.tmin = tmin;
        }
        else
        {
            if(optray.tmin > tmin)
            {
                optray.tmin = tmin;
                optray.closestID = objID;
            }
        }
    }

    return collision;
}

#endif // METAPHYSICAL_H_INCLUDED
