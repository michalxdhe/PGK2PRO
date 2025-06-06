#include "customgui.h"
#include "units.h"

ImGuiWindowFlags invisPreset = ImGuiWindowFlags_NoTitleBar |
                               ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoScrollbar;

UnitGui::UnitGui() = default;

const std::string abilityNames[ABILITIES_COUNT] = {
    "Attack",
    "Create",
    "Morph",
    "Spit",
    "Hex",
    "Decimate",
    "Cleanse",
    "Rally",
    "Fortify",
    "SpeedBoon",
    "HealthBoon"
};

const glm::vec3 colors[EFFECTS_COUNT] = {
    glm::vec3(1.f,0.f,0.f),
    glm::vec3(0.f,1.f,0.f),
    glm::vec3(1.f,0.5f,0.f),
    glm::vec3(1.f,1.f,0.f),
    glm::vec3(0.f,1.f,0.f)
};

SDL_Cursor* cursorFromGLTexture(GLuint tex, int hotX, int hotY) {
    glBindTexture(GL_TEXTURE_2D, tex);

    GLint width = 0, height = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    if (width <= 0 || height <= 0) {
        std::fprintf(stderr, "CursorFromGLTexture: invalid texture size %dx%d\n", width, height);
        return nullptr;
    }

    std::vector<unsigned char> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        pixels.data(),
        width, height,
        32,
        width * 4,
        SDL_PIXELFORMAT_RGBA32
    );
    if (!surf) {
        std::fprintf(stderr, "CursorFromGLTexture: SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        return nullptr;
    }

    SDL_Cursor* cursor = SDL_CreateColorCursor(surf, hotX, hotY);
    if (!cursor) {
        std::fprintf(stderr, "CursorFromGLTexture: SDL_CreateColorCursor failed: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surf);

    return cursor;
}

//https://github.com/ocornut/imgui/pull/5386
void ImageTurner(ImTextureID tex_id, ImVec2 center, ImVec2 size, float* angle_, float round_sec, ImU32 color, ImDrawList* draw_list)
{
    auto& angle = *angle_;
    auto& io = ImGui::GetIO();
    float sin_a = sinf(angle);
    float cos_a = cosf(angle);
    ImVec2 pos[4] = {
        center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
    };

    if (!draw_list)
        draw_list = ImGui::GetBackgroundDrawList();
    draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], ImVec2(0,0), ImVec2(1,0), ImVec2(1,1), ImVec2(0,1), color);

    if (round_sec) {
        angle += io.DeltaTime * round_sec * 6.2831f;
        if (angle < -628.0f)
            angle += 628.0f;
        else if (angle > 628.0f)
            angle -= 628.0f;
    } else if (angle) {
        angle = 0.0f;
    }
}

void GuiElement::renderOverlayTexture(GLuint textID, ImVec4 colorMult, float* angle, float round_sec)
{
    ImU32 color = ImGui::GetColorU32(colorMult);
    ImVec2 p_min = ImGui::GetWindowPos();
    ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetWindowHeight());
    ImVec2 center = ImVec2(0.5f * (p_min.x + p_max.x), 0.5f * (p_min.y + p_max.y));
    ImVec2 size = ImVec2(p_max.x - p_min.x, p_max.y - p_min.y);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if(angle)
        ImageTurner((ImTextureID)(intptr_t)textID, center, size, angle, round_sec, color, draw_list);
    else
        ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)textID, p_min, p_max, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color);
}


PauseMenu::PauseMenu() = default;

PauseMenu::PauseMenu(ImVec2 windowSize, int *sRef)
{
    soundRef = sRef;
    this->screenSize = windowSize;
    windowSpan = windowSize;
}

void PauseMenu::update(double deltaTime)
{
}

