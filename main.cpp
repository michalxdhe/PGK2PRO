#include "common.h"
#include "mindless.h"


using namespace std;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        std::cerr << "OpenGL Warning: " << message << std::endl;
    }
}

void Game::init()
{
    for(int i = 1; i <= numOfPlayers; i++)
    {
        playerIntes[i] = make_unique<PlayerInterface>();
    }
    initiativeGui = make_unique<InitiativeTrackerGui>(ImVec2(Globals::windowW, Globals::windowH),&initiativeQueue,&initiativeHighlightID,&initiativeHighlightFlag);

    testhex = Model("Modeldos/Wieza.obj");
    HexGrid = GenerateHexGrid(boardSize);
    UnitFactory::initialize(&HexGrid);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, nullptr);

    unsigned int vertexShader;
    unsigned int fragmentShader;

    kamera = RevoltingCamera(glm::vec3(0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 0.5f, 0.f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    createAndCompileShader("shaders/vertexS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/fragmentS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    createAndCompileShader("shaders/lightvertexS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/lightfragS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    createAndCompileShader("shaders/depthvertexS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/depthfragS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    createAndCompileShader("shaders/skyBoxVertS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/skyBoxFragS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
    projection = glm::perspective(glm::radians(45.0f),  static_cast<float>(windowW)/static_cast<float>(windowH), 0.1f, 100.0f);

    Object::viewRef = &view;
    Object::projectionRef = &projection;
    Object::obiektyRef = &obiekty;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    glm::vec3 factionColors[10] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(0.9f, 0.4f, 0.1f),
        glm::vec3(0.5f, 0.0f, 0.5f),
        glm::vec3(0.2f, 0.8f, 0.2f)
    };

    resModels[ORE] = Model("Model/Res/Ore.obj");
    resModels[GAS] = Model("Model/Res/Gas.obj");

    glUseProgram(shaderPrograms[0]);
    glUniform3fv(glGetUniformLocation(shaderPrograms[0], "factionColors"), 10, glm::value_ptr(factionColors[0]));

    mouseTrack = MousePicker(projection,view,window);

    lights.insert(make_pair<int,unique_ptr<LightSource>>(Globals::numberOfEntities++,make_unique<LightCube>(0.2f,0.2f,0.2f)));

    HexGrid[glm::vec3(0.f,0.f,0.f)].passable = 0;
    HexGrid[glm::vec3(0.f,0.f,0.f) + cube_direction_vectors[0]].passable = 0;
    HexGrid[glm::vec3(0.f,0.f,0.f) + cube_direction_vectors[3]].passable = 0;

    for(int i = 0; i < 5; i++){//zmien to potem na ile surowcuw ma byc wygenerowanych
        glm::vec3 randomHex = getRandomHex(boardSize);
        if(HexGrid[randomHex].passable && HexGrid[randomHex].presentResource == -1)
            HexGrid[randomHex].presentResource = ORE; //zmien to potem na losowy surowiec
        else{
            i--;
        }
    }

        for(int i = 0; i < 5; i++){
        glm::vec3 randomHex = getRandomHex(boardSize);
        if(HexGrid[randomHex].passable && HexGrid[randomHex].presentResource == -1)
            HexGrid[randomHex].presentResource = GAS; //zmien to potem na losowy surowiec
        else{
            i--;
        }
    }

    skyBox = Cube(50.f,50.f,50.f);

    for(auto& pair : HexGrid)
    {
        obiekty[Globals::numberOfEntities++] = make_unique<Hexagon>(testhex, pair.second);
    }

    obiekty[Globals::numberOfEntities++] = make_unique<GenericUnit>(glm::vec3(2.f,0.f,-2.f), &HexGrid, 1, Globals::numberOfEntities);
    obiekty[Globals::numberOfEntities++] = make_unique<GenericUnit>(glm::vec3(3.f,0.f,-3.f), &HexGrid, 2,Globals::numberOfEntities);
    obiekty[Globals::numberOfEntities++] = make_unique<GenericUnit>(glm::vec3(0.f,0.f,0.f), &HexGrid, 2,Globals::numberOfEntities);
    obiekty[Globals::numberOfEntities++] = make_unique<GenericUnit>(glm::vec3(4.f,0.f,-4.f), &HexGrid, 1,Globals::numberOfEntities);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    endTurn();
}

void Game::input(const double deltaTime)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    ImguiIOflag = ImGui::GetIO().WantCaptureMouse;
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
        if(event.key.keysym.sym == SDLK_SPACE)
            endTurn();

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
        if(event.button.button == SDL_BUTTON_LEFT && !ImguiIOflag)
        {
            //LightCube* testlightcube = dynamic_cast<LightCube*>(lights[0].get());
            //testlightcube->model = glm::translate(testlightcube->model,glm::vec3(0.f, 0.f, 0.5f));
            //testlightcube->model = glm::rotate(testlightcube->model, 0.05f, glm::vec3(1.0f, 0.f, 0.0f));
            //playerIntes[currentPlayersTurn]->ore++; //debug shit

            ///resolving abilities if any selected
            abilityCall tryCalling{.abilityID = -1};
            if(!holdRotate && ray.closestID != -1 && playerIntes[currentPlayersTurn]->selectedID != -1)
            {
                Unit* selectedBefore = dynamic_cast<Unit*>(obiekty[playerIntes[currentPlayersTurn]->selectedID].get());
                Selectable* target = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
                if(selectedBefore != nullptr && target != nullptr)
                {
                    if(selectedBefore->selectedAbil != -1 && selectedBefore->ID == initiativeQueue.front().ID)
                    {
                        selectedBefore->commandLC(target, &tryCalling);
                        handleAbility(tryCalling,this);
                    }
                }
            }

            ///selecting and deselecting
            if(tryCalling.abilityID == -1)
            {
                if(playerIntes[currentPlayersTurn]->selectedID != -1)
                {
                    Selectable* selectedBefore = dynamic_cast<Selectable*>(obiekty[playerIntes[currentPlayersTurn]->selectedID].get());
                    selectedBefore->isSelected = false;
                    playerIntes[currentPlayersTurn]->selectedID = -1;
                }

                if(!holdRotate && ray.closestID != -1)
                {
                    Selectable* hovered = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
                    Unit* hasOwnership = dynamic_cast<Unit*>(obiekty[ray.closestID].get());
                    if(hasOwnership == nullptr)
                    {
                        playerIntes[currentPlayersTurn]->selectedID = ray.closestID;
                        hovered->onSelect();
                    }
                    else
                    {
                        if(hasOwnership->owner == currentPlayersTurn)
                        {
                            playerIntes[currentPlayersTurn]->selectedID = ray.closestID;
                            hovered->onSelect();
                        }
                    }
                }
            }

        }

        if(event.button.button == SDL_BUTTON_RIGHT && !ImguiIOflag)
        {
            //LightCube* testlightcube = dynamic_cast<LightCube*>(lights[0].get());
            //testlightcube->model = glm::translate(testlightcube->model,glm::vec3(0.f, 0.f, -0.5f));
            //testlightcube->model = glm::rotate(testlightcube->model, -0.05f, glm::vec3(1.0f, 0.f, 0.0f));

            if(!holdRotate && ray.closestID != -1 && playerIntes[currentPlayersTurn]->selectedID != -1)
            {
                Unit* selectedBefore = dynamic_cast<Unit*>(obiekty[playerIntes[currentPlayersTurn]->selectedID].get());
                Selectable* target = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
                if(selectedBefore != nullptr && target != nullptr && selectedBefore->ID == initiativeQueue.front().ID)
                {
                    abilityCall tryCalling;
                    selectedBefore->commandRC(target, &tryCalling);
                }
            }
        }

        if(event.button.button == SDL_BUTTON_MIDDLE)
        {
            if(ray.closestID != -1)
            {
                Selectable* target = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
                if(target != nullptr)
                {
                    target->refresh();
                }
            }
            ray.closestID = -1;
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
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    vector<int> deadPlayers;
/*
    for(const auto& pair : playerIntes){
        if(pair.second.get()->commanderID == -1)
            deadPlayers.push_back(pair.first);
    }
*/
    //wtf?
    if(kamera.w)
        kamera.ProcessKeyboard(FORWARD, deltaTime);
    if(kamera.a)
        kamera.ProcessKeyboard(LEFT, deltaTime);
    if(kamera.s)
        kamera.ProcessKeyboard(BACKWARD, deltaTime);
    if(kamera.d)
        kamera.ProcessKeyboard(RIGHT, deltaTime);

    mouseTrack.update(view);
    ray = optimizedRay(kamera.Position,mouseTrack.getCurrentRay());

    for(const auto& pair : lights)
    {
        pair.second->update(deltaTime);
    }

    for(auto it = obiekty.begin(); it != obiekty.end();) {
        Unit* isUnit = dynamic_cast<Unit*>(it->second.get());

        if(isUnit != nullptr) {
            for(auto it: deadPlayers){
                if(it == isUnit->owner)
                    isUnit->takeDamage(999,true);
            }

            if(isUnit->stats.health <= 0) {
                if(isUnit->ID == playerIntes[isUnit->owner]->commanderID)
                    playerIntes[isUnit->owner]->commanderID = -1;

                if(playerIntes[currentPlayersTurn]->selectedID == isUnit->ID)
                    playerIntes[currentPlayersTurn]->selectedID  = -1;
                if(isUnit->stats.flying) {
                    HexGrid[isUnit->hexPos].airID = -1;
                    HexGrid[isUnit->hexPos].occupiedAir = false;
                } else {
                    HexGrid[isUnit->hexPos].groundID = -1;
                    HexGrid[isUnit->hexPos].occupiedGround = false;
                }
                int64_t tempID = isUnit->ID;

                //it = obiekty.erase(it);
                autoGraveyard.push_back(it->first);

                for(auto queueIt = initiativeQueue.begin(); queueIt != initiativeQueue.end(); ++queueIt) {
                    if(queueIt->ID == tempID) {
                        if(tempID == initiativeQueue.front().ID) {
                            endTurn(tempID);
                        } else {
                            initiativeQueue.erase(queueIt);
                        }
                        break;
                    }
                }
                ++it;
                continue;
            }
        }

        if(!holdRotate) {
            Selectable* d = dynamic_cast<Selectable*>(it->second.get());
            if (d != nullptr) {
                collisonCubeRay(ray, d->boundingBox, it->first);
                d->refresh();
                Globals::currentlyHoveredID = -1;
            }
        }

        it->second->update(deltaTime);

        TextParticle* isTextParticle = dynamic_cast<TextParticle*>(it->second.get());
        if(isTextParticle != nullptr)
        {
            if(isTextParticle->lifeTimer <= 0)
                autoGraveyard.push_back(it->first);
        }
        ++it;
    }


    if(ray.closestID != -1)
    {
        Selectable* closest = dynamic_cast<Selectable*>(obiekty[ray.closestID].get());
        if (closest != nullptr)
        {
            closest->onHover();
            Globals::currentlyHoveredID = ray.closestID;
        }
    }

    if(initiativeHighlightFlag && initiativeHighlightID != -1){
        Unit* isUnit = dynamic_cast<Unit*>(obiekty[initiativeHighlightID].get());
        if(isUnit != nullptr)
        {
            isUnit->hovering = 1;
        }
    }
    initiativeHighlightID = -1;
    initiativeHighlightFlag = false;


    ///GIANT hack zeby pokazac zasieg umiejetnosci
    if(playerIntes[currentPlayersTurn]->selectedID != -1){
        Unit* currentSelectedUnit = dynamic_cast<Unit*>(obiekty[playerIntes[currentPlayersTurn]->selectedID].get());
        if(currentSelectedUnit != nullptr){
            if(currentSelectedUnit->selectedAbil != -1 && Globals::currentlyHoveredID != -1){
                Hexagon* isHex = dynamic_cast<Hexagon*>(obiekty[Globals::currentlyHoveredID].get());
                if(isHex != nullptr){
                   vector<HexCell*> highlighted = getHexesFromAOE(currentSelectedUnit->hexPos,currentSelectedUnit->abilitiesAOE[currentSelectedUnit->selectedAbil],isHex->cell.LogicPos,&HexGrid);
                    for(auto it : highlighted){
                        it->moveRangeView = true;
                    }
                }
                Unit* isUnit = dynamic_cast<Unit*>(obiekty[Globals::currentlyHoveredID].get());
                if(isUnit != nullptr){
                   vector<HexCell*> highlighted = getHexesFromAOE(currentSelectedUnit->hexPos,currentSelectedUnit->abilitiesAOE[currentSelectedUnit->selectedAbil],isUnit->hexPos,&HexGrid);
                    for(auto it : highlighted){
                        it->moveRangeView = true;
                    }
                }
            }
        }
    }

    for(auto it: autoGraveyard){
        obiekty.erase(it);
        autoGraveyard.pop_back();
    }
}


void Game::render(double deltaTime)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //tutaj update kamery
    view = kamera.GetViewMatrix();

    glUseProgram(shaderPrograms[0]);

    unsigned int viewLoc = glGetUniformLocation(shaderPrograms[0], "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderPrograms[0], "projection");

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

    glUseProgram(shaderPrograms[3]);

    viewLoc = glGetUniformLocation(shaderPrograms[3], "view");
    projectionLoc = glGetUniformLocation(shaderPrograms[3], "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    ///skybox Render
    glUseProgram(shaderPrograms[3]);
    GLint OldDepthFuncMode;
    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glUniform1f(glGetUniformLocation(shaderPrograms[3],"u_time"), (float)SDL_GetTicks()/1500.f);

    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);

    skyBox.Draw(shaderPrograms[3],true);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);

    ///Glowny rendering

    initiativeGui->render(shaderPrograms[0],shaderPrograms);

    glStencilMask(0x00);

    glUseProgram(shaderPrograms[0]);
    for(const auto& pair : lights)
    {
        pair.second->render(shaderPrograms[0],shaderPrograms);
    }

    glUseProgram(shaderPrograms[0]);
    for(const auto& pair : obiekty)
    {
        pair.second->render(shaderPrograms[0],shaderPrograms);
    }

    playerIntes[currentPlayersTurn]->renderGui(shaderPrograms[0],shaderPrograms);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::cleanup()
{

}

int main(int argc, char *argv[])
{
    //                          inicjalizacje podstawowych zmiennych
    srand(time(NULL));
    Game *game = new Game(1366,768,1);

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
