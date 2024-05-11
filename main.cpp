#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <vector>
#include <list>
#include <limits>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

//#include "cameraworks.h"
#include "dijkstra.h"
#include "units.h"
#include "camerarevolting.h"
#include "boardlogic.h"
#include "mousepick.h"
#include "PlayerInterface.h"
#include "globals.h"
#include "mindless.h"
#include "shaders.h"
#include "model.h"

using namespace std;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::cerr << "OpenGL Error: " << message << std::endl;
    //abort();
}

void Game::init()
{
    testhex = Model("Modeldos/Wieza.obj");
    HexGrid = GenerateHexGrid(5);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, nullptr);

    unsigned int vertexShader;
    unsigned int fragmentShader;

    kamera = RevoltingCamera(glm::vec3(0.0f, 5.0f, -10.0f));

    glEnable(GL_DEPTH_TEST);

    // Create framebuffer
    glGenFramebuffers(1, &shadowMapFBO);

    // Create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

    // Attach depth texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    //Wylaczanie kolorow
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    createAndCompileShader("shaders/vertexS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/fragmentS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    createAndCompileShader("shaders/lightvertexS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/lightfragS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    createAndCompileShader("shaders/depthvertexS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/depthfragS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderPrograms[0]);

    createAndLoadTexture(teksturaTest,"magus.jpg");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, teksturaTest);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "texture1"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(shaderPrograms[0], "shadowMap"), 1);

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
    projection = glm::perspective(glm::radians(45.0f),  static_cast<float>(windowW)/static_cast<float>(windowH), 0.1f, 100.0f);

    mouseTrack = MousePicker(projection,view,window);

    lights.insert(make_pair<int,unique_ptr<LightSource>>(Globals::numberOfEntities++,make_unique<LightCube>(0.2f,0.2f,0.2f)));

    HexGrid[glm::vec3(0.f,0.f,0.f)].passable = 0;
    HexGrid[glm::vec3(0.f,0.f,0.f) + cube_direction_vectors[0]].passable = 0;
    HexGrid[glm::vec3(0.f,0.f,0.f) + cube_direction_vectors[3]].passable = 0;
    HexGrid[glm::vec3(0.f,0.f,0.f) + cube_direction_vectors[2]*2.f].passable = 0;
    HexGrid[glm::vec3(0.f,0.f,0.f) + cube_direction_vectors[0]*3.f].passable = 0;

    for(auto& pair : HexGrid)
    {
        obiekty[Globals::numberOfEntities++] = make_unique<Hexagon>(testhex, pair.second);
    }

    obiekty[Globals::numberOfEntities++] = make_unique<Unit>(glm::vec3(2.f,0.f,-2.f), testhex, HexGrid);

    testCube = Cube(0.15f,0.01f,0.15f,glm::vec3(0.f,2.5f,0.f));
}