void PauseMenu::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    ImGui::SetNextWindowSize(ImVec2(screenSize.x, screenSize.y), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0.f,0.f), ImGuiCond_Always);
    ImGui::Begin("PauseChamp", nullptr, invisPreset | ImGuiWindowFlags_NoBackground );
    ImGui::SetWindowFocus();
    renderOverlayTexture(testOverlay,ImVec4(1.0f,1.0f,1.0f,0.8f));

    ImGui::SetCursorPosY(80.0f);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(2.0f);

    const char* pauseText = "Pause";
    float textWidth = ImGui::CalcTextSize(pauseText).x;
    ImGui::SetCursorPosX((screenSize.x - textWidth) * 0.5f);

    ImGui::Text("%s", pauseText);

    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    float sliderWidth = screenSize.x * 0.5f;
    ImGui::SetCursorPosX((screenSize.x - sliderWidth) * 0.5f);
    ImGui::SetCursorPosY(50.0f);

    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderInt("Volume", soundRef, 0, 100);


    ImGui::End();
}

TextParticle::TextParticle() = default;

TextParticle::TextParticle(ImVec2 windowSize, int damage, EFFECTS type, glm::vec3 worldPos, int objID)
{
    this->ID = objID;
    this->worldPos = worldPos;
    lifeTimer = 25 + damage;
    windowSpan = windowSize;
    message = std::to_string(damage);

    color = ImVec4(colors[type].x,colors[type].y,colors[type].z,1.f);
}

void TextParticle::update(double deltaTime)
{
    worldPos += glm::vec3(0.f,1.f* deltaTime,0.f) ;
    if(lifeTimer > 0)
        lifeTimer -= 50*deltaTime;
}

