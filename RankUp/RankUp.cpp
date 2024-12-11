/* NOTICE */

// THIS PLUGIN IS BASED ON <tracker.network> AND MAY
// BE INACCURATE, NOT MY FAULT IF DATA IS INCORRECT

// ALSO THE MMR STATS START AT SILVER

#include "pch.h"
#include "RankUp.h"

#include <unordered_set>

namespace
{

	BAKKESMOD_PLUGIN(RankUp, "RankUp", plugin_version, PLUGINTYPE_FREEPLAY)
	
	struct SSParams {
		uintptr_t PRI_A;
		uintptr_t PRI_B;

		// if hooking post
		int32_t ReturnValue;
	};

	std::string nameAndId(PriWrapper pri) {
		return pri.GetPlayerName().ToString() + "|" + pri.GetUniqueIdWrapper().GetIdString();
	}

	std::string nameAndId(const RankUp::Pri& p) {
		return p.name + "|" + p.uid;
	}
}

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RankUp::onLoad()
{
	_globalCvarManager = cvarManager;
	LOG("[RankUp] load this baby up (Loaded plugin)");
	PluginEnabled = true;

gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.GFxData_Scoreboard_TA.UpdateSortedPlayerIDs", [this](ActorWrapper caller, ...)
		{
			getSortedIds(caller);
			ComputeScoreboardInfo();
		});
	gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard", std::bind(&RankUp::openScoreboard, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnCloseScoreboard", std::bind(&RankUp::closeScoreboard, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded", [this](std::string eventName) {
		if (isSBOpen) closeScoreboard("");
		});
	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", [this](std::string event_name) {updateDisplay(); });
	gameWrapper->HookEvent("Function TAGame.GRI_TA.Destroyed", std::bind(&RankUp::griDestroyed, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnAllTeamsCreated", [this](std::string eventName) {
		// Called when the match is loaded, so we can set the display playlist if the selection is on -1 (Current)

		if (!cvarManager || !gameWrapper) return;

		CVarWrapper playlist_cvar = cvarManager->getCvar("ingamerank_playlist");
		if (playlist_cvar.IsNull()) return;

		if (playlist_cvar.getIntValue() == -1) {
			// If the playlist selection is "Current" set display playlist to the current playlist

			int current_playlist = gameWrapper->GetMMRWrapper().GetCurrentPlaylist();
			if (PLAYLIST_NAMES.find(current_playlist) == PLAYLIST_NAMES.end()) {
				// If the current playlist is not ranked, set the display to 0 (Best)
				display_playlist = 0;
			}
			else {
				// Otherwise set it to the current playlist
				display_playlist = current_playlist;
			}
		}
		else {
			// Otherwise set it to the selected playlist

			if (PLAYLIST_NAMES.find(playlist_cvar.getIntValue()) == PLAYLIST_NAMES.end()) {
				// Make sure the playlist cvar has a valid value
				display_playlist = 0;
			}
			else {
				display_playlist = playlist_cvar.getIntValue();
			}
		}
		});

	gameWrapper->HookEvent("Function ReplayDirector_TA.Playing.EndState", [this](std::string eventName) {
		// Refresh the display if an instant replay has ended
		isReplaying = false;
		updateDisplay();
		});
	gameWrapper->HookEvent("Function ReplayDirector_TA.Playing.BeginState", [this](std::string eventName) {
		// Refresh the display if an instant replay has started so the images can move out of the way of the check marks
		isReplaying = true;
		updateDisplay();
		});

	// Handle changes in the team and "Ghost Players" staying on the scoreboard
	gameWrapper->HookEventWithCaller<ActorWrapper>(
		"Function TAGame.GameEvent_Soccar_TA.ScoreboardSort",
		[this](ActorWrapper gameEvent, void* params, std::string eventName) {
			RecordScoreboardComparison(gameEvent, params, eventName);
		});
	gameWrapper->HookEventWithCaller<ActorWrapper>(
		"Function TAGame.PRI_TA.GetScoreboardStats",
		[this](auto args...) { ComputeScoreboardInfo();
		});
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", [this](...) {
		comparisons.clear();
		ComputeScoreboardInfo();
		});

	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.StartRound", [this](std::string eventName) {
		CheckMMRForRankUpdate();	
	});
}


