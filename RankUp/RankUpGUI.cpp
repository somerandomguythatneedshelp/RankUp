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