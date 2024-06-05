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
    "Missile"
};

const glm::vec3 colors[EFFECTS_COUNT] = {
    glm::vec3(1.f,0.f,0.f),
    glm::vec3(0.f,1.f,0.f),
    glm::vec3(1.f,0.5f,0.f),
    glm::vec3(1.f,1.f,0.f)
};

TextParticle::TextParticle() = default;

TextParticle::TextParticle(ImVec2 windowSize, int damage, EFFECTS type, glm::vec3 worldPos, int objID)
{
    this->ID = objID;
    this->worldPos = worldPos;
    lifeTimer = 25 + damage/10;
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

UnitGui::UnitGui(ImVec2 windowSize, UnitStats *stats,  std::array<int, ABILITIES_COUNT> *abilityList,int* selectedAbil)
    : stats(stats), abilityList(abilityList), selectedAbil(selectedAbil)
{
    windowSpan = windowSize;
    windowHe = 45;
    windowWi = 0.7f * windowSpan.x;
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

    statswindowHe = 90;
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
    ImGui::Begin("hotBar", nullptr, invisPreset);
    for(const auto& it : *abilityList)
    {
        if(it >= 0)
            if(ImGui::Button(abilityNames[it].c_str(),ImVec2(25.f,25.f))){
                *selectedAbil = it;

            }
        ImGui::SameLine();
    }
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::SetNextWindowSize(ImVec2(statswindowWi/2, statswindowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(statswindowX, statswindowY), ImGuiCond_Always);
    ImGui::Begin("unitInfo", nullptr, invisPreset);
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
    windowHe = 45;
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

    ImGui::Begin("resources", nullptr, invisPreset);
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
    windowHe = 45;
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
    ImGui::Begin("initiativeTracker", nullptr, invisPreset);

    for(auto& it : *queueRef)
    {
        ImGui::Text("%d", it.stats.speed - it.stats.effects[SLOW].intensity);
        if(ImGui::IsItemHovered()){
            *highlightRef = it.ID;
            *hoveredRef = true;
        }

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
    windowHe = 10.f;
    windowWi = 10.f;
    createAndLoadTexture(marker_text,"resTextures/turnMarker.png");

    createAndLoadTexture(effectText[POISON] ,"resTextures/poison.png", false);
    createAndLoadTexture(effectText[BURNING] ,"resTextures/fire.png", false);
    createAndLoadTexture(effectText[DAMAGE] ,"resTextures/skull.png", false);
    createAndLoadTexture(effectText[SLOW] ,"resTextures/slow.png", false);
}

void UnitBar::update(double deltaTime)
{
    glm::vec3 aboveUnit = *unitPos + glm::vec3(0.f,1.f,0.f);
    glm::vec2 screenPos = calculateScreenPosition(aboveUnit,*Object::viewRef, *Object::projectionRef,windowSpan.x,windowSpan.y);
    windowX = screenPos.x - windowWi*2;
    windowY = screenPos.y;
}

void UnitBar::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms)
{
    if(windowX > 0 && windowY > 0)
    {
        float padding = 10.0f;

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

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
        ImGui::Begin(("##test"+ std::to_string(unitID)).c_str(), nullptr, invisPreset);
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
