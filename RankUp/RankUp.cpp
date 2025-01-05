#include "pch.h"
#include "RankUp.h"

BAKKESMOD_PLUGIN(RankUp, "Rank Up", "1.0.0", PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RankUp::onLoad()
{
	_globalCvarManager = cvarManager;
	//LOG("Plugin loaded!");
}

void RankUp::onUnload()
{
	
}


// Gui Code

void RankUp::RenderSettings()
{
	ImGui::Text("Hello, world!");
}

void RankUp::SetImGuiContext(uintptr_t ctx)
{
	
}

std::string RankUp::GetPluginName()
{
	return "RankUp";
}