void TextParticle::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    glm::vec2 screenPos = calculateScreenPosition(worldPos,*Object::viewRef, *Object::projectionRef,windowSpan.x,windowSpan.y);
    ImGui::SetNextWindowSize(ImVec2(10.f, 25.f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((int)screenPos.x, (int)screenPos.y - 16), ImGuiCond_Always);
    ImGui::Begin(("##dmgprt"+ std::to_string(this->ID)).c_str(), nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::Text("%s", message.c_str());
        ImGui::PopStyleColor();
    ImGui::End();
}

UnitGui::UnitGui(ImVec2 windowSize, UnitStats *stats,  std::array<int, ABILITIES_COUNT> *abilityList,int* selectedAbil, array<UnitType, 10> *buildMenu, array<UnitType, 10> *morphMenu)
    : stats(stats), abilityList(abilityList), selectedAbil(selectedAbil), buildMenu(buildMenu), morphMenu(morphMenu)
{
    windowSpan = windowSize;
    windowHe = 0.058f * windowSpan.y;
    windowWi = 0.6f * windowSpan.x;
    windowX = (windowSpan.x - windowWi) / 2;
    windowY = 0.93f * windowSpan.y;

    createAndLoadTexture(activeTokenTexture ,"resTextures/activeTurnMarker.png", false);
    createAndLoadTexture(inactiveTokenTexture ,"resTextures/NOTactiveTurnMarker.png", false);

    createAndLoadTexture(godForgiveMe[0] ,"resTextures/att.png", false);
    createAndLoadTexture(godForgiveMe[1] ,"resTextures/armor.png", false);
    createAndLoadTexture(godForgiveMe[2] ,"resTextures/heart.png", false);
    createAndLoadTexture(godForgiveMe[3] ,"resTextures/spd.png", false);
    createAndLoadTexture(godForgiveMe[4] ,"resTextures/movrange.png", false);

    createAndLoadTexture(effectText[POISON] ,"resTextures/poison.png", false);
    createAndLoadTexture(effectText[BURNING] ,"resTextures/fire.png", false);
    createAndLoadTexture(effectText[DAMAGE] ,"resTextures/skull.png", false);
    createAndLoadTexture(effectText[SLOW] ,"resTextures/slow.png", false);
    createAndLoadTexture(effectText[HEAL] ,"resTextures/heal.png", false);

    statswindowHe = 0.115f * windowSpan.y;
    statswindowWi = 0.2f * windowSpan.x;
    statswindowX = (windowSpan.x - statswindowWi) / 100;
    statswindowY = 0.80f * windowSpan.y;
}

void UnitGui::update(double deltaTime)
{

}

void UnitGui::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    float padding = 10.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));

    if(stats->maxActionTokens > 0){
        ImGui::SetNextWindowSize(ImVec2(stats->maxActionTokens * 35.f, 45.f), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0.5f * windowSpan.x - (stats->maxActionTokens * 35.f)/2, windowY - 60.f), ImGuiCond_Always);
        ImGui::Begin("actionTokens", nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
        for(int i = 0; i < stats->maxActionTokens; i++){
            if(stats->actionTokens > i)
                ImGui::Image((void*)(intptr_t)activeTokenTexture, ImVec2(25, 25));
            else
                ImGui::Image((void*)(intptr_t)inactiveTokenTexture, ImVec2(25, 25));

            ImGui::SameLine();
        }
        ImGui::End();
    }

    ImVec2 numberImageSpacing(2,0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, numberImageSpacing);
    ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
    ImGui::Begin("hotBar", nullptr, invisPreset | ImGuiWindowFlags_NoBackground);

    renderOverlayTexture(testOverlay,ImVec4(1.0f,1.0f,1.0f,0.7f));

    bool rainingToday = false;
    for(const auto& it : *abilityList)
    {
        if(it == *selectedAbil){
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.7f, 0.0f, 1.0f));
            rainingToday = true;
        }
        if(it < ABILITIES_COUNT){
            if(ImGui::Button(abilityNames[it].c_str(),ImVec2(25.f,25.f)))
                *selectedAbil = it;
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
            ImGui::SetTooltip(abilityDesc[it].c_str());
            }
        }
        if(rainingToday){
            ImGui::PopStyleColor(1);
            rainingToday = false;
        }

        ImGui::SameLine();
    }


    if(*selectedAbil == CREATE){
        int countOfButtons = 0;
        for(auto itt : *buildMenu){
            if(itt != UNIT_TYPE_COUNT){
                countOfButtons += 1;
            }
        }

        ImGui::SetNextWindowSize(ImVec2((countOfButtons * 27.f) + padding*2, windowHe), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY - 45.f), ImGuiCond_Always);
        ImGui::Begin("BuildSelect", nullptr, invisPreset);

        for(auto itt : *buildMenu){
            if(itt != UNIT_TYPE_COUNT){
                if(itt == stats->selectedToBuild){
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.7f, 0.0f, 1.0f));
                    rainingToday = true;
                }
                if(ImGui::Button(std::to_string(itt).c_str(),ImVec2(25.f,25.f))){
                    stats->selectedToBuild = itt;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    ImGui::SetTooltip(unitDesc[itt].c_str());
                }
                if(rainingToday){
                    ImGui::PopStyleColor(1);
                    rainingToday = false;
                }
                ImGui::SameLine();
            }
        }
        ImGui::End();
    }

    if(*selectedAbil == MORPH){
        int countOfButtons = 0;
        for(auto itt : *morphMenu){
            if(itt != UNIT_TYPE_COUNT){
                countOfButtons += 1;
            }
        }

        ImGui::SetNextWindowSize(ImVec2((countOfButtons * 27.f) + padding*2, windowHe), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY - 45.f), ImGuiCond_Always);
        ImGui::Begin("BuildSelect", nullptr, invisPreset);

        for(auto itt : *morphMenu){
            if(itt != UNIT_TYPE_COUNT){
                if(itt == stats->selectedToBuild){
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.7f, 0.0f, 1.0f));
                    rainingToday = true;
                }
                if(ImGui::Button(std::to_string(itt).c_str(),ImVec2(25.f,25.f))){
                    stats->selectedToBuild = itt;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    ImGui::SetTooltip(unitDesc[itt].c_str());
                }
                if(rainingToday){
                    ImGui::PopStyleColor(1);
                    rainingToday = false;
                }
                ImGui::SameLine();
            }
        }
        ImGui::End();
    }

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::SetNextWindowSize(ImVec2(statswindowWi/2, statswindowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(statswindowX, statswindowY), ImGuiCond_Always);
    ImGui::Begin("unitInfo", nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    renderOverlayTexture(testOverlay,ImVec4(1.0f,1.0f,1.0f,0.7f));

    //actionTokens, maxActionTokens;
    ImGui::Text("%d/%d ", stats->health, stats->maxHealth);
    ImGui::SameLine();
    ImGui::Image((void*)(intptr_t)godForgiveMe[2], ImVec2(10, 10));
    ImGui::Text("%d/%d ", stats->movRange, stats->maxMovRange);
    ImGui::SameLine();
    ImGui::Image((void*)(intptr_t)godForgiveMe[4], ImVec2(10, 10));
    ImGui::Text("%d ", stats->speed - stats->effects[SLOW].intensity);
    ImGui::SameLine();
    ImGui::Image((void*)(intptr_t)godForgiveMe[3], ImVec2(10, 10));
    ImGui::Text("%d ", stats->att);
    ImGui::SameLine();
    ImGui::Image((void*)(intptr_t)godForgiveMe[0], ImVec2(10, 10));
    ImGui::Text("%d ", stats->def);
    ImGui::SameLine();
    ImGui::Image((void*)(intptr_t)godForgiveMe[1], ImVec2(10, 10));
    ImGui::End();
    ImGui::SetNextWindowSize(ImVec2(statswindowWi/2, statswindowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(statswindowX+statswindowWi/2, statswindowY), ImGuiCond_Always);
    ImGui::Begin("unitStatus", nullptr, invisPreset);
    for(int i = 0; i < EFFECTS_COUNT; i++){
        ImGui::Image((void*)(intptr_t)effectText[i], ImVec2(10, 10));
        ImGui::SameLine();
        ImGui::Text(" %d x %d", stats->effects[i].duration, stats->effects[i].intensity);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

PlayerGui::PlayerGui() = default;

PlayerGui::PlayerGui(ImVec2 windowSize, std::array<int, RESOURCE_COUNT> *resourcesRef): resources(resourcesRef)
{
    windowSpan = windowSize;
    windowHe = 0.058 * windowSpan.y;
    windowWi = 0.2f * windowSpan.x;
    windowX = (windowSpan.x - windowWi) / 100;
    windowY = 0.02f * windowSpan.y;

    for(int i = 0; i < RESOURCE_COUNT; i++)
    {
        std::string filePath = "resTextures/res" + std::to_string(i + 1) + ".png";
        createAndLoadTexture(resTextures[i],filePath.c_str(), false);
    }
}

void PlayerGui::update(double deltaTime)
{

}

void PlayerGui::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    ImVec2 generalSpacing((windowWi / 4.9f), 2.f);
    ImVec2 numberImageSpacing(2,2);
    float padding = 10.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, numberImageSpacing);

    ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);

    ImGui::Begin("resources", nullptr, invisPreset | ImGuiWindowFlags_NoBackground);

    renderOverlayTexture(testOverlay,ImVec4(1.0f,1.0f,1.0f,0.7f));

    for (int i = 0; i < RESOURCE_COUNT; ++i)
    {
        ImGui::Text("%d", (*resources)[i]);
        if(i!=0)
            ImGui::PopStyleVar();
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, generalSpacing);
        ImGui::Image((void*)(intptr_t)resTextures[i], ImVec2(10, 10));
        if(i==RESOURCE_COUNT-1)
            ImGui::PopStyleVar();
        if (i % 4 != 3)
        {
            ImGui::SameLine();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

InitiativeTrackerGui::InitiativeTrackerGui() = default;

InitiativeTrackerGui::InitiativeTrackerGui(ImVec2 windowSize, std::deque<Unit> *qu, int64_t *highlight, bool *hovered)
: queueRef(qu), highlightRef(highlight), hoveredRef(hovered)
{
    windowSpan = windowSize;
    windowHe = 0.055f * windowSpan.y;
    windowWi = 0.4f * windowSpan.x;
    windowX = (windowSpan.x - windowWi) / 2;
    windowY = 0.001f * windowSpan.y;
}

void InitiativeTrackerGui::update(double deltaTime)
{
}

void InitiativeTrackerGui::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    ImVec2 numberImageSpacing(5,2);
    float padding = 10.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, numberImageSpacing);

    ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
    ImGui::Begin("initiativeTracker", nullptr, invisPreset | ImGuiWindowFlags_NoBackground);

    Unit& nextUnit = queueRef->front();
    glm::vec3 currentPlayerColor = factionColors[nextUnit.owner-1];
    renderOverlayTexture(testOverlay,ImVec4(currentPlayerColor.x,currentPlayerColor.y,currentPlayerColor.z,0.7f));

    for(auto& it : *queueRef)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(factionColors[it.owner-1].x,factionColors[it.owner-1].y,factionColors[it.owner-1].z,1.f));
        ImGui::Text("%d", it.stats.speed - it.stats.effects[SLOW].intensity);
        if(ImGui::IsItemHovered()){
            *highlightRef = it.ID;
            *hoveredRef = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

UnitBar::UnitBar() = default;

UnitBar::UnitBar(ImVec2 windowSize, UnitStats *stats, glm::vec3 *unitPos, int64_t ID)
    : stats(stats), unitID(ID), unitPos(unitPos)
{
    markerMoveDir = true;
    markerHeightMult = 0.f;
    windowSpan = windowSize;
    windowHe = 0.099f * windowSpan.y;
    windowWi = 0.099f * windowSpan.y;
    createAndLoadTexture(marker_text,"resTextures/turnMarker.png");

    borderAngle = 0.f;

    createAndLoadTexture(effectText[POISON] ,"resTextures/poison.png", false);
    createAndLoadTexture(effectText[BURNING] ,"resTextures/fire.png", false);
    createAndLoadTexture(effectText[DAMAGE] ,"resTextures/skull.png", false);
    createAndLoadTexture(effectText[SLOW] ,"resTextures/slow.png", false);
    createAndLoadTexture(effectText[HEAL] ,"resTextures/heal.png", false);
}

void UnitBar::update(double deltaTime)
{
    if(stats->yourTurn){
    if(markerHeightMult > 1.0f)
        markerMoveDir = false;
    if(markerHeightMult < 0.1f)
        markerMoveDir = true;

    if(markerMoveDir)
        markerHeightMult += deltaTime;
    else
        markerHeightMult -= deltaTime;
    }

    glm::vec3 aboveUnit = glm::vec3((*unitPos).x,stats->properHeight,(*unitPos).z) + glm::vec3(0.f,1.f,0.f);
    glm::vec2 screenPos = calculateScreenPosition(aboveUnit,*Object::viewRef, *Object::projectionRef,windowSpan.x,windowSpan.y);
    windowX = screenPos.x;
    windowY = screenPos.y;
}

void UnitBar::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    if(windowX > 0 && windowY > 0)
    {
        float padding = 7.0f;

        ImGui::SetNextWindowSize(ImVec2(43, 10), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY - 13.f), ImGuiCond_Always,ImVec2(0.5f, 0.5f));

        ImGui::Begin(("status"+ std::to_string(unitID)).c_str(), nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
        for(int i = 0; i < EFFECTS_COUNT; i++){
            if(stats->effects[i].duration > 0){
            ImGui::Image((void*)(intptr_t)effectText[i], ImVec2(10, 10));
            ImGui::SameLine();
            }
        }
        ImGui::End();


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::SetNextWindowSize(ImVec2(windowHe, windowWi), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always,ImVec2(0.5f, 0.5f));

        float myDistToCam = glm::distance(glm::vec3(*unitPos), glm::vec3(Globals::cameraX, Globals::cameraY, Globals::cameraZ));

        ImGui::Begin(("##test"+ std::to_string(unitID)).c_str(), nullptr, invisPreset | ImGuiWindowFlags_NoBackground);

        if(myDistToCam < Globals::closestUnitToCam || Globals::closestUnitToCam == -1.f){
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            Globals::closestUnitToCam = myDistToCam;
        }

        glm::vec3 ownerColor = factionColors[stats->ownerID-1];

        GLuint targetText;
        if(stats->isCommander)
            targetText = unitBarOverlayCommVer;
        else
            targetText = unitBarOverlay;
        if(stats->yourTurn)
            renderOverlayTexture(unitBarOverlayCommVer,ImVec4(ownerColor.x,ownerColor.y,ownerColor.z,0.8f),&borderAngle,0.1f);
        else
            renderOverlayTexture(unitBarOverlayCommVer,ImVec4(ownerColor.x,ownerColor.y,ownerColor.z,0.8f),&borderAngle,0.000001f);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos  = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();

        ImVec2 center = ImVec2(
            window_pos.x + window_size.x * 0.5f,
            window_pos.y + window_size.y * 0.5f
        );

        float radius = (std::min(window_size.x, window_size.y) * 0.5f) - 18.0f;

        int totalSlices   = stats->maxHealth;
        int filledSlices  = stats->health;

        if (totalSlices == 1) {
            radius -= 10;
            ImVec2 top    = ImVec2(center.x, center.y - radius);
            ImVec2 right  = ImVec2(center.x + radius, center.y);
            ImVec2 bottom = ImVec2(center.x, center.y + radius);
            ImVec2 left   = ImVec2(center.x - radius, center.y);

            ImU32 fillColor = (filledSlices > 0) ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 0, 0, 255);

            ImVec2 rhombus_pts[4] = { top, right, bottom, left };
            draw_list->AddConvexPolyFilled(rhombus_pts, 4, fillColor);

            draw_list->AddPolyline(rhombus_pts, 4, IM_COL32(75, 25, 25, 255), true, 1.0f);
        }
        else if (totalSlices == 2) {
            radius -= 5;
            ImVec2 top    = ImVec2(center.x, center.y - radius);
            ImVec2 right  = ImVec2(center.x + radius, center.y);
            ImVec2 bottom = ImVec2(center.x, center.y + radius);
            ImVec2 left   = ImVec2(center.x - radius, center.y);

            ImU32 leftColor = (filledSlices > 0) ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 0, 0, 255);
            ImVec2 left_triangle[3] = { center, top, left };
            draw_list->AddTriangleFilled(left_triangle[0], left_triangle[1], left_triangle[2], leftColor);
            ImVec2 left_triangle2[3] = { center, left, bottom };
            draw_list->AddTriangleFilled(left_triangle2[0], left_triangle2[1], left_triangle2[2], leftColor);

            ImU32 rightColor = (filledSlices > 1) ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 0, 0, 255);
            ImVec2 right_triangle[3] = { center, top, right };
            draw_list->AddTriangleFilled(right_triangle[0], right_triangle[1], right_triangle[2], rightColor);
            ImVec2 right_triangle2[3] = { center, right, bottom };
            draw_list->AddTriangleFilled(right_triangle2[0], right_triangle2[1], right_triangle2[2], rightColor);

            draw_list->AddLine(top, bottom, IM_COL32(75, 25, 25, 255), 1.0f);
            ImVec2 rhombus_pts[4] = { top, right, bottom, left };
            draw_list->AddPolyline(rhombus_pts, 4, IM_COL32(75, 25, 25, 255), true, 1.0f);
        }
        else {
            for (int i = 0; i < totalSlices; i++)
            {
                float t0 = float(i) / float(totalSlices);
                float t1 = float(i + 1) / float(totalSlices);

                float a0 = 2.0f * 3.14159265f * t0 - 3.14159265f * 0.5f;
                float a1 = 2.0f * 3.14159265f * t1 - 3.14159265f * 0.5f;
                ImVec2 p0 = ImVec2(center.x + cosf(a0) * radius, center.y + sinf(a0) * radius);
                ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);

                bool filled = (i < filledSlices);
                ImU32 sliceCol = filled ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 0, 0, 255);
                draw_list->AddTriangleFilled(center, p0, p1, sliceCol);

                ImVec2 pts[3] = { center, p0, p1 };
                draw_list->AddPolyline(pts, 3, IM_COL32(75, 25, 25, 255), true, 1.0f);
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();

        if(stats->yourTurn){
            ImGui::SetNextWindowSize(ImVec2(50, 24), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(windowX + 12.f, windowY - (13.f + (glm::smoothstep(0.1f, 1.0f, markerHeightMult)*25))), ImGuiCond_Always,ImVec2(0.5f, 0.5f));
            ImGui::Begin("turn indicator", nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            ImGui::Image((void*)(intptr_t)marker_text, ImVec2(25, 12));
            ImGui::End();
        }
    }
}
