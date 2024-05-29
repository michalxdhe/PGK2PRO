#ifndef MINDLESS_H_INCLUDED
#define MINDLESS_H_INCLUDED

#include "metaphysical.h"
#include "model.h"
#include "PlayerInterface.h"
#include "mousepick.h"
#include "camerarevolting.h"
#include "globals.h"
#include "units.h"
#include "customgui.h"
#include "boardlogic.h"
#include "units.cpp"
#include "customgui.cpp"


using namespace std;


inline double sign(double x)
{
    return x<0.0?-1.0:(x > 0.0 ? 1.0 : 0.0);
}

inline int sign(int x)
{
    return x<0.0?-1.0:(x > 0.0 ? 1.0 : 0.0);
}

class Game
{

public:
    SDL_Event event;
    SDL_Window *window;
    Cube testCube;
    double testCounter = 0;

    bool ImguiIOflag = false;

    SDL_GLContext gContext;

    Model testhex;

    optimizedRay ray;

    int mousePosx, mousePosy;

    MousePicker mouseTrack;

    unique_ptr<PlayerInterface> playerInte;
    unique_ptr<InitiativeTrackerGui> initiativeGui;

    unordered_map<glm::vec3, HexCell> HexGrid;

    int windowH, windowW;
    vector<unsigned int> shaderPrograms;
    RevoltingCamera kamera;

    GLuint shadowMapFBO;
    GLuint depthMap;
    unsigned int teksturaTest;

    ///Widok Kamery
    glm::mat4 view = glm::mat4(1.0f);
    ///FoV oraz AspectRatio dla kamery
    glm::mat4 projection = glm::mat4(1.0f);;

    deque<Unit> initiativeQueue;
    deque<Unit> initiativeQueueTemp;

    unordered_map<int, unique_ptr<LightSource>> lights;
    unordered_map<int, unique_ptr<Object>> obiekty;

    ///Flaga stanowiaca czy pora isc spac
    bool shutdown = 0;

    bool holdRotate = 0;

    void cleanup();

    Game()
    {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );

        window = SDL_CreateWindow("siema",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        SDL_SetRelativeMouseMode(SDL_FALSE);

        //SDL_ShowCursor(SDL_DISABLE);
        gContext = SDL_GL_CreateContext(window);
        glewInit();
        SDL_GL_SetSwapInterval(0);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(window, gContext);
        ImGui_ImplOpenGL3_Init("#version 330");

        glClearColor( 0.f, 0.f, 0.f, 1.f );

        windowH = 480;
        windowW = 640;
        Globals::windowH = windowH;
        Globals::windowW = windowW;
    }

    Game(int w, int h, int vsync)
    {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );

        window = SDL_CreateWindow("siema",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        SDL_SetRelativeMouseMode(SDL_FALSE);

        gContext = SDL_GL_CreateContext(window);
        glewInit();
        SDL_GL_SetSwapInterval(sign(vsync));

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(window, gContext);
        ImGui_ImplOpenGL3_Init("#version 330");

        glClearColor( 0.1f, 0.1f, 0.2f, 1.f );



        windowH = h;
        windowW = w;
        Globals::windowH = windowH;
        Globals::windowW = windowW;
    }

    ~Game()
    {
        SDL_DestroyWindow(this->window);
        SDL_GL_DeleteContext(gContext);
        window = NULL;
        SDL_Quit();

        cleanup();
    }

    bool inputCore(double deltaTime);
    bool updateCore(double deltaTime);
    bool renderCore(double deltaTime);

    void init();
    void input(double deltaTime);
    void update(double deltaTime);
    void render(double deltaTime);
};

bool Game::inputCore(double deltaTime)
{
    while(SDL_PollEvent(&event))
    {
        input(deltaTime);
    }
    return 1;
}

bool Game::updateCore(double deltaTime)
{
    update(deltaTime);

    return 1;
}

bool Game::renderCore(double deltaTime)
{
    render(deltaTime);

    SDL_GL_SwapWindow(window);
    return 1;
}

#endif // MINDLESS_H_INCLUDED
