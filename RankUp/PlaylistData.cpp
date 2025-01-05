#include "pch.h"
#include "PlaylistData.h"

std::map<Rank, RankInfo> RankInfoDB
{
        {Rank::Unranked, {"Unranked"}},
        {Rank::Bronze1, {"Bronze 1"}},
        {Rank::Bronze2, {"Bronze 2"}},
        {Rank::Bronze3, {"Bronze 3"}},
        {Rank::Silver1, {"Silver 1"}},
        {Rank::Silver2, {"Silver 2"}},
        {Rank::Silver3, {"Silver 3"}},
        {Rank::Gold1, {"Gold 1"}},
        {Rank::Gold2, {"Gold 2"}},
        {Rank::Gold3, {"Gold 3"}},
        {Rank::Platinum1, {"Platinum 1"}},
        {Rank::Platinum2, {"Platinum 2"}},
        {Rank::Platinum3, {"Platinum 3"}},
        {Rank::Diamond1, {"Diamond 1"}},
        {Rank::Diamond2, {"Diamond 2"}},
        {Rank::Diamond3, {"Diamond 3"}},
        {Rank::Champ1, {"Champion 1"}},
        {Rank::Champ2, {"Champion 2"}},
        {Rank::Champ3, {"Champion 3"}},
        {Rank::GrandChamp1, {"Grand Champion 1"}},
        {Rank::GrandChamp2, {"Grand Champion 2"}},
        {Rank::GrandChamp3, {"Grand Champion 3"}},
        {Rank::SupersonicLegend, {"Supersonic Legend"}}
};