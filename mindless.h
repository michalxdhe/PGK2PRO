#ifndef MINDLESS_H_INCLUDED
#define MINDLESS_H_INCLUDED

#include "animator.h"
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
#include "particles.h"
#include "fluidSim.h"

using namespace std;

enum CursorTextures{
    IDLE,
    CLICKED,
    CURSORTEXCOUNT
};


/** \brief  Taka tam pomocna funkcja co zwraca znak -1,0,1 dla double'a
 *
 * \param x double
 * \return double
 *
 */
inline double sign(double x)
{
    return x<0.0?-1.0:(x > 0.0 ? 1.0 : 0.0);
}

/** \brief Taka tam pomocna funkcja co zwraca znak -1,0,1 dla int'a
 *
 * \param x int
 * \return int
 *
 */
inline int sign(int x)
{
    return x<0.0?-1.0:(x > 0.0 ? 1.0 : 0.0);
}

/** \brief Klasa definiujaca instancje gry/symulacji.
 *  https://youtu.be/mhoRxWGh1z4?si=J4lUP3Y92_supNFK
 *
 */
class Game
{

public:
    /// Obiekt eventów SDL'a
    SDL_Event event;
    /// Obiekt okna SDL'a
    SDL_Window *window;
    /// Flaga uzywana do okreslenia czy ImGui aktualnie obsluguje input czy nie
    bool ImguiIOflag = false;

    SDL_GLContext gContext;

    ///hack dla kursora
    SDL_Cursor* sdlCursors[CURSORTEXCOUNT];
    bool cursorVisible = true;

    /// Ray dla MousePick'era
    optimizedRay ray;

    /// Integer'y przechowywujace pozycje myszy, w Globals:: namespace jest te¿ globalna wersja chyba
    int mousePosx, mousePosy;

    int boardSize;

    /// Obiekt mousepicker'a
    MousePicker mouseTrack;

    /// Info o ID gracza ktorego aktualnie jest tura
    int currentPlayersTurn;

    /// Info o liczbie graczy
    int numOfPlayers;

    /// Jakis hack z tego co pamietam zeby podswietlic unit'a na initiative trackerze
    int64_t initiativeHighlightID = -1;
    bool initiativeHighlightFlag = false;

    ///Mapa interfejsow grajacych graczy
    unordered_map<int, unique_ptr<PlayerInterface>> playerIntes;
    ///GUI inicjatywy
    unique_ptr<InitiativeTrackerGui> initiativeGui;
    ///GUI PauseMenu
    int soundVolume = 100;
    unique_ptr<PauseMenu> pauseMenuChamp;

    FluidSim *simA = nullptr;

    ///Map'a grida, nwm to wpadl na to zeby uzywac float glm::vec3 jako key
    unordered_map<glm::vec3, HexCell> HexGrid;

    int windowH, windowW;
    vector<unsigned int> shaderPrograms;
    ///wektor uzywany do seperacji usuwania obiektow od aktywnego programu
    vector<int> autoGraveyard;

    ///Kamera wpatrzona w punkt
    RevoltingCamera kamera;

    Cube skyBox;

    ///Widok Kamery
    glm::mat4 view = glm::mat4(1.0f);
    ///FoV oraz AspectRatio dla kamery
    glm::mat4 projection = glm::mat4(1.0f);;

    ///Kolejka inicjatywy
    deque<Unit> initiativeQueue;
    deque<Unit> initiativeQueueTemp;

    ///Glowna map'a swiatel
    unordered_map<int, unique_ptr<LightSource>> lights;
    ///Glowna mapa obiektow
    unordered_map<int, unique_ptr<Object>> obiekty;
    ///Partikles
    std::vector<Particle> particles;
    std::array<GLuint,7> fluidShaders;

    ///Flaga stanowiaca czy pora isc spac
    bool shutdown = 0;

    bool pause = 0;

    bool holdRotate = 0;

