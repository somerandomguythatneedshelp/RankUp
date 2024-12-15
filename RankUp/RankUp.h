#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include <fstream>

#include "version.h"
#include <set>
#include <map>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <ctime>
#include <chrono>
#include <json.hpp>
#include <future>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include "Windows.h"
#include <sys/stat.h>


#include "GuiBase.h"
#include "./Settings.h"
#include "./ScoreboardPositionInfo.h"


#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class RankUp: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	,public PluginWindowBase // Uncomment if you want to render your own plugin window
{
    
public:
	 struct Pri {
		std::string uid;
        std::string id;
        int score{};
        unsigned char team{};
        bool isBot{};
        std::string name;
        std::string platform;
        bool ghost_player;
        bool isSpectator;

        // Default constructor
        Pri() {}

        // Constructor using PriWrapper
        Pri(PriWrapper p) {
            if (!p) { return; }
            uid = p.GetUniqueIdWrapper().GetIdString();
            score = p.GetMatchScore();
            team = p.GetTeamNum2();
            isBot = p.GetbBot();
            name = p.GetPlayerName().ToString();
            platform = p.GetPlatform();
            isSpectator = p.IsSpectator();
            ghost_player = team > 1;

            size_t firstPipe = uid.find('|');
            size_t secondPipe = uid.find('|', firstPipe + 1);
            platform = uid.substr(0, firstPipe);

            // Extract the substring between the first and second '|' characters to get the id
            id = uid.substr(firstPipe + 1, secondPipe - firstPipe - 1);

        }

	     bool operator<(const Pri& other) const {
            return name < other.name;
        }
    };

    struct SbPosOffsets offsets;

    struct ScoreboardObj
    {
        unsigned char pad[0xB0];
        wchar_t* sorted_names;
    };

private:
    /**
     * Stores data derived from each scoreboard sort cycle (happens once every second).
     */
 struct ComputedScoreboardInfo {
        std::vector<Pri> sortedPlayers;
        int bluePlayerCount{};
        int orangePlayerCount{};
    };

    Settings settings;

    virtual void onLoad();
    virtual void onUnload();
    /**
     * Pre-compute scoreboard info after the sorting algorithm finishes. The hook
     * "TAGame.PRI_TA.GetScoreboardStats" runs at least once immediately after Rocket League sorts
     * the scoreboard. This happens every second, so computing the info like this saves us some
     * performance.
     */
    void ComputeScoreboardInfo();
    void RecordScoreboardComparison(ActorWrapper gameEvent, void* params, std::string eventName);
    void RenderPlatformLogos(CanvasWrapper canvas);
    std::set<std::string> disconnectedPris;
    std::string GetPluginName() override;
    virtual void RenderSettings() override; 
    virtual void Render() override;
    virtual std::string GetMenuName() override;
    virtual std::string GetMenuTitle() override;
    virtual void SetImGuiContext(uintptr_t ctx) override;
    virtual bool ShouldBlockInput() override;
    virtual bool IsActiveOverlay() override;
    virtual void OnOpen() override;
    virtual void OnClose() override;
    void getSortedIds(ActorWrapper caller);
    bool sortPris(Pri a, Pri b);
    bool hotfix = false;

    // Members for scoreboard tracking logic.
    std::string sortedIds = "";
    std::vector<std::pair<Pri, Pri>> comparisons;
    

    std::filesystem::path settingsFile = gameWrapper->GetDataFolder() / "RankUp" / "settings.json";
    /**
     * teamHistory records the last team that a player (represented by the
     * string combining their name and uid) was seen on. This is necessary,
     * because during ScoreboardSort any disconnected players will have a team
     * number different from Blue or Orange, but we still need to know which team
     * they show up on in the scoreboard display.
     */
    std::unordered_map<std::string, int> teamHistory;
    ComputedScoreboardInfo computedInfo{};  // Derived from comparisons and teamHistory.
    bool accumulateComparisons{};

    // Members for scoreboard rendering.
    bool scoreBoardOpen{};
    LinearColor teamColors[2]{ {255, 255, 255, 255}, {255, 255, 255, 255} };
    const static int LOGO_COUNT = 6;

    std::shared_ptr<ImageWrapper> UserIcon;

    // Cache for storing images based on UID
    std::unordered_map<std::string, std::shared_ptr<ImageWrapper>> imageCache;
    std::mutex imageCacheMutex; // For thread-safe access to imageCache

    // Cache management
    std::filesystem::path cacheDirectory;
    std::filesystem::path cacheJsonFile;
    std::unordered_map<std::string, time_t> cacheTimestamps;
    const int CACHE_EXPIRATION_SECONDS = 7 * 24 * 60 * 60; // One week

    // For managing background image downloads
    std::queue<RankUp::Pri> downloadQueue;
    std::set<RankUp::Pri> queuedDownloads;
    std::mutex downloadQueueMutex;
    std::condition_variable downloadCV;
    bool stopDownloadThread = false;
    std::thread downloadThread;
	std::thread offsetsThread;

    // ImGui settings
    ImGuiStyle UserStyle;

    // Members for menus.
    bool windowOpen{};
    std::string menuTitle{ "RankUp" };

    /*
 *  I would like to say one thing and one thing only. WHY THE FUCK DID I SPEND 30 MINUTES
 *  TRYING TO DEBUG "RankUp cant be a fucking abstract" AND ALL I HAD TO DO WAS UNCOMMENT
 *  THESE 2 FUCKING LINES. I MADE A NEW PROJECT AND I SEE THIS NOW. FUCK YOU
 */

    // ^ there will be more comments like this sorry
    void RenderWindow() override; // Uncomment if you want to render your own plugin window

    // code i havent copied

public:
    void CheckMMRForRankUpdate(); // only works with the current player playing on the same pc stfu yk what i mean

    // brace yourself hardcoded mmr states incoming

    void griDestroyed(std::string eventName);

    void updateDisplay();

    bool show_rank_on;

//    THE MMR STATS START AT SILVER 

    // MMR on the same line is the divisions
    // one line of mmr is a rank (silver 1 champ 2 etc)
    // 3 MMR lines grouped together is a group rank (Plat GC etc)

    // season just started ffs 5 people in ssl 3v3 rn
    
    int ones[73] =
    {
        275, 281, 302, 317,
        335, 342, 359, 377, 
        395, 401, 420, 437, 

        455, 466, 483, 497,
        515, 521, 542, 557,
        575, 585, 598, 617,

        635, 640, 658, 677,
        695, 700, 723, 737,
        755, 760, 778, 797,

        815, 820, 838, 857,
        875, 880, 898, 917,
        935, 940, 958, 977,

        995, 1000, 1019, 1037,
        1055, 1060, 1078, 1097,
        1115, 1122, 1138, 1157,

        1175, 1181, 1198, 1218,
        1236, 1240, 1258, 1277,
        1298, 1299, 1318, 1337,

        1358
    }; // starts at silver

    int twos[73]
    {
        296, 304, 319, 337,
        351, 360, 385, 397,
        415, 423, 439, 457,
        
        475, 484, 502, 517,
        535, 545, 558, 577,
        595, 600, 620, 637,

        655, 660, 682, 697,
        715, 722, 742, 757,
        775, 781, 801, 817,

        835, 845, 871, 892,
        915, 925, 948, 972,
        995, 1005, 1035, 1052,

        1075, 1095, 1128, 1162,
        1195, 1215, 1248, 1282,
        1315, 1335, 1371, 1402,

        1435, 1460, 1498, 1537,
        1575, 1602, 1647, 1677,
        1708, 1744, 1789, 1833,

        1876
    };
    
    void openScoreboard(std::string eventName);
    void closeScoreboard(std::string eventName);

private:
    struct DisplayRank {
        SkillRank skillRank;
        int playlist;
        bool isUnranked;
        bool isSynced;
    };

    struct image {
        std::shared_ptr<ImageWrapper> img;
        Vector2 position;
        float scale;
        LinearColor color;
    };

    struct PlaylistRank {
        SkillRank skillrank;
        int mmr;
        int playlist_id;
        bool isUnranked;
        bool isSynced;
    };
    struct PRanks {
        PlaylistRank ranks[9];
    };

    enum PLCondition {
        NONE = 0,
        EXTRAMODE = 1,
        TOURNAMENT = 2
    };
    struct Playlist {
        std::string name;
        int index;
        char condition;
    };

    const std::map<int, Playlist> PLAYLIST_NAMES = {
        {-1, {"Current", -2, PLCondition::NONE}},
        {0, {"Best", -1, PLCondition::NONE}},
        {10, {"Solo Duel", 0, PLCondition::NONE}},
        {11, {"Doubles", 1, PLCondition::NONE}},
        //{12, {"Solo Standard", 2, PLCondition::NONE}},
        {13, {"Standard", 2, PLCondition::NONE}},
        // {27, {"Hoops", 3, PLCondition::EXTRAMODE}},
        // {28, {"Rumble", 4, PLCondition::EXTRAMODE}},
        // {29, {"Dropshot", 5, PLCondition::EXTRAMODE}},
        // {30, {"Snow Day", 6, PLCondition::EXTRAMODE}},
        // {34, {"Tournaments", 7, PLCondition::TOURNAMENT}}
    };

    enum OffsetKey {
        SCOREBOARD_LEFT,
        BLUE_BOTTOM,
        ORANGE_TOP,
        BANNER_DISTANCE,
        IMAGE_WIDTH,
        IMAGE_HEIGHT,
        CENTER_X,
        CENTER_Y,
        SCOREBOARD_HEIGHT,
        SCOREBOARD_WIDTH,
        IMBALANCE_SHIFT,
        MUTATOR_SIZE,
        SKIP_TICK_SHIFT,
        Y_OFFCENTER_OFFSET,
        UNKNOWN
    };

    
    struct ScoreboardOffsets {
        int SCOREBOARD_LEFT = 537;
        int BLUE_BOTTOM = 67;
        int ORANGE_TOP = 43;
        int BANNER_DISTANCE = 57;
        int IMAGE_WIDTH = 150;
        int IMAGE_HEIGHT = 100;
        int CENTER_X = 960;
        int CENTER_Y = 540;
        int SCOREBOARD_HEIGHT = 548;
        int SCOREBOARD_WIDTH = 1033;
        int IMBALANCE_SHIFT = 32;
        int MUTATOR_SIZE = 478;
        int SKIP_TICK_SHIFT = 67;
        int Y_OFFCENTER_OFFSET = 32;
    };

    // Private match and Custom tournament
    const std::vector<int> EXCLUDED_PLAYLISTS = { 6, 22 };

    DisplayRank displayRankOf(Pri pri, bool include_extras, bool include_tournaments, bool calculate_unranked);

    std::unordered_map<std::string, PRanks> player_ranks;
    std::vector<image> toRender;
    bool isSBOpen = false;
    bool mutators = false;
    bool isReplaying = false;
    float uiScale = 1.0f;
    Vector2 canvas_size = Vector2{ 1920, 1080 }; // Default value for safety
    SbPosInfo sbPosInfo = { Vector2F{0, 0}, Vector2F{0, 0}, BANNER_DISTANCE, 0.48 }; // Default value just to be safe
    ScoreboardOffsets ScoreboardPos;

    int display_playlist = 0;

    bool PluginEnabled;

    void render(CanvasWrapper canvas);

    int iCurrentMMR;
    int iRankDifference;

    int iStatus = 0; // 0 = unknown, 1 = rank up, 2 = maybe, 3 = no

    void RenderDrawable(std::string eventName);
    void UnRenderDrawable(std::string eventName);

    std::vector<int> MMRGainList;

    void createFiles();

    inline bool exists(const std::filesystem::path& name);

    std::ofstream MMRGainListFile;

    // -------------------------------------------------------------------- shit code incoming

    int MMRGain;
    void CalculateMMRGain();
    int newCurrentMMR;

    std::string playlist;
};