void RankUp::griDestroyed(std::string eventName) {
	computedInfo.sortedPlayers.clear();
	player_ranks.clear();
	//mmrs.clear();
	closeScoreboard("");
}

void RankUp::updateDisplay() {
	if (!gameWrapper || !cvarManager) return;
	if (!isSBOpen) return;

	ServerWrapper sw = gameWrapper->IsInOnlineGame() ? gameWrapper->GetOnlineGame() : gameWrapper->GetGameEventAsServer();
	if (sw.IsNull()) return;

	MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();
	if (sw.GetbMatchEnded()) {
		closeScoreboard("");
		return; 
	}

	// whether the 'show player mmr' cvars are enabled
	show_rank_on = cvarManager->getCvar("ranked_showranks").getBoolValue() && cvarManager->getCvar("ranked_showranks_casual").getBoolValue();

	toRender.clear();

	canvas_size = gameWrapper->GetScreenSize();
	uiScale = gameWrapper->GetInterfaceScale() * gameWrapper->GetDisplayScale();
	mutators = mmrWrapper.GetCurrentPlaylist() == 34; // When playing a tournament
	//sbPosInfo = getSbPosInfo(canvas_size, uiScale, mutators, computedInfo.bluePlayerCount, computedInfo.orangePlayerCount, SbPosIt);

	//Get cvars
	int playlist = display_playlist;
	bool show_division = cvarManager->getCvar("ingamerank_show_division").getBoolValue();
	bool calculate_unranked = cvarManager->getCvar("ingamerank_calculate_unranked").getBoolValue();
	bool include_extras = cvarManager->getCvar("ingamerank_include_extramodes").getBoolValue();
	bool include_tournaments = cvarManager->getCvar("ingamerank_include_tournaments").getBoolValue();
	bool show_playlist = cvarManager->getCvar("ingamerank_show_playlist").getBoolValue();

	if (PLAYLIST_NAMES.find(playlist) == PLAYLIST_NAMES.end()) { //This should be handled in the OnValueChanged but better safe than sorry.
		playlist = 0;
	}

	int blues = -1;  // For counting players to be able to calculate image positions
	int oranges = -1;
	for (Pri pri : computedInfo.sortedPlayers)
	{
		if (pri.team == 0) blues++;
		else if (pri.team == 1) oranges++;
		if (pri.isBot || pri.team > 1) continue;

		DisplayRank displayRank = displayRankOf(pri, include_extras, include_tournaments, calculate_unranked);

		//precomputeRankImages(pri, displayRank, oranges, blues, playlist, show_division, show_playlist, calculate_unranked);
	}
	gameWrapper->UnregisterDrawables();
	gameWrapper->RegisterDrawable(std::bind(&RankUp::render, this, std::placeholders::_1));
}

void RankUp::render(CanvasWrapper canvas) {
	// Images are "cached" in the toRender variable so we don't have to recalculate positions and images every frame just render the precalculated images
	
	for (image img : toRender)
	{
		canvas.SetColor(img.color);
		canvas.SetPosition(img.position);
		if (img.img.get()->IsLoadedForCanvas()) {
			canvas.DrawTexture(img.img.get(), img.scale);
		}
		else {
			img.img.get()->LoadForCanvas();
		}

#ifdef _DEBUG
		//Draws a box around the rendered images
		int X1 = img.position.X;
		int X2 = X1 + int(img.img->GetSize().X * img.scale);
		int Y1 = img.position.Y;
		int Y2 = Y1 + int(img.img->GetSize().Y * img.scale);
		canvas.DrawLine(Vector2{ X1, Y1 }, Vector2{ X2, Y1 });
		canvas.DrawLine(Vector2{ X1, Y1 }, Vector2{ X1, Y2 });
		canvas.DrawLine(Vector2{ X1, Y2 }, Vector2{ X2, Y2 });
		canvas.DrawLine(Vector2{ X2, Y1 }, Vector2{ X2, Y2 });
#endif

	}
	canvas.SetColor((char)255, (char)255, (char)255, (char)255);

	if (!show_rank_on) {
		// If show player mmr is disabled in bakkesmod settings notify the player with a red message
		canvas.SetColor(255, 0, 0, 255);
		canvas.SetPosition(Vector2{ 0, 0 });
		canvas.DrawString("Turn on \"Ranked->Show player MMR on scoreboard\" and \"Ranked->Show MMR in casual playlists\" in the bakkesmod menu!", 1.5f, 1.5f);
	}

#ifdef _DEBUG
	int offset = 100;
	for (auto id : sortednames) {
		canvas.SetPosition(Vector2{10, offset});
		canvas.DrawString(id, 2.0f, 2.0f);
		offset += 25;
	}
#endif // _DEBUG


	//renderPlaylist(canvas);
}

