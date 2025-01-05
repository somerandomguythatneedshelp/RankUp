#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>
#include <bakkesmod/plugin/bakkesmodsdk.h>
#include <bakkesmod/plugin/pluginwindow.h>
#include <bakkesmod/plugin/PluginSettingsWindow.h>

class RankUp : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow {

    void onLoad();
    void onUnload();

public:
    void RenderSettings() override;
    void SetImGuiContext(uintptr_t ctx) override;
    std::string GetPluginName() override;
private:
    UniqueIDWrapper uniqueWrapper;

    bool isEnabled;

    void CheckMMR(int retryCount);
    bool gotNewMMR;

    int userPlaylist, userDiv, userTier, upperTier, lowerTier, upperDiv, lowerDiv, nextLower, beforeUpper, yPos;
    float userMMR = 0;

    // Div numbers are stored in these
    std::string nameCurrent, nameNext, nameBefore;
    std::string nextDiff, beforeDiff;
};