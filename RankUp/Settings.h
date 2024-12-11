#pragma once
#include <./json.hpp>

struct Settings {
    /*bool showEpic = true;
    bool showSteam = true;
    bool showPSN = true;
    bool showXbox = true;
    bool showSwitch = true;*/

    long long ago; // in another galaxy

    // ^ copied from another repo, not needed for this repo

    // Enable JSON serialization/deserialization for the new fields
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Settings, ago)
};