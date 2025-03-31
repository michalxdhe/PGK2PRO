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
    audioJungle.loadAudio("audioStuff/grunt1.wav");

    glEnable(GL_PROGRAM_POINT_SIZE);

    for (unsigned int i = 0; i < nr_particles; ++i)
        particles.push_back(Particle());

    initParticles();

    createAndLoadTexture(teksturaDymu,"uiTextures/smok.png", false);

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

    createAndCompileShader("shaders/particleVertS.c",GL_VERTEX_SHADER,vertexShader);
    createAndCompileShader("shaders/particleFragS.c",GL_FRAGMENT_SHADER,fragmentShader);
    shaderPrograms.push_back(createProgram(vertexShader,fragmentShader));

    //glUseProgram(shaderPrograms[4]);
    //glUniform1i(glGetUniformLocation(shaderPrograms[4],"dym"),teksturaDymu);

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

    ///inicjalizowanie modeli, we to w funkcje jakas walnij
    ///MISC
    resModels[ORE] = Model("Model/Res/Ore.obj");
    resModels[GAS] = Model("Model/Res/Gas.obj");
    createAndLoadTexture(testOverlay,"uiTextures/UnitInfo.png",false);

    ///UNIT'Y
    unitModels[LARVE] = {"Model/GenericTest/larve.gltf", Model("Model/GenericTest/larve.gltf")};
    unitModels[EGG] = {"Model/GenericTest/egg.gltf", Model("Model/GenericTest/egg.gltf")};
    unitModels[CENTI] = {"Model/GenericTest/Centi.gltf", Model("Model/GenericTest/Centi.gltf")};
    unitModels[BIRD] = {"Model/GenericTest/birdOfPrey.gltf", Model("Model/GenericTest/birdOfPrey.gltf")};
    unitModels[MORTARBUG] = {"Model/GenericTest/mortarBug.gltf", Model("Model/GenericTest/mortarBug.gltf")};
    unitModels[COLLECTOR] = {"Model/GenericTest/weirdAssCollector.gltf", Model("Model/GenericTest/weirdAssCollector.gltf")};
    unitModels[BALLER] = {"Model/GenericTest/spiderBaller.gltf", Model("Model/GenericTest/spiderBaller.gltf")};
    unitModels[COMM] = {"Model/GenericTest/commanderLazy.gltf", Model("Model/GenericTest/commanderLazy.gltf")};

    glUseProgram(shaderPrograms[0]);
    glUniform3fv(glGetUniformLocation(shaderPrograms[0], "factionColors"), 10, glm::value_ptr(factionColors[0]));

    mouseTrack = MousePicker(projection,view,window);

    lights.insert(make_pair<int,unique_ptr<LightSource>>(Globals::numberOfEntities++,make_unique<LightCube>(0.2f,0.2f,0.2f)));

    for(int i = 0; i < boardSize*4; i++){
        glm::vec3 randomHex = getRandomHex(boardSize);
        HexGrid[randomHex].passable = false;
    }

    for(int i = 0; i < 15; i++)
        {//zmien to potem na ile surowcuw ma byc wygenerowanych
            glm::vec3 randomHex = getRandomHex(boardSize);
            if(HexGrid[randomHex].passable && HexGrid[randomHex].presentResource == -1)
                HexGrid[randomHex].presentResource = ORE; //zmien to potem na losowy surowiec
            else{
                i--;
            }
        }

        for(int i = 0; i < 15; i++)
        {
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

    /*for(int i = 0; i < 100; i++)
    {
        glm::vec3 randomHex = getRandomHex(boardSize);
        obiekty[Globals::numberOfEntities++] = make_unique<Larve>(randomHex, &HexGrid, 1, Globals::numberOfEntities);
    }*/

    HexGrid[cube_direction_vectors[0]*(float)boardSize].passable = true;
    HexGrid[cube_direction_vectors[1]*(float)boardSize].passable = true;
    HexGrid[cube_direction_vectors[3]*(float)boardSize].passable = true;
    HexGrid[cube_direction_vectors[4]*(float)boardSize].passable = true;

    obiekty[Globals::numberOfEntities++] = make_unique<LazyComm>(cube_direction_vectors[0]*(float)boardSize, &HexGrid, 1, Globals::numberOfEntities);
    playerIntes[1]->commanderID = Globals::numberOfEntities -1; //-1 no bo post incrementacja etc.

    obiekty[Globals::numberOfEntities++] = make_unique<LazyComm>(cube_direction_vectors[1]*(float)boardSize, &HexGrid, 2,Globals::numberOfEntities);
    playerIntes[2]->commanderID = Globals::numberOfEntities -1; //-1 no bo post incrementacja etc.

    obiekty[Globals::numberOfEntities++] = make_unique<LazyComm>(cube_direction_vectors[3]*(float)boardSize, &HexGrid, 3, Globals::numberOfEntities);
    playerIntes[3]->commanderID = Globals::numberOfEntities -1; //-1 no bo post incrementacja etc.

    obiekty[Globals::numberOfEntities++] = make_unique<LazyComm>(cube_direction_vectors[4]*(float)boardSize, &HexGrid, 4,Globals::numberOfEntities);
    playerIntes[4]->commanderID = Globals::numberOfEntities -1; //-1 no bo post incrementacja etc.

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
    //auto start = std::chrono::high_resolution_clock::now();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    vector<int> deadPlayers;

    for(const auto& pair : playerIntes){
        if(pair.second.get()->commanderID == -1)
            deadPlayers.push_back(pair.first);
    }

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
                            if(isUnit->ID == HexGrid[isUnit->hexPos].airID){
                                HexGrid[isUnit->hexPos].airID = -1;
                                HexGrid[isUnit->hexPos].occupiedAir = false;
                            }
                    } else {
                            if(isUnit->ID == HexGrid[isUnit->hexPos].groundID){
                                HexGrid[isUnit->hexPos].groundID = -1;
                                HexGrid[isUnit->hexPos].occupiedGround = false;
                            }
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
    ///TO:DO we no ogarnij sie, czemu particle nie jest potomkiem Object wgl?
    ///bo nie chcemy go usuwac i zawsze ma byc ich tyle ile nr_particles pokazuje klaunie
    ///Particlesy
    for(auto &it: particles){
        if(it.life > 0.0f){
            glm::vec3 deltaVelocity = it.velocity * static_cast<float>(deltaTime);
            it.position += deltaVelocity;
            it.velocity.y = max(0.f,static_cast<float>(it.velocity.y - (it.gravity * deltaTime)));
            //glm::vec3 direction = glm::normalize(glm::vec3(0.f,3.f,0.f) - it.position);
            //glm::vec3 acceleration = (direction * glm::vec3(0.5f)) * static_cast<float>(deltaTime);
            //it.velocity += acceleration;
            it.life -= 1.f * deltaTime;
        }else{
            /*it.position = glm::vec3((rand() % 49) - 24.5f, (rand() % 49) - 24.5f, (rand() % 49) - 24.5f);
            it.velocity = glm::vec3((rand() % 100 - 50) / 100.0f, (rand() % 100 - 50) / 100.0f, (rand() % 100 - 50) / 100.0f);
            it.life = 15.f + rand()%50;
*/

            it.position = glm::vec3(0.f,3.f,0.f);
            it.velocity = glm::vec3((rand() % 100 - 50) / 100.0f, (rand() % 150) / 50.0f, (rand() % 100 - 50) / 100.0f);
            it.life = 10.f + rand()%50;

        }
    }

    //auto end = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double> duration = end - start;
    //std::cout << "Update zajal" << duration.count() << " seconds" << std::endl;
}