    /** \brief Metoda do dekonstruowania Game'a
     *
     * \return void
     *
     */
    void cleanup();

    /** \brief Konstruktor gry domyœlny
     *
     *
     */
    Game()
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
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

        numOfPlayers = 4;
        currentPlayersTurn = (rand()%2) + 1;

        boardSize = 8;
    }

    /** \brief Konstruktor gry z parametrami
     *
     * \param w int Szerokosc okna
     * \param h int Wysokosc okna
     * \param vsync int Vsync: x < 1 = off else on
     *
     */
    Game(int w, int h, int vsync)
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
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

        numOfPlayers = 4;
        currentPlayersTurn = (rand()%2) + 1;

        boardSize = 8;
    }

    ~Game()
    {
        SDL_DestroyWindow(this->window);
        SDL_GL_DeleteContext(gContext);
        window = NULL;
        SDL_Quit();

        cleanup();
    }

    /** \brief Rdzenny input, w ramach organizacji
     *
     * \param deltaTime double deltatime pobrany z main'a
     * \return bool czy udala sie klatka
     *
     */
    bool inputCore(double deltaTime, uint32_t t);

    /** \brief Rdzenny input, w ramach organizacji
     *
     * \param deltaTime double deltatime pobrany z main'a
     * \return bool czy udala sie klatka
     *
     */
    bool updateCore(double deltaTime, uint32_t t);

    /** \brief Rdzenny update, w ramach organizacji
     *
     * \param deltaTime double deltatime pobrany z main'a
     * \return bool czy udala sie klatka
     *
     */
    bool renderCore(double deltaTime, uint32_t t);

    /** \brief Metoda do inicjalizacji rzeczy
     *
     * \return void
     *
     */
    void init();

    /** \brief Input
     *
     * \param deltaTime double deltatime pobrany z main'a
     * \return bool czy udala sie klatka
     *
     */
    void input(double deltaTime, uint32_t t);

    /** \brief Update
     *
     * \param deltaTime double deltatime pobrany z main'a
     * \return bool czy udala sie klatka
     *
     */
    void update(double deltaTime, uint32_t t);

    /** \brief Render
     *
     * \param deltaTime double deltatime pobrany z main'a
     * \return bool czy udala sie klatka
     *
     */
    void render(double deltaTime, uint32_t t);

