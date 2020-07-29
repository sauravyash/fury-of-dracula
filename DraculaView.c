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
#define PLAY_S_SIZE 7


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
	Player currentPlayer;								// looks like G always starts first? judging by the testfiles given G->S->H->M->D
	PlaceId trapLocations[MAX_LOC_HISTORY_SIZE];		// array of trap locations -- multiple traps in same place added seperately
	int trapLocationsIndex;								// index of the most recently added trap location
	PlaceId vampire;									// only one vampire alive at any time
	Map map; 											// graph thats been typedefed already
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO not functional
	// Allocate memory for new GV
	DraculaView new = malloc(sizeof(*new));

	// Check if memory was allocated correctly
	memoryError(new);
	new->map = MapNew();
	initialiseGame(new);

    // Number of Rounds can be determined from size of string
    // ( each play is 7 chars + space)
    // Note last move has no space...
    // Note each round is 5 plays...
	new->roundNumber = (strlen(pastPlays) + 1) / ((PLAY_S_SIZE + 1)*5);

	// 5 players that always go in order of G->S->H->M->D. returns 0 - 4
	// Player is not dependent on round...
	// Just set it here to player zero- the zero case, then change in parsMove.
	// Already done in initialiseGame.

    // Need to create a copy of pastPlays rather than just pointing to it.
	char string[strlen(pastPlays)];
	strcpy(string, pastPlays);

	// Iterate through past plays...
	char *token = strtok(string, " ");
	// while not end of string
	while (token != NULL) {
		// printf("current token: %d\n", token);
		new->currentPlayer = parseMove(new, token);
		token = strtok(NULL, " ");
	}

	//fill out map???

	return new;
}

void DvFree(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	MapFree(dv->map);
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
	PlaceId *reachable = GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), 1, 0, 1, numReturnedMoves);
	// todo needs to be checked against trail
	// + hide
	// + doubleback
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	return GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), 1, 0, 1, numReturnedLocs);
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	return GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), road, 0, boat, numReturnedLocs);
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	return GvGetReachable(dv, player, dv->roundNumber, DvGetPlayerLocation(dv, player), numReturnedLocs);
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	return GvGetReachableByType(dv, player, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), road, rail, boat, numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