void Game::render(double deltaTime)
{
    //auto start = std::chrono::high_resolution_clock::now();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //tutaj update kamery
    view = kamera.GetViewMatrix();

    glUseProgram(shaderPrograms[0]);

    unsigned int viewLoc = glGetUniformLocation(shaderPrograms[0], "view");;
    unsigned int projectionLoc = glGetUniformLocation(shaderPrograms[0], "projection");

    //view jest dla perspektywy
    ///TO:DO ok wtf typie, wez to w jakas petle wsadz bo bedziesz to kopiowac dla kazdego nowego shadera
    glUniform3fv(glGetUniformLocation(shaderPrograms[0],"viewPos"),1, glm::value_ptr(kamera.Position));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderPrograms[1]);

    viewLoc = glGetUniformLocation(shaderPrograms[1], "view");
    projectionLoc = glGetUniformLocation(shaderPrograms[1], "projection");

    glUniform3fv(glGetUniformLocation(shaderPrograms[1],"viewPos"),1, glm::value_ptr(kamera.Position));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderPrograms[4]);

    viewLoc = glGetUniformLocation(shaderPrograms[4], "view");
    projectionLoc = glGetUniformLocation(shaderPrograms[4], "projection");

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

    ///Particle Render
    glUseProgram(shaderPrograms[4]);
    glUniform3fv(glGetUniformLocation(shaderPrograms[4], "cameraPos"),1, glm::value_ptr(kamera.Position));
    DrawParticles(shaderPrograms[4],particles);

    ///UI Render
    playerIntes[currentPlayersTurn]->renderGui(shaderPrograms[0],shaderPrograms);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    //auto end = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double> duration = end - start;
    //std::cout << "Render zajal" << duration.count() << " seconds" << std::endl;
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
