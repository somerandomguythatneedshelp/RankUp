#pragma once
#include <map>
#include "RankEnums.h"

struct FName2
{
    int32_t Index;
    int32_t Instance;
};

struct RankInfo { std::string name; };

//extern std::map<Playlists, PlaylistData&> playlistMMRDatabase;
extern std::map<Rank, RankInfo> RankInfoDB;