void RankUp::onUnload()
{
	LOG("[RankUp] byeeeeee");
	PluginEnabled = false;
}


void RankUp::ComputeScoreboardInfo() {
	if (!accumulateComparisons) {
		return;
	}
	accumulateComparisons = false;

	auto hash = [](const Pri& p) { return std::hash<std::string>{}(nameAndId(p)); };
	auto keyEqual = [](const Pri& lhs, const Pri& rhs) { return nameAndId(lhs) == nameAndId(rhs); };
	std::unordered_set<Pri, decltype(hash), decltype(keyEqual)> seenPris{ 10, hash, keyEqual };

	disconnectedPris.clear();
	for (const auto& comparison : comparisons) {
		seenPris.insert(comparison.first);
		seenPris.insert(comparison.second);

		if (comparison.first.ghost_player) {
			disconnectedPris.insert(nameAndId(comparison.first));
		}
		if (comparison.second.ghost_player) {
			disconnectedPris.insert(nameAndId(comparison.second));
		}
	}

	std::vector<Pri> seenPrisVector;
	int numBlues{};
	int numOranges{};
	for (auto pri : seenPris) {
		pri.team = teamHistory[nameAndId(pri)];

		if (pri.team > 1) disconnectedPris.insert(nameAndId(pri));
		if (disconnectedPris.find(nameAndId(pri)) != disconnectedPris.end()) {
			pri.ghost_player = true;
		}

		if (pri.team == 0 && !pri.isSpectator) {
			numBlues++;
		}
		else if (pri.team == 1 && !pri.isSpectator) {
			numOranges++;
		}
		seenPrisVector.push_back(pri);
	}
	std::sort(seenPrisVector.begin(), seenPrisVector.end(), [this](const Pri& a, const Pri& b) { return sortPris(a, b); });
	computedInfo = ComputedScoreboardInfo{ seenPrisVector, numBlues, numOranges };
}

void RankUp::RenderPlatformLogos(CanvasWrapper canvas) {

    if (!scoreBoardOpen) { return; }
    if (!gameWrapper->IsInOnlineGame()) { return; }
    ServerWrapper sw = gameWrapper->GetOnlineGame();
    if (!sw) { return; }
    if (sw.GetbMatchEnded()) { return; }

    LinearColor blueColor = teamColors[0];
    LinearColor orangeColor = teamColors[1];

	MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();
	Vector2 screenSize = gameWrapper->GetScreenSize();
	Vector2F screenSizeFloat(screenSize.X, screenSize.Y);
	SbPosInfo sbPosInfo = getSbPosInfo(screenSizeFloat,
		gameWrapper->GetDisplayScale() * gameWrapper->GetInterfaceScale(),
		/* mutators= */ mmrWrapper.GetCurrentPlaylist() == 34,
		computedInfo.bluePlayerCount,
		computedInfo.orangePlayerCount, offsets);

    // int blues = -1;
    // int oranges = -1;
    //
    // Vector2F imageShift = { 0, 0 };
    //
    // for (auto pri : computedInfo.sortedPlayers) {
    //
    //     if (pri.isSpectator) continue;
    //
    //     Vector2F drawPos{};
    //
    //     if (pri.team == 0) {
    //         blues++;
    //         canvas.SetColor(blueColor);
    //         if (pri.ghost_player) canvas.SetColor(LinearColor{ blueColor.R, blueColor.G, blueColor.B, 155 / 1.5 });
    //         drawPos = sbPosInfo.blueLeaderPos + Vector2F{ 0, sbPosInfo.playerSeparation * blues } + imageShift;
    //     }
    //     else if (pri.team == 1) {
    //         oranges++;
    //
    //         canvas.SetColor(orangeColor);
    //         if (pri.ghost_player) canvas.SetColor(LinearColor{ orangeColor.R, orangeColor.G, orangeColor.B, 155 / 1.5 });
    //         drawPos = sbPosInfo.orangeLeaderPos + Vector2F{ 0, sbPosInfo.playerSeparation * oranges } + imageShift;
    //     }
    //     else {
    //         LOG("[RLProfilePictures] Unexpected team value {} for pri {}", pri.team, nameAndId(pri));
    //         continue;
    //     }
    //     if (pri.isBot) { continue; }
    //
    //     canvas.SetPosition(drawPos);
    //
    //     // Use the UID to get the appropriate image
    //     std::shared_ptr<ImageWrapper> image = GetImageForPlayer(pri);
    //     if (image && image->IsLoadedForCanvas()) {
    //         canvas.DrawTexture(image.get(), 100.0f / 48.0f * sbPosInfo.profileScale); // Scale images accordingly
    //     }
    //     // Else, do not draw anything
    // }

	

	
} // this is going to kill me

