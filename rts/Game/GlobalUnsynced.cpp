/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

/**
 * @brief Globally accessible stuff
 * Contains implementation of synced and unsynced global stuff.
 */

#include "GlobalUnsynced.h"

#include "Game/Players/Player.h"
#include "Game/Players/PlayerHandler.h"
#include "Sim/Misc/TeamHandler.h"
#include "System/Config/ConfigHandler.h"
#include "System/Exceptions.h"
#include "System/Util.h"
#include "System/creg/creg_cond.h"
#include "System/Misc/SpringTime.h"
#include "System/Sync/SyncTracer.h"

#include <ctime>


/**
 * @brief global unsynced
 *
 * Global instance of CGlobalUnsynced
 */
CGlobalUnsynced* gu;
CGlobalUnsyncedRNG guRNG;

const float CGlobalUnsynced::reconnectSimDrawBalance = 0.15f;

CR_BIND(CGlobalUnsynced, )

CR_REG_METADATA(CGlobalUnsynced, (
	CR_IGNORED(simFPS),
	CR_IGNORED(avgSimFrameTime),
	CR_IGNORED(avgDrawFrameTime),
	CR_IGNORED(avgFrameTime),
	CR_MEMBER(modGameTime),
	CR_MEMBER(gameTime),
	CR_MEMBER(startTime),
	CR_MEMBER(myPlayerNum),
	CR_MEMBER(myTeam),
	CR_MEMBER(myAllyTeam),
	CR_MEMBER(myPlayingTeam),
	CR_MEMBER(myPlayingAllyTeam),
	CR_MEMBER(spectating),
	CR_MEMBER(spectatingFullView),
	CR_MEMBER(spectatingFullSelect),
	CR_IGNORED(fpsMode),
	CR_IGNORED(globalQuit),
	CR_IGNORED(globalReload),
	CR_IGNORED(reloadScript)
))

CGlobalUnsynced::CGlobalUnsynced()
{
	guRNG.Seed(time(nullptr) % ((spring_gettime().toNanoSecsi() + 1) * 9007));

	assert(playerHandler == nullptr);
	ResetState();
}

CGlobalUnsynced::~CGlobalUnsynced()
{
	spring::SafeDelete(playerHandler);
	assert(playerHandler == nullptr);
}


void CGlobalUnsynced::ResetState()
{
	simFPS = 0.0f;

	avgSimFrameTime = 0.001f;
	avgDrawFrameTime = 0.001f;
	avgFrameTime = 0.001f;

	modGameTime = 0;
	gameTime = 0;
	startTime = 0;

	myPlayerNum = 0;
	myTeam = 1;
	myAllyTeam = 1;
	myPlayingTeam = -1;
	myPlayingAllyTeam = -1;

	spectating           = false;
	spectatingFullView   = false;
	spectatingFullSelect = false;

	fpsMode = false;
	globalQuit = false;
	globalReload = false;
	reloadScript = "";

	if (playerHandler == nullptr) {
		playerHandler = new CPlayerHandler();
	} else {
		playerHandler->ResetState();
	}
}

void CGlobalUnsynced::LoadFromSetup(const CGameSetup* setup)
{
	playerHandler->LoadFromSetup(setup);
}


void CGlobalUnsynced::SetMyPlayer(const int myNumber)
{
	myPlayerNum = myNumber;

#ifdef TRACE_SYNC
	tracefile.Initialize(myPlayerNum);
#endif

	const CPlayer* myPlayer = playerHandler->Player(myPlayerNum);

	myTeam = myPlayer->team;
	if (!teamHandler->IsValidTeam(myTeam)) {
		throw content_error("Invalid MyTeam in player setup");
	}

	myAllyTeam = teamHandler->AllyTeam(myTeam);
	if (!teamHandler->IsValidAllyTeam(myAllyTeam)) {
		throw content_error("Invalid MyAllyTeam in player setup");
	}

	spectating           = myPlayer->spectator;
	spectatingFullView   = myPlayer->spectator;
	spectatingFullSelect = myPlayer->spectator;

	if (!spectating) {
		myPlayingTeam = myTeam;
		myPlayingAllyTeam = myAllyTeam;
	}
}

CPlayer* CGlobalUnsynced::GetMyPlayer() {
	return (playerHandler->Player(myPlayerNum));
}

