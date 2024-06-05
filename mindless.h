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
#include "abilities.h"

using namespace std;

struct UnitFactory{
        inline static deque<unique_ptr<Unit>> toBeCreated;
        inline static unordered_map<glm::vec3, HexCell> *HexGridRef;
        inline static unordered_map<UnitType, function<unique_ptr<Unit>(glm::vec3, unordered_map<glm::vec3, HexCell>*, int)>> unitCreationMap;

        static void initialize(unordered_map<glm::vec3, HexCell> *HexGrid) {
            HexGridRef = HexGrid;

            unitCreationMap[GENERIC_UNIT] = [](glm::vec3 hexCellCords, unordered_map<glm::vec3, HexCell>* HexGrid, int factionID) {
                return std::make_unique<GenericUnit>(hexCellCords, HexGrid, factionID, 0);
            };

        }

        static void createUnit(UnitType unitType, glm::vec3 hexCellCords, int factionID){
            auto it = unitCreationMap.find(unitType);
            if (it != unitCreationMap.end()) {
                toBeCreated.push_back(it->second(hexCellCords, HexGridRef, factionID));
            } else {
                std::cerr << "Error: Unknown unit type." << std::endl;
            }
        }

        static void resolveCreation(unordered_map<int, unique_ptr<Object>> *obiekty){
            while(!toBeCreated.empty()){
                int ID = Globals::numberOfEntities++;
                toBeCreated.front().get()->ID = ID;
                (*obiekty)[ID] = move(toBeCreated.front());
                toBeCreated.pop_front();
            }
        }
};

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

    bool ImguiIOflag = false;

    SDL_GLContext gContext;

    Model testhex;

    optimizedRay ray;

    int mousePosx, mousePosy;

    int boardSize;

    MousePicker mouseTrack;
    int currentPlayersTurn;
    int numOfPlayers;
    int64_t initiativeHighlightID = -1;
    bool initiativeHighlightFlag = false;

    unordered_map<int, unique_ptr<PlayerInterface>> playerIntes;
    unique_ptr<InitiativeTrackerGui> initiativeGui;

    unordered_map<glm::vec3, HexCell> HexGrid;

    int windowH, windowW;
    vector<unsigned int> shaderPrograms;
    vector<int> autoGraveyard;
    RevoltingCamera kamera;

    Cube skyBox;

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

        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
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
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(window, gContext);
        ImGui_ImplOpenGL3_Init("#version 330");

        glClearColor( 0.f, 0.f, 0.f, 1.f );

        windowH = 480;
        windowW = 640;
        Globals::windowH = windowH;
        Globals::windowW = windowW;

        numOfPlayers = 2;
        currentPlayersTurn = (rand()%2) + 1;

        boardSize = 5;
    }

    Game(int w, int h, int vsync)
    {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
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
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(window, gContext);
        ImGui_ImplOpenGL3_Init("#version 330");

        glClearColor( 0.1f, 0.1f, 0.2f, 1.f );

        windowH = h;
        windowW = w;
        Globals::windowH = windowH;
        Globals::windowW = windowW;

        numOfPlayers = 2;
        currentPlayersTurn = (rand()%2) + 1;

        boardSize = 5;
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

private:
    void endGame(){

    }

    void endTurn(int deadID = -1)
    {
        if(!initiativeQueue.empty())
        {
            Unit* eotUnit = dynamic_cast<Unit*>(obiekty[(initiativeQueue.front().ID)].get());
            if(eotUnit != nullptr)
                eotUnit->stats.yourTurn = false;
            initiativeQueue.pop_front();
            if(playerIntes[currentPlayersTurn]->selectedID != -1)
            {
                Selectable* selectedBefore = dynamic_cast<Selectable*>(obiekty[playerIntes[currentPlayersTurn]->selectedID].get());
                if(selectedBefore != nullptr)
                {
                    selectedBefore->isSelected = 0;
                }
            }

            playerIntes[currentPlayersTurn]->selectedID = -1;
            if(!initiativeQueue.empty()){
                Unit* eotUnit = dynamic_cast<Unit*>(obiekty[(initiativeQueue.front().ID)].get());
                if(eotUnit != nullptr){
                    eotUnit->stats.yourTurn = true;
                    eotUnit->resolveStartOfTurn();
                    int resourceOnHex = (*eotUnit->hexGrid)[eotUnit->hexPos].presentResource;
                    if(resourceOnHex != -1){
                        playerIntes[eotUnit->owner]->resources[resourceOnHex] += eotUnit->stats.miningCapability;
                    }
                    currentPlayersTurn = initiativeQueue.front().owner;
                }
            }
        }

        if(initiativeQueue.empty())
        {
            initiativeQueueTemp.clear();
            for(const auto& pair : obiekty)
            {
                bool foundDead = false;
                for(auto it: autoGraveyard){
                    if(it == pair.first)
                        foundDead = true;
                }
                if(foundDead)
                    continue;
                Unit* isUnit = dynamic_cast<Unit*>(pair.second.get());
                if(isUnit != nullptr)
                {
                    initiativeQueueTemp.push_back(*isUnit);
                    isUnit->stats.movRange = isUnit->stats.maxMovRange;
                    isUnit->stats.actionTokens = min(isUnit->stats.maxActionTokens,isUnit->stats.actionTokens+1);
                    endTurnEffects effects; ///placeholder, to powinno byc w kazdym interface gracza, lub w Game
                    isUnit->resolveEndOfTurn(effects);
                }
            }

            if(!initiativeQueueTemp.empty()){
                sort(initiativeQueueTemp.begin(),initiativeQueueTemp.end(),[](const Unit& a, const Unit& b)
                {
                    return a.stats.speed - a.stats.effects[SLOW].intensity > b.stats.speed - a.stats.effects[SLOW].intensity;
                });
                initiativeQueue = initiativeQueueTemp;
                Unit* eotUnit = dynamic_cast<Unit*>(obiekty[(initiativeQueue.front().ID)].get());
                if(eotUnit != nullptr){
                    eotUnit->resolveStartOfTurn();
                    int resourceOnHex = (*eotUnit->hexGrid)[eotUnit->hexPos].presentResource;
                    if(resourceOnHex != -1)
                        playerIntes[eotUnit->owner]->resources[resourceOnHex] += eotUnit->stats.miningCapability;
                    eotUnit->stats.yourTurn = true;
                    currentPlayersTurn = initiativeQueue.front().owner;
                }
            }
        }
    }

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

void doAttack(abilityCall info, Game *gameRef)
{
    int damage = info.culprit->stats.att;
    for(auto it : info.target)
    {
        if(it->groundID != -1)
        {
            Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
            if(inZone != nullptr)
            {
                inZone->takeDamage(damage);
                for(int i = 0; i < EFFECTS_COUNT; i++)
                {
                    inZone->stats.effects[i].duration += info.effects[i].duration;
                    inZone->stats.effects[i].intensity += info.effects[i].intensity;
                }
            }
        }
    }
    //info.culprit->updateMovRange();
}

void doMissile(abilityCall info, Game *gameRef)
{
    int damage = info.culprit->stats.att;
    for(auto it : info.target)
    {
        if(it->groundID != -1)
        {
            Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
            if(inZone != nullptr)
            {
                inZone->takeDamage(damage);
                for(int i = 0; i < EFFECTS_COUNT; i++)
                {
                    inZone->stats.effects[i].duration += info.effects[i].duration;
                    inZone->stats.effects[i].intensity += info.effects[i].intensity;
                }
            }
        }
        it->presentResource = ORE;
    }
    //info.culprit->updateMovRange();
}

bool handleAbility(abilityCall info, Game *gameRef)
{
    switch(info.abilityID)
    {
    case ATTACK:
        if(info.culprit->stats.actionTokens >= 1){
            doAttack(info,gameRef);
            info.culprit->stats.actionTokens -= 1;
            return true;
        }else
        return false;
        break;
    case CREATE:

        return false;
        break;
    case MISSILE:
        if(info.culprit->stats.actionTokens >= 2){
            doMissile(info,gameRef);
            info.culprit->stats.actionTokens -= 2;
            return true;
        }else
        return false;
        break;
    }
    return false;
}

#endif // MINDLESS_H_INCLUDED