void RankUp::RecordScoreboardComparison(ActorWrapper gameEvent, void* params, std::string eventName) {
	if (!accumulateComparisons) {
		accumulateComparisons = true;
		comparisons.clear();
	}
	SSParams* p = static_cast<SSParams*>(params);

	if (!p) { LOG("NULL SSParams"); return; }
	PriWrapper a(p->PRI_A);
	PriWrapper b(p->PRI_B);

	comparisons.push_back({ a, b });
	auto teamNumA = a.GetTeamNum2();
	if (teamNumA <= 1) {
		teamHistory[nameAndId(a)] = teamNumA;
	}

	auto teamNumB = b.GetTeamNum2();
	if (teamNumB <= 1) {
		teamHistory[nameAndId(b)] = teamNumB;
	}
}

void RankUp::getSortedIds(ActorWrapper caller) {

	auto* scoreboard = reinterpret_cast<ScoreboardObj*>(caller.memory_address);
	if (scoreboard->sorted_names == nullptr) return;
	auto sorted_names = std::wstring(scoreboard->sorted_names);

	std::string str;
	std::transform(sorted_names.begin(), sorted_names.end(), std::back_inserter(str), [](wchar_t c) {
		return (char)c;
		});
	sortedIds = str;
}

bool RankUp::sortPris(Pri a, Pri b) {
	std::string id_a = a.uid;
	std::string id_b = b.uid;

	if (a.isBot) id_a = "Bot_" + a.name;
	if (b.isBot) id_b = "Bot_" + b.name;

	size_t index_a = sortedIds.find(id_a);
	size_t index_b = sortedIds.find(id_b);
	if (index_a != std::string::npos && index_b != std::string::npos) {
		return index_a < index_b;
	}
	else {
		return a.score > b.score;
	}
}

void RankUp::CheckMMRForRankUpdate()
{
	// assume this shit is ranked

	if (!gameWrapper->GetMMRWrapper().IsRanked(gameWrapper->GetMMRWrapper().GetCurrentPlaylist())) { return;}

	int currentMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(gameWrapper->GetUniqueID(), gameWrapper->GetMMRWrapper().GetCurrentPlaylist());

	// Use std::lower_bound to find the first element >= input
	const int* next = std::lower_bound(ones, ones + 73, currentMMR);

	if (next == ones + 73) {
		//std::cout << "No number found greater than or equal to " << input << '\n';
	} else { // TODO: voodoo shit
		//std::cout << "Next closest number: " << *next << '\n';
		cvarManager->log("[RANKUP EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE] Closest number: " + *next);

		// average mmr gain is +9/-9 MMR

		// 12+ MMR not ranking up defo (i hope)
		// 5-12 MMR maybe
		// 0-5 MMR defo

		const int RankLeft = *next - currentMMR;

		LOG("Current MMR: " + std::to_string(currentMMR) + ", Next Rank MMR: " + std::to_string(*next) + ", Rank Difference: " + std::to_string(RankLeft));
		/*
		 *  *next and currentMMR SHOULD already have a value so RankLeft shouldent be null. Im hoping 21:50 10/12
		 *  Ok i think it works its displays 16 MMR difference but i cant be asked to fact check it 21:56 10/12
		 */

		// nested code incoming

		
	}
}

