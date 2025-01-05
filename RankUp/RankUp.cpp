#include "pch.h"
#include "RankUp.h"


BAKKESMOD_PLUGIN(RankUp, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RankUp::onLoad()
{
	_globalCvarManager = cvarManager;
	//LOG("Plugin loaded!");
	
}
