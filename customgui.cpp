#include "customgui.h"
#include "units.h"

ImGuiWindowFlags invisPreset = ImGuiWindowFlags_NoTitleBar |
                               ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoScrollbar;

UnitGui::UnitGui() = default;

UnitGui::UnitGui(ImVec2 windowSize, int *health, int *speed, int *attack, int *defense, int *movementRange)
    : hp(health), spd(speed), att(attack), def(defense), movRange(movementRange) {
    windowSpan = windowSize;
    windowHe = 45;
    windowWi = 0.7f * windowSpan.x;
    windowX = (windowSpan.x - windowWi) / 2;
    windowY = 0.93f * windowSpan.y;

    statswindowHe = 90;
    statswindowWi = 0.2f * windowSpan.x;
    statswindowX = (windowSpan.x - statswindowWi) / 100;
    statswindowY = 0.80f * windowSpan.y;
}

void UnitGui::update(double deltaTime) {

}

void UnitGui::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms) {
    ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
    ImGui::Begin("hotBar", nullptr, invisPreset);

    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(statswindowWi, statswindowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(statswindowX, statswindowY), ImGuiCond_Always);
    ImGui::Begin("unitInfo", nullptr, invisPreset);

    ImGui::End();
}

PlayerGui::PlayerGui() = default;

PlayerGui::PlayerGui(ImVec2 windowSize, int *r1, int *r2, int *r3, int *r4, int *r5, int *r6, int *r7, int *r8) {
    windowSpan = windowSize;
    windowHe = 45;
    windowWi = 0.2f * windowSpan.x;
    windowX = (windowSpan.x - windowWi) / 100;
    windowY = 0.02f * windowSpan.y;

    resources[ORE] = r1;
    resources[GEMS] = r2;
    resources[GAS] = r3;
    resources[OIL] = r4;
    resources[WATER] = r5;
    resources[FIRE] = r6;
    resources[EARTH] = r7;
    resources[AIR] = r8;

    for(int i = 0; i < RESOURCE_COUNT; i++){
        std::string filePath = "resTextures/res" + std::to_string(i + 1) + ".png";
        createAndLoadTexture(resTextures[i],filePath.c_str());
    }
}

void PlayerGui::update(double deltaTime) {

}

void PlayerGui::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms) {
    ImVec2 generalSpacing((windowWi / 4.9f), 2.f);
    ImVec2 numberImageSpacing(2,2);
    float padding = 10.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, numberImageSpacing);

    ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);

    ImGui::Begin("resources", nullptr, invisPreset);
    for (int i = 0; i < RESOURCE_COUNT; ++i) {
        ImGui::Text("%d", *resources[i]);
        if(i!=0)
            ImGui::PopStyleVar();
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, generalSpacing);
        ImGui::Image((void*)(intptr_t)resTextures[i], ImVec2(10, 10));
        if(i==RESOURCE_COUNT-1)
          ImGui::PopStyleVar();
        if (i % 4 != 3) {
            ImGui::SameLine();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

InitiativeTrackerGui::InitiativeTrackerGui() = default;

InitiativeTrackerGui::InitiativeTrackerGui(ImVec2 windowSize, std::deque<Unit> *qu): queueRef(qu) {
    windowSpan = windowSize;
    windowHe = 45;
    windowWi = 0.4f * windowSpan.x;
    windowX = (windowSpan.x - windowWi) / 2;
    windowY = 0.001f * windowSpan.y;
}

void InitiativeTrackerGui::update(double deltaTime) {
}

void InitiativeTrackerGui::render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms) {
    ImVec2 numberImageSpacing(5,2);
    float padding = 10.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, numberImageSpacing);

    ImGui::SetNextWindowSize(ImVec2(windowWi, windowHe), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
    ImGui::Begin("initiativeTracker", nullptr, invisPreset);

    for(auto& it : *queueRef){
        ImGui::Text("%d", it.speed);
        ImGui::SameLine();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}