void RankUp::openScoreboard(std::string eventName) {
	//if (!*PluginEnabled) return;
	if (!gameWrapper) return;
#ifdef _DEBUG
	// When _DEBUG is defined allow the plugin to run in private games aswell
	if (!gameWrapper->IsInGame() && !gameWrapper->IsInOnlineGame() || gameWrapper->IsInFreeplay() || gameWrapper->IsInReplay()) return;
#else
	// Else only allow in online
	if (!gameWrapper->IsInOnlineGame()) return;
	MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();
	if (std::find(EXCLUDED_PLAYLISTS.begin(), EXCLUDED_PLAYLISTS.end(), mmrWrapper.GetCurrentPlaylist()) != EXCLUDED_PLAYLISTS.end()) return;
#endif
	isSBOpen = true;

	//updateDisplay();
}

void RankUp::closeScoreboard(std::string eventName) {
	if (!gameWrapper) return;

	if (isSBOpen) {
		gameWrapper->UnregisterDrawables();
		toRender.clear();
		isSBOpen = false;
	}
}


RankUp::DisplayRank RankUp::displayRankOf(Pri pri, bool include_extras, bool include_tournaments, bool calculate_unranked) {
	//-------Decide Rank To Display------------
	SkillRank sr;
	int playlist = display_playlist;
	int temp_playlist = playlist;
	bool isUnranked = false;
	bool isSynced = true;
	std::string idString = pri.uid;
	// if (player_ranks.find(idString) == player_ranks.end()) {
	// 	updateRankFor(pri.uid); 
	// }

	// ^ i dont know what that does but i hope its nothing important 8/12 11:09

	if (playlist == 0) { //"Best"
		int best_pl = 0;
		SkillRank best_rank = { -1, 0, 0 };
		for (auto& pid: PLAYLIST_NAMES)
		{
			if (pid.first <= 0) continue;
			if (pid.second.condition == PLCondition::EXTRAMODE && !include_extras || pid.second.condition == PLCondition::TOURNAMENT && !include_tournaments) continue;
			PlaylistRank current = player_ranks[idString].ranks[pid.second.index];
			if (!current.isSynced) continue;

			if (current.isUnranked && !calculate_unranked) {
				if (best_rank.Tier < 0) {
					best_rank.Tier = 0;
					best_rank.Division = -1;
				}
			} else if (best_rank.Tier + (best_rank.Division + 1) * 0.1f < current.skillrank.Tier + (current.skillrank.Division + 1) * 0.1f) {
				best_rank = current.skillrank;
				best_pl = current.playlist_id;
				isUnranked = current.isUnranked;
			}
		}
		sr = best_rank;
		temp_playlist = best_pl;
		if (best_rank.Tier < 0) isSynced = false;
	}
	else { //Any other
		PlaylistRank current = player_ranks[idString].ranks[PLAYLIST_NAMES.at(playlist).index];
		if (current.isUnranked && !calculate_unranked) {
			sr.Tier = 0;
			sr.Division = -1;
		} else sr = current.skillrank;
		isUnranked = current.isUnranked;
		isSynced = current.isSynced;
	}
	if (sr.Tier == 22) {
		sr.Division = -1;
	}
	if (sr.Tier == 0) {
		sr.Division = -1;
		isUnranked = false;
	}
	if (!isSynced) { 
		sr.Tier = 23;
		sr.Division = -1;
		isUnranked = false;
	}

	DisplayRank output;
	output.skillRank = sr;
	output.isUnranked = isUnranked;
	output.isSynced = isSynced;
	output.playlist = temp_playlist;
	return output;
}