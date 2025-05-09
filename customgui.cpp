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

void GuiElement::renderOverlayTexture(GLuint textID, ImVec4 colorMult){
    ImU32 color = ImGui::GetColorU32(colorMult);
    ImVec2 p_min = ImGui::GetWindowPos();
    ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetWindowHeight());
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
    windowSpan = windowSize;
    windowHe = 0.013f * windowSpan.y;
    windowWi = 0.019f * windowSpan.y;
    createAndLoadTexture(marker_text,"resTextures/turnMarker.png");

    createAndLoadTexture(effectText[POISON] ,"resTextures/poison.png", false);
    createAndLoadTexture(effectText[BURNING] ,"resTextures/fire.png", false);
    createAndLoadTexture(effectText[DAMAGE] ,"resTextures/skull.png", false);
    createAndLoadTexture(effectText[SLOW] ,"resTextures/slow.png", false);
    createAndLoadTexture(effectText[HEAL] ,"resTextures/heal.png", false);
}

void UnitBar::update(double deltaTime)
{
    glm::vec3 aboveUnit = glm::vec3((*unitPos).x,stats->properHeight,(*unitPos).z) + glm::vec3(0.f,1.f,0.f);
    glm::vec2 screenPos = calculateScreenPosition(aboveUnit,*Object::viewRef, *Object::projectionRef,windowSpan.x,windowSpan.y);
    windowX = screenPos.x - windowWi*2;
    windowY = screenPos.y;
}

void UnitBar::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    if(windowX > 0 && windowY > 0)
    {
        float padding = 7.0f;

        ImGui::SetNextWindowSize(ImVec2(43, 10), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY - 13.f), ImGuiCond_Always);
        ImGui::Begin(("status"+ std::to_string(unitID)).c_str(), nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
        for(int i = 0; i < EFFECTS_COUNT; i++){
            if(stats->effects[i].duration > 0){
            ImGui::Image((void*)(intptr_t)effectText[i], ImVec2(10, 10));
            ImGui::SameLine();
            }
        }
        ImGui::End();

        int neededChars = std::to_string(stats->maxHealth).length();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::SetNextWindowSize(ImVec2(5.f + (neededChars*2*10.f), windowHe), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
        ImGui::Begin(("##test"+ std::to_string(unitID)).c_str(), nullptr, invisPreset | ImGuiWindowFlags_NoBackground);

        glm::vec3 ownerColor = factionColors[stats->ownerID-1];
        renderOverlayTexture(testOverlay,ImVec4(ownerColor.x,ownerColor.y,ownerColor.z,0.8f));

        ImGui::Text("%d/%d", stats->health, stats->maxHealth);
        ImGui::End();
        ImGui::PopStyleVar();

        if(stats->yourTurn){
            ImGui::SetNextWindowSize(ImVec2(25, 12), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(windowX, windowY - 15.f), ImGuiCond_Always);
            ImGui::Begin("turn indicator", nullptr, invisPreset | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::Image((void*)(intptr_t)marker_text, ImVec2(25, 12));
            ImGui::End();
        }
    }
}