private:
    void endGame()
    {

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
            if(!initiativeQueue.empty())
            {
                Unit* eotUnit = dynamic_cast<Unit*>(obiekty[(initiativeQueue.front().ID)].get());
                if(eotUnit != nullptr)
                {
                    eotUnit->stats.yourTurn = true;
                    eotUnit->resolveStartOfTurn();
                    int resourceOnHex = (*eotUnit->hexGrid)[eotUnit->hexPos].presentResource;
                    if(resourceOnHex != -1)
                    {
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
                for(auto it: autoGraveyard)
                {
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

            if(!initiativeQueueTemp.empty())
            {
                sort(initiativeQueueTemp.begin(),initiativeQueueTemp.end(),[](const Unit& a, const Unit& b)
                {
                    return a.stats.speed - a.stats.effects[SLOW].intensity > b.stats.speed - a.stats.effects[SLOW].intensity;
                });
                initiativeQueue = initiativeQueueTemp;
                Unit* eotUnit = dynamic_cast<Unit*>(obiekty[(initiativeQueue.front().ID)].get());
                if(eotUnit != nullptr)
                {
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

    void doAttack(abilityCall info, Game *gameRef)
    {
        int damage = max(0,info.culprit->stats.att);
        for(auto it : info.target)
        {
            int targetID = info.culprit->stats.flying ?  it->airID : it->groundID;
            if(targetID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[targetID].get());
                if(inZone != nullptr)
                {
                    if(info.culprit->stats.flying == inZone->stats.flying)///Redundant
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
        }
    }

    bool doCreate(abilityCall info, Game *gameRef)
    {
        Unit offspringTemp;
        bool status = false;
        if(info.offSpring != -1 && info.offSpring < UNIT_TYPE_COUNT)
        {
            offspringTemp = *(UnitFactory::unitCreationMap[info.offSpring](glm::vec3(100.f),info.culprit->hexGrid,info.culprit->owner,-1));
            for(auto it : info.target)
            {
                if(it->passable)
                {
                    if(offspringTemp.stats.flying)
                    {
                        if(it->airID == -1 && !it->occupiedAir)
                        {
                            status = true;
                            UnitFactory::createUnit(info.offSpring,it->LogicPos,info.culprit->owner);
                        }
                    }
                    else
                    {
                        if(it->groundID == -1 && !it->occupiedGround)
                        {
                            status = true;
                            UnitFactory::createUnit(info.offSpring,it->LogicPos,info.culprit->owner);
                        }
                    }
                }
            }
        }
        UnitFactory::resolveCreation(&gameRef->obiekty,&gameRef->initiativeQueue);

        return status;
    }

    bool doMorph(abilityCall info, Game *gameRef)
    {
        bool status = false;
        HexCell &culpritOrigin = (*info.culprit->hexGrid)[info.culprit->hexPos];
        Unit offspringTemp;
        if(info.offSpring != -1 && info.offSpring < UNIT_TYPE_COUNT)
        {
            offspringTemp = *(UnitFactory::unitCreationMap[info.offSpring](glm::vec3(100.f),info.culprit->hexGrid,info.culprit->owner,-1));

            if(!info.culprit->stats.flying)
            {
                culpritOrigin.groundID = -1;
                culpritOrigin.occupiedGround = false;
            }
            else
            {
                culpritOrigin.airID = -1;
                culpritOrigin.occupiedAir = false;
            }

            if(offspringTemp.stats.flying)
            {
                if(culpritOrigin.airID == -1 && !culpritOrigin.occupiedAir && culpritOrigin.passable)
                {
                    status = true;
                    UnitFactory::createUnit(info.offSpring,info.culprit->hexPos,info.culprit->owner);
                }
            }
            else
            {
                if(culpritOrigin.groundID == -1 && !culpritOrigin.occupiedGround && culpritOrigin.passable)
                {
                    status = true;
                    UnitFactory::createUnit(info.offSpring,info.culprit->hexPos,info.culprit->owner);
                }
            }
        }
        if(status)
            info.culprit->stats.health = 0;
        else
        {
            if(!info.culprit->stats.flying)
            {
                culpritOrigin.groundID = info.culprit->ID;
                culpritOrigin.occupiedGround = true;
            }
            else
            {
                culpritOrigin.airID = info.culprit->ID;
                culpritOrigin.occupiedAir = true;
            }
        }

        UnitFactory::resolveCreation(&gameRef->obiekty,&gameRef->initiativeQueue);
        return status;
    }

    void doSpit(abilityCall info, Game *gameRef)
    {
        int damage = 1;
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
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
    }

    void doHex(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    inZone->stats.health -= 1;
                    inZone->stats.maxHealth -= 1;
                    inZone->stats.speed -= 1;
                    inZone->stats.att -= 1;
                    inZone->stats.def -= 1;
                    inZone->stats.maxMovRange -= 1;
                    inZone->stats. movRange -= 1;
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        inZone->stats.effects[i].duration += info.effects[i].duration;
                        inZone->stats.effects[i].intensity += info.effects[i].intensity;
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    inZone->stats.health -= 1;
                    inZone->stats.maxHealth -= 1;
                    inZone->stats.speed -= 1;
                    inZone->stats.att -= 1;
                    inZone->stats.def -= 1;
                    inZone->stats.maxMovRange -= 1;
                    inZone->stats. movRange -= 1;
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        inZone->stats.effects[i].duration += info.effects[i].duration;
                        inZone->stats.effects[i].intensity += info.effects[i].intensity;
                    }
                }
            }
        }
    }

    void doDecimate(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    inZone->stats.def -= 6;
                    inZone->takeDamage(6);
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        inZone->stats.effects[i].duration += info.effects[i].duration;
                        inZone->stats.effects[i].intensity += info.effects[i].intensity;
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    inZone->stats.def -= 6;
                    inZone->takeDamage(2);
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        inZone->stats.effects[i].duration += info.effects[i].duration;
                        inZone->stats.effects[i].intensity += info.effects[i].intensity;
                    }
                }
            }
        }
    }

    void doCleanse(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        if(i == HEAL)
                            continue;
                        inZone->stats.effects[i].duration = max(0,inZone->stats.effects[i].duration - 2);
                        inZone->stats.effects[i].intensity = max(0,inZone->stats.effects[i].intensity - 2);
                    }
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        inZone->stats.effects[i].duration += info.effects[i].duration;
                        inZone->stats.effects[i].intensity += info.effects[i].intensity;
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        if(i == HEAL)
                            continue;
                        inZone->stats.effects[i].duration = max(0,inZone->stats.effects[i].duration - 2);
                        inZone->stats.effects[i].intensity = max(0,inZone->stats.effects[i].intensity - 2);
                    }
                    for(int i = 0; i < EFFECTS_COUNT; i++)
                    {
                        inZone->stats.effects[i].duration += info.effects[i].duration;
                        inZone->stats.effects[i].intensity += info.effects[i].intensity;
                    }
                }
            }
        }
    }

    void doRally(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.att += 2;
                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.att += 2;
                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
        }
    }

    void doFortify(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.def += 2;
                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.def += 2;
                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
        }
    }

    void doSpeedBoon(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.speed += 3;
                        inZone->stats.movRange += 1;
                        inZone->stats.maxMovRange +=1;

                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.speed += 3;
                        inZone->stats.movRange += 1;
                        inZone->stats.maxMovRange +=1;

                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
        }
    }

    void doHealthBoon(abilityCall info, Game *gameRef)
    {
        for(auto it : info.target)
        {
            if(it->airID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->airID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.maxHealth += 2;

                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
            if(it->groundID != -1)
            {
                Unit* inZone = dynamic_cast<Unit*>(gameRef->obiekty[it->groundID].get());
                if(inZone != nullptr)
                {
                    if(inZone->owner == info.culprit->owner)
                    {
                        inZone->stats.maxHealth += 2;
                        for(int i = 0; i < EFFECTS_COUNT; i++)
                        {
                            inZone->stats.effects[i].duration += info.effects[i].duration;
                            inZone->stats.effects[i].intensity += info.effects[i].intensity;
                        }
                    }
                }
            }
        }
    }


    bool handleAbility(abilityCall info, Game *gameRef)
    {
        switch(info.abilityID)
        {
        case ATTACK:
            if(info.culprit->stats.actionTokens >= 1)
            {
                doAttack(info,gameRef);
                info.culprit->stats.actionTokens -= 1;
                return true;
            }
            else
                return false;
            break;
        case CREATE:
        {
            Unit offspringTemp;
            if(info.offSpring != -1 && info.offSpring < UNIT_TYPE_COUNT)
            {
                offspringTemp = *(UnitFactory::unitCreationMap[info.offSpring](glm::vec3(100.f),info.culprit->hexGrid,info.culprit->owner,-1));

                bool hasTheResources = true;

                for(int i = 0; i < RESOURCE_COUNT; i++)
                {
                    if(playerIntes[info.culprit->owner]->resources[i] < offspringTemp.stats.cost[i])
                    {
                        hasTheResources = false;
                    }
                }

                if(info.culprit->stats.actionTokens >= offspringTemp.stats.buildActionPointCost && hasTheResources)
                {
                    if(doCreate(info,gameRef))
                    {
                        info.culprit->stats.actionTokens -= offspringTemp.stats.buildActionPointCost;
                        for(int i = 0; i < RESOURCE_COUNT; i++)
                        {
                            playerIntes[info.culprit->owner]->resources[i] = max(0,playerIntes[info.culprit->owner]->resources[i] - offspringTemp.stats.cost[i]);
                        }
                        return true;
                    }
                }
            }
            return false;
            break;
        }
        case MORPH:
        {
            Unit offspringTemp;
            if(info.offSpring != -1 && info.offSpring < UNIT_TYPE_COUNT)
            {
                offspringTemp = *(UnitFactory::unitCreationMap[info.offSpring](glm::vec3(100.f),info.culprit->hexGrid,info.culprit->owner,-1));

                bool hasTheResources = true;

                for(int i = 0; i < RESOURCE_COUNT; i++)
                {
                    if(playerIntes[info.culprit->owner]->resources[i] < offspringTemp.stats.cost[i])
                    {
                        hasTheResources = false;
                    }
                }

                if(info.culprit->stats.actionTokens >= offspringTemp.stats.buildActionPointCost && hasTheResources)
                {
                    if(doMorph(info,gameRef))
                    {
                        info.culprit->stats.actionTokens -= offspringTemp.stats.buildActionPointCost;
                        for(int i = 0; i < RESOURCE_COUNT; i++)
                        {
                            playerIntes[info.culprit->owner]->resources[i] = max(0,playerIntes[info.culprit->owner]->resources[i] - offspringTemp.stats.cost[i]);
                        }
                    }
                    return true;
                }
            }
            return false;
            break;
        }
        case SPIT:
        {
            if(info.culprit->stats.actionTokens >= 2)
            {
                doSpit(info,gameRef);
                info.culprit->stats.actionTokens -= 2;
                return true;
            }
            else
                return false;
            break;
        }
        case HEX:
        {
            if(info.culprit->stats.actionTokens >= 4)
            {
                doHex(info,gameRef);
                info.culprit->stats.actionTokens -= 4;
                return true;
            }
            else
                return false;
            break;
        }
        case DECIMATE:
        {
            if(info.culprit->stats.actionTokens >= 3)
            {
                doDecimate(info,gameRef);
                info.culprit->stats.actionTokens -= 3;
                return true;
            }
            else
                return false;
            break;
        }
        case CLEANSE:
        {
            if(info.culprit->stats.actionTokens >= 2)
            {
                doCleanse(info,gameRef);
                info.culprit->stats.actionTokens -= 2;
                return true;
            }
            else
                return false;
            break;
        }
        case RALLY:
        {
            if(info.culprit->stats.actionTokens >= 3)
            {
                doRally(info,gameRef);
                info.culprit->stats.actionTokens -= 3;
                return true;
            }
            else
                return false;
            break;
        }
        case FORTIFY:
        {
            if(info.culprit->stats.actionTokens >= 3)
            {
                doFortify(info,gameRef);
                info.culprit->stats.actionTokens -= 3;
                return true;
            }
            else
                return false;
            break;
        }
        case SPEEDBOON:
        {
            if(info.culprit->stats.actionTokens >= 2)
            {
                doSpeedBoon(info,gameRef);
                info.culprit->stats.actionTokens -= 2;
                return true;
            }
            else
                return false;
            break;
        }
        case HEALTHBOON:
        {
            if(info.culprit->stats.actionTokens >= 2)
            {
                doHealthBoon(info,gameRef);
                info.culprit->stats.actionTokens -= 2;
                return true;
            }
            else
                return false;
            break;
        }
        }
        return false;
    }

};

bool Game::inputCore(double deltaTime, uint32_t t)
{
    while(SDL_PollEvent(&event))
    {
        input(deltaTime, t);
    }
    return 1;
}

bool Game::updateCore(double deltaTime, uint32_t t)
{
    update(deltaTime, t);

    return 1;
}

bool Game::renderCore(double deltaTime, uint32_t t)
{
    render(deltaTime, t);

    SDL_GL_SwapWindow(window);
    return 1;
}


#endif // MINDLESS_H_INCLUDED
