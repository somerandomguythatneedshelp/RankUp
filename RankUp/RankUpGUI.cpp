#include <pch.h>
#include <RankUp.h>

void RankUp::Render()
{
    ImGui::Text("This plugin allows you to see all players' profile pictures on the scoreboard in-game.");
}

void RankUp::RenderWindow()
{
    
}

void RankUp::RenderSettings()
{
    ImGui::Text("Let me know if you have any suggestions.");
    ImGui::Text("Please note: there is some features you might be thinking of that i am currently developing\n In that case read the README.md in the github repo");
    ImGui::Separator();
    ImGui::Text("When this plugin states rank up, It WILL talk about divisions so if it says you will rank up it means divisions, may fix this in the future");
    ImGui::Separator();
    ImGui::Text("Discord: bobtypeshi");
}


void RankUp::SetImGuiContext(uintptr_t ctx)
{
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string RankUp::GetMenuName() {
    return "RankUp";
}

std::string RankUp::GetMenuTitle() {
    return menuTitle;
}

bool RankUp::ShouldBlockInput() {
    return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

bool RankUp::IsActiveOverlay() {
    return true;
}

void RankUp::OnOpen() {
    windowOpen = true;
}

void RankUp::OnClose() {
    windowOpen = false;
}

 std::string RankUp::GetPluginName() {
    return "RankUp";
 }