void Game::input(const double deltaTime)
{
    switch(event.type)
    {
    case SDL_QUIT:
        this->shutdown = 1;
        break;
    case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_w)
            kamera.w=1;
        if(event.key.keysym.sym == SDLK_a)
            kamera.a=1;
        if(event.key.keysym.sym == SDLK_s)
            kamera.s=1;
        if(event.key.keysym.sym == SDLK_d)
            kamera.d=1;
        if(event.key.keysym.sym == SDLK_ESCAPE)
            shutdown = 1;

        break;
    case SDL_KEYUP:
        if(event.key.keysym.sym == SDLK_w)
            kamera.w=0;
        if(event.key.keysym.sym == SDLK_a)
            kamera.a=0;
        if(event.key.keysym.sym == SDLK_s)
            kamera.s=0;
        if(event.key.keysym.sym == SDLK_d)
            kamera.d=0;
        break;
    case SDL_MOUSEBUTTONDOWN:
    {
        if(event.button.button == SDL_BUTTON_LEFT)
        {
            if(playerInte.selectedID != -1)
            {
                Selectable* selectedBefore = dynamic_cast<Selectable*>(obiekty[playerInte.selectedID].get());
                selectedBefore->isSelected = false;
                playerInte.selectedID = -1;
            }
            if(!holdRotate && ray.closestID != -1)
            {
                Selectable* hovered = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
                hovered->onSelect();
                playerInte.selectedID = ray.closestID;
            }
        }

        if(event.button.button == SDL_BUTTON_RIGHT)
        {
            if(!holdRotate && ray.closestID != -1 && playerInte.selectedID != -1)
            {
                Selectable* selectedBefore = dynamic_cast<Selectable*>(obiekty[playerInte.selectedID].get());
                Selectable* target = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
                if(selectedBefore != nullptr && target != nullptr)
                {
                    selectedBefore->commandRC(target);
                }
            }
        }

        if(event.button.button == SDL_BUTTON_MIDDLE)
        {
            holdRotate = 1;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        break;
    }
    case SDL_MOUSEBUTTONUP:
    {
        if(event.button.button == SDL_BUTTON_MIDDLE)
        {
            holdRotate = 0;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        break;
    }
    case SDL_MOUSEMOTION:
    {
        SDL_GetRelativeMouseState(&mousePosx, &mousePosy);
        if(holdRotate)
        {
            kamera.ProcessMouseMovement(mousePosx, mousePosy);
            SDL_WarpMouseInWindow(window, windowW/2,windowH/2);
        }
        break;
    }
    case SDL_MOUSEWHEEL:
    {
        kamera.doZoom((float)(event.wheel.y));
        break;
    }
    }
}

void Game::update(const double deltaTime)
{
    //wtf?
    if(kamera.w)
        kamera.ProcessKeyboard(FORWARD, deltaTime);
    if(kamera.a)
        kamera.ProcessKeyboard(LEFT, deltaTime);
    if(kamera.s)
        kamera.ProcessKeyboard(BACKWARD, deltaTime);
    if(kamera.d)
        kamera.ProcessKeyboard(RIGHT, deltaTime);

    testCounter += 1*deltaTime;
    if(testCounter > 2)
        testCounter = 0;

    mouseTrack.update(view);
    ray = optimizedRay(kamera.Position,mouseTrack.getCurrentRay());

    for(const auto& pair : lights)
    {
        pair.second->update(deltaTime);
    }

    for(const auto& pair : obiekty)
    {
        Selectable* d = dynamic_cast<Selectable*>(pair.second.get());
        if (d != nullptr)
        {
            collisonCubeRay(ray,d->boundingBox,pair.first);
            d->refresh();
        }

        pair.second->update(deltaTime);
    }

    if(ray.closestID != -1)
    {
        Selectable* closest = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
        if (closest != nullptr)
        {
            closest->onHover();
        }
    }
}


void Game::render(double deltaTime)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int viewLoc = glGetUniformLocation(shaderPrograms[0], "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderPrograms[0], "projection");

    glUseProgram(shaderPrograms[0]);

    //tutaj update kamery
    view = kamera.GetViewMatrix();

    //view jest dla perspektywy
    glUniform3fv(glGetUniformLocation(shaderPrograms[0],"viewPos"),1, glm::value_ptr(kamera.Position));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderPrograms[1]);

    viewLoc = glGetUniformLocation(shaderPrograms[1], "view");
    projectionLoc = glGetUniformLocation(shaderPrograms[1], "projection");

    glUniform3fv(glGetUniformLocation(shaderPrograms[1],"viewPos"),1, glm::value_ptr(kamera.Position));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glClear( GL_COLOR_BUFFER_BIT );

    //depth map rendering

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glUseProgram(shaderPrograms[2]);
    for(const auto& pair : lights)
    {
        pair.second->bindDepthShader(shaderPrograms);
    }
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    for(const auto& pair : obiekty)
    {
        pair.second->render(shaderPrograms[2],shaderPrograms);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // reset viewport
    glViewport(0, 0, windowW, windowH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Glowny rendering
    for(const auto& pair : lights)
    {
        pair.second->render(shaderPrograms[0],shaderPrograms);
    }

    glUseProgram(shaderPrograms[0]);
    for(const auto& pair : obiekty)
    {
        pair.second->render(shaderPrograms[0],shaderPrograms);
    }

    //test czy unit dobrze widzi swoj moverange
    if(playerInte.selectedID != -1)
    {
        Unit* selectedUnit = dynamic_cast<Unit*>(obiekty[playerInte.selectedID].get());
        if (selectedUnit != nullptr)
        {
            for(auto& pair : selectedUnit->reachableHexes)
            {
                testCube.model = glm::translate(glm::mat4(1.f),getWorldPosFromHex(pair.first) + glm::vec3(0.f,1.6f,0.f));
                testCube.Draw(shaderPrograms[0],true);
            }
        }
    }
}

void Game::cleanup()
{

}

int main(int argc, char *argv[])
{
    //                          inicjalizacje podstawowych zmiennych
    srand(time(NULL));
    Game *game = new Game(1366,768,0);

    //                           deltatajm
    uint32_t prevTime = 0;
    uint32_t currTime = 0;
    double deltaTime = (currTime - prevTime)/1000.0f;

    //                           glowna petla gry

    double klatki = 1.0 / 60.0; //dzielnik = FPS

    game->init();

    while(!(game->shutdown))
    {
        game->inputCore(deltaTime);
        game->updateCore(deltaTime);
        game->renderCore(deltaTime);

        currTime = SDL_GetTicks();
        deltaTime = (currTime - prevTime)/1000.0f;
        prevTime = currTime;
        if (deltaTime < klatki)
        {
            SDL_Delay((klatki-deltaTime)*1000.0);
            currTime = SDL_GetTicks();
            deltaTime = (currTime - prevTime)/1000.0f;
            prevTime = currTime;
        }
    }

    delete game;
    return 0;
}
