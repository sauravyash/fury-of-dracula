////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"
//#include "DLList.h"
// add your own #includes here

#define PLAY_S_SIZE 7
#define MAX_GAME_SCORE 366
#define MAX_HUNTER_HEALTH 9
#define START_DRAC_POINT 40

typedef struct playerData *PlayerData;
typedef struct vampireData *IVampire;


//ADT for player statuses
struct playerData {
	int health;
	PlaceId * locationHistory;
	PlaceId * currentLocation;
	//DLL for list of locations player has been at? or array (would cover move history as well)
	//will also include move history
	//OR
	//dynamicallyy allocated array of PlaceID's lol
};

struct gameView {
	Round roundNumber;
	int score;
	Player currentPlayer;			//looks like G always starts first? judging by the testfiles given G->S->H->M->D
	PlayerData allPlayers[NUM_PLAYERS];
	PlaceId * trapLocations;
	PlaceId vampire;		//only one vampire alive at any time
	Map map; 				//graph thats been typedefed already

};


// private functions

static void memoryError (const void * input);							//not sure if this works, but for sake of being lazy and not having to write this multiple times
static void initialiseGame (GameView gv, char * pastPlays);			//initialise an empty game to fill in
static void parsePastPlays (GameView gv, char * pastPlays);			//parse through that string

static void memoryError (const void * input){
	if (input == NULL) {
		fprintf(stderr, "Couldn't allocate Memory!\n");
		exit(EXIT_FAILURE);
	}
}

static void initialiseGame (GameView gv) {
	gv->roundNumber = 0;
	gv->score = MAX_GAME_SCORE;
	gv->currentPlayer = PLAYER_LORD_GODALMING; 		//always starts with G
	//Allocate memory for players
	gv->allPlayers[PLAYER_LORD_GODALMING] = malloc(sizeof( * PlayerData));
	memoryError (gv->allPlayers[PLAYER_LORD_GODALMING]);
	gv->allPlayers[PLAYER_LORD_GODALMING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory = NULL;
	gv->allPlayers[PLAYER_DR_SEWARD] = malloc(sizeof( * PlayerData));
	memoryError (gv->allPlayers[PLAYER_DR_SEWARD]);
	gv->allPlayers[PLAYER_LORD_GODALMING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory = NULL;
	gv->allPlayers[PLAYER_VAN_HELSING] = malloc(sizeof( * PlayerData));
	memoryError (gv->allPlayers[PLAYER_VAN_HELSING]);
	gv->allPlayers[PLAYER_LORD_GODALMING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory = NULL;
	gv->allPlayers[PLAYER_MINA_HARKER] = malloc(sizeof( * PlayerData));
	memoryError (gv->allPlayers[PLAYER_MINA_HARKER]);
	gv->allPlayers[PLAYER_LORD_GODALMING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory = NULL;
	gv->allPlayers[PLAYER_DRACULA] = malloc(sizeof( * PlayerData));
	memoryError (gv->allPlayers[PLAYER_DRACULA]);
	gv->allPlayers[PLAYER_LORD_GODALMING] -> health = START_DRAC_POINT;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory = NULL;

	gv->trapLocations = NULL; 		//no trap locations at start of game, therefore no array yet
	gv->vampire = NOWHERE;
	gv->map = MapNew();				//do we have to do anything else with map?

}

static void parsePastPlays (char * pastPlays){
	char * c;
	for (c = pastPlays; *c != '\0'; c++)
	//parse through what those numbers mean
	}
}

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GameView new = malloc(sizeof(*new));
	memoryError(new);
	initialiseGame (new);
	gv->roundNumber = strlen(pastPlays) / (PLAY_S_SIZE + 1);				//each round info is 7 chars + 1 space deliminator
	gv->currentPlayer = gv->roundNumber % 5;									//5 players that always go in order. returns 0 - 4


	return new;
}

void GvFree(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->roundNumber;
}

Player GvGetPlayer(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->currentPlayer;
}

int GvGetScore(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->score;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return player->health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return NOWHERE;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->vampire->location;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
