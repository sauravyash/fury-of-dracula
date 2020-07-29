////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here
#define MAX_LOC_HISTORY_SIZE (GAME_START_SCORE * 2 * 4)


typedef struct playerData *PlayerData;

// ADT for player statuses
struct playerData {
	int health;											// current player health
	PlaceId currentLocation;						  	// current location
};

struct draculaView {
	Round roundNumber;									// current round
	int score;											// current game score
	PlayerData allPlayers[NUM_PLAYERS];					// array of playerData structs
	PlaceId trapLocations[MAX_LOC_HISTORY_SIZE];		// array of trap locations -- multiple traps in same place added seperately
	int trapLocationsIndex;								// index of the most recently added trap location
	PlaceId vampire;									// only one vampire alive at any time
	Map map; 											// graph thats been typedefed already
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}

	return new;
}

void DvFree(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	return GvGetRound(dv);
}

int DvGetScore(DraculaView dv)
{
	return GvGetScore(dv);
}

int DvGetHealth(DraculaView dv, Player player)
{
	return GvGetHealth(dv, player);
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	return GvGetPlayerLocation(dv, player);
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv);
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	// TODO
	//return GvGetTrapLocation(dv);
	*numTraps = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	PlaceId *reachable = GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), 1, 0, 1, *numReturnedMoves);
	// todo needs to be checked against trail
	// + hide
	// + doubleback
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), 1, 0, 1, *numReturnedLocs);
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), road, 0, boat, *numReturnedLocs);
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	GvGetReachable(dv, player, dv->roundNumber, DvGetPlayerLocation(dv, player), *numReturnedLocs);
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	GvGetReachableByType(dv, player, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), road, rail, boat, *numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
