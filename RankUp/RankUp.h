#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>
#include <bakkesmod/plugin/bakkesmodsdk.h>
#include <bakkesmod/plugin/pluginwindow.h>
#include <bakkesmod/plugin/PluginSettingsWindow.h>
#include <fstream>

using namespace std;

class RankUp : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow {

    void onLoad();
    void onUnload();

public:
    void RenderSettings() override;
    void SetImGuiContext(uintptr_t ctx) override;
    std::string GetPluginName() override;
private:
    UniqueIDWrapper uniqueID;

    bool isEnabled;

    void CheckMMR(int retryCount);
    bool gotNewMMR;

    void EndGameHook();

    int userPlaylist, userDiv, userTier, upperTier, lowerTier, upperDiv, lowerDiv, nextLower, beforeUpper, yPos;
    float userMMR = 0;

    // Div numbers are stored in these
    std::string nameCurrent, nameNext, nameBefore;
    std::string nextDiff, beforeDiff;

    int unranker(int mode, int rank, int div, bool upperLimit);
    
    std::ofstream MMRGainListFile;

    int rankedPlaylists[8] = {	10, // Ones
                                11, // Twos
                                13, // Threes
                                27, // Hoops
                                28, // Rumble
                                29, // Dropshot
                                30, // Snowday
                                34 // Psynoix Tournaments
    };
};