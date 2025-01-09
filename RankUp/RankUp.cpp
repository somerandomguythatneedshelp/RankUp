#include "pch.h"
#include "RankUp.h"

#include "json.hpp"
#include <fstream>

#include "logging.h"

BAKKESMOD_PLUGIN(RankUp, "Rank Up", "1.0.0", PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RankUp::onLoad()
{

    MMRGainListFile.open(gameWrapper->GetDataFolder() / "RankUp" / "Rm9sbG93IG15IHRpa3RvayB0YWguMWE=.txt"); // shouldent remove data
    
    _globalCvarManager = cvarManager;
    LOG("Plugin loaded!");

    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", std::bind(&RankUp::EndGameHook, this));

}

void RankUp::EndGameHook()
{
    if (gameWrapper->GetMMRWrapper().IsRanked(gameWrapper->GetMMRWrapper().GetCurrentPlaylist()))
    {
        LOG("Player is in a ranked game");
        CheckMMR(5);  
    } else
    {
        LOG("Player is not in a ranked game"); // if you really think about it, the brackets are the balls and this is the tip (i am so sorry i just had to mention it)
        CheckMMR(5);
    }
}


void RankUp::onUnload()
{
	LOG("Plugin UnLoaded");
    MMRGainListFile.close(); // game will crash and this might not be called, but who cares
}

// Converts information into mmr. Format is directly from game (mode is 11-30, rank is 0-22, div is 0-3). Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
int RankUp::unranker(int mode, int rank, int div, bool upperLimit) {

    std::string fileName = std::to_string(mode) + ".json";
    
    // Gets the correct json from the folder
    const auto rankJSON = gameWrapper->GetDataFolder() / "RankUp" / "tiers" / fileName;
    
    std::string limit;

    if (upperLimit == true) {
        limit = "maxMMR";
    }
    else {
        limit = "minMMR";
    }

    // Stores the json
    std::ifstream file(rankJSON);
    nlohmann::json j = nlohmann::json::parse(file);

    
    // Gets the correct mmr number
    return j["data"]["data"][((rank - 1) * 4) + (div + 1)][limit];
}

void RankUp::CheckMMR(int retryCount) 
{
	ServerWrapper sw = gameWrapper->GetOnlineGame();
    LOG("CHECK MMR IS CALLED");

    // if (sw.IsNull() || !sw.IsOnlineMultiplayer() || gameWrapper->IsInReplay())
    //     return;
    //
    // if (retryCount > 20 || retryCount < 0)
    //     return;

    LOG("CHECK MMR IS CALLED");

    //if (userPlaylist != 0) {
        LOG("CHECK MMR IS CALLED");
        gameWrapper->SetTimeout([retryCount, this](GameWrapper* gameWrapper) {
            gotNewMMR = false;
            while (!gotNewMMR) {
                LOG("Test 1");
                //if (gameWrapper->GetMMRWrapper().IsSynced(uniqueID, userPlaylist) && !gameWrapper->GetMMRWrapper().IsSyncing(uniqueID)) {

                    // Makes sure it is one of the ranked gamemodes to prevent crashes
                    // if (!(find(begin(rankedPlaylists), end(rankedPlaylists), userPlaylist) != end(rankedPlaylists))) {
                    //     LOG("Error line 98");
                    //     return;
                    //     
                    // }
                    // makes sure THIS SHIT ACTUALLY WORKS
                
                LOG("Test 2");

                    // Getting the mmr
                    userMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(uniqueID, userPlaylist);
                    gotNewMMR = true;

                    MMRWrapper mw = gameWrapper->GetMMRWrapper();

                    // The SkillRank has information about the players rank
                    SkillRank userRank = mw.GetPlayerRank(uniqueID, userPlaylist);

                    // Getting the player rank information into separate variables
                    userDiv = userRank.Division;
                    userTier = userRank.Tier;

                    int n;

                    // Converts the tier and div into the division with the roman numeral (I, II, III, IV)
                    // nameCurrent = GetDivName(userTier, userDiv);
                    
                     // Gets and loads the rank icon for your current rank from the RankViewer folder
                    // std::string fileName;
                    // fileName = std::to_string(userTier) + ".png";
                    // const auto currentPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    // currentRank = std::make_shared<ImageWrapper>(currentPath, false, true);

                    LOG("Test 2");

                    // This all checks for different scenarios where it won't be the default method of displaying
                    if (userTier <= 0) { // --- When still in placement matches -- 
                        // For placement shows from bronze 1 and supersonic legend
                        lowerTier = 1;
                        upperTier = 22;

                        nextLower = unranker(userPlaylist, upperTier, 0, true); // div has to be I (0) since ssl doesn't have divisions
                        //nameNext = GetDivName(22, 0);
                        n = nextLower - userMMR;

                       if (n < 0) { continue; }

                       

                       MMRGainListFile << std::to_string(n) << std::endl;

                        LOG("Please dont be null: ------------------------------------------------------------ int n = " + std::to_string(n));

                    MMRGainListFile.close(); // already closed but fuck off

                        beforeUpper = unranker(userPlaylist, lowerTier, 0, false);
                        //nameBefore = GetDivName(0, 0); // This inputs the unranked name since it just won't show the division number
                    }

                    // srgtnh: code above doesnt need to be modified 
                    
                    else if (userTier == 1 && userDiv == 0) {
                        // For bronze 1 div 1. It just shows the bronze 1 div 1 lower limit on the bottom and bronze 1 div 2 on top
                        upperTier = userTier;
                        lowerTier = userTier;
                        upperDiv = userDiv + 1;
                        lowerDiv = 0;

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
                        //nameNext = GetDivName(upperTier, upperDiv);

                        n = nextLower - userMMR;

                        if (n < 0) { continue; }

                        MMRGainListFile << std::to_string(n) << std::endl;

                        LOG("Please dont be null: ------------------------------------------------------------ int n = " + std::to_string(n));

                    MMRGainListFile.close(); // already closed but fuck off
                        
                       // beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, false);
                        //nameBefore = GetDivName(lowerTier, lowerDiv);
                    }
                    else if (userTier == 22) {
                        // For ssl. Shows the ssl upper limit on top and gc 3 div 4 on bottom
                        upperTier = userTier;
                        lowerTier = userTier - 1;
                        upperDiv = userDiv;
                        lowerDiv = 3;

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, true);
                        //nameNext = GetDivName(upperTier, upperDiv);

                        n = nextLower - userMMR;

                       if (n < 0) { continue; }

                        MMRGainListFile << std::to_string(n) << std::endl;

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                        //nameBefore = GetDivName(lowerTier, lowerDiv);

                        LOG("Please dont be null: ------------------------------------------------------------ int n = " + std::to_string(n));

                    MMRGainListFile.close(); // already closed but fuck off
                    }
                    else {
                        // Finds out what div is above and below you
                        if (userDiv == 0) {
                            upperTier = userTier;
                            lowerTier = userTier - 1;
                            upperDiv = userDiv + 1;
                            lowerDiv = 3;

                        }
                        else if (userDiv == 3) {
                            upperTier = userTier + 1;
                            lowerTier = userTier;
                            upperDiv = 0;
                            lowerDiv = userDiv - 1;
                        }
                        else {
                            upperTier = userTier;
                            lowerTier = userTier;
                            upperDiv = userDiv + 1;
                            lowerDiv = userDiv - 1;
                        }

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
                        //nameNext = GetDivName(upperTier, upperDiv);

                        n = nextLower - userMMR;

                       if (n < 0) { continue; }

                       MMRGainListFile << std::to_string(n) << std::endl;

                        LOG("Please dont be null: ------------------------------------------------------------ int n = " + std::to_string(n));

                    MMRGainListFile.close(); // already closed but fuck off

                        //beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                        //nameBefore = GetDivName(lowerTier, lowerDiv);
                    }

                    // // Gets correct rank icons from folder for before and after ranks
                    // fileName = std::to_string(lowerTier) + ".png";
                    // const auto beforePath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    // beforeRank = std::make_shared<ImageWrapper>(beforePath, false, true);
                    //
                    // fileName = std::to_string(upperTier) + ".png";
                    // const auto nextPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    // nextRank = std::make_shared<ImageWrapper>(nextPath, false, true);
                    //
                    // // Lets rank viewer display
                    // drawCanvas = true;
                //}
                // Failsafe
                if (!gotNewMMR && retryCount > 0) {
                    gameWrapper->SetTimeout([retryCount, this](GameWrapper* gameWrapper) {
                        this->CheckMMR(retryCount - 1);
                        }, 0.5f);
                }
                else {
                    return;
                }
            }
        }, 3);
    //}
}


// Gui Code

void RankUp::RenderSettings()
{
	ImGui::Text("Hello, world!");
}

void RankUp::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string RankUp::GetPluginName()
{
	return "RankUp";
}
