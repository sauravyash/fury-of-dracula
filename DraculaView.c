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
#include <string.h>

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

static PlayerData initialisePlayer(DraculaView dv, Player player);
static void initialiseGame (DraculaView dv);
static Player parseMove (DraculaView dv, char *string);


// MEMORY ERROR: Helper function to check correct memory allocation. Exits if
// memory was not correctly allocated
// -- INPUT: pointer to a malloced object
// -- OUTPUT: void, but prints error message and exits code 1
static void memoryError (const void * input) {
	if (input == NULL) {
		fprintf(stderr, "Couldn't allocate Memory!\n");
		exit(EXIT_FAILURE);
	}
	return;
}

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO not functional
	// Allocate memory for new dv
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
	// TODO
	MapFree(dv->map);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	return dv->roundNumber;
}

int DvGetScore(DraculaView dv)
{
	return dv->score;
}

int DvGetHealth(DraculaView dv, Player player)
{
	return dv->allPlayers[player]->health;
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	return dv->allPlayers[player]->currentLocation;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return dv->allPlayers[PLAYER_DRACULA]->currentLocation;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	// TODO
	*numTraps = dv->trapLocationsIndex + 1;
	PlaceId *traps = malloc(sizeof(PlaceId) * *numTraps);
	
	for (int i = 0; i < *numTraps; i++) {
		traps[i] = dv->trapLocations[i];
	}
	return traps;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	//PlaceId *reachable = GvGetReachableByType(dv, PLAYER_DRACULA, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), 1, 0, 1, numReturnedMoves);
	// todo needs to be checked against trail
	// + hide
	// + doubleback
	*numReturnedMoves = 0;
	return NULL;

}
// adapted from Gameview
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	// 1. We need to access the map :)
	Map map = dv->map;

	// create dynamically allocated array 
	PlaceId *visited = malloc(sizeof(PlaceId));

	// get connections from current location
	ConnList list = MapGetConnections(map, dv->allPlayers[PLAYER_DRACULA]->currentLocation);

	// 3. Iterate through...
	int loc_num = 0;
	visited[loc_num] = dv->allPlayers[PLAYER_DRACULA]->currentLocation;

	while (loc_num < NUM_REAL_PLACES) {

	    // Extra conditions for drac:
		if (list->p == HOSPITAL_PLACE) continue;
		if (list->type == RAIL) continue;

	    // If it is a road type.
	    if (list->type == ROAD) {
	        visited[loc_num] = list->p;
	        loc_num++;
			visited = realloc(visited, loc_num * sizeof(PlaceId));
	    }

		// If it is a boat type.
		else if (list->type == BOAT) {
	        visited[loc_num] = list->p;
	        loc_num++;
			visited = realloc(visited, loc_num * sizeof(PlaceId));
	    }

	    if (list->next == NULL) break;
	    list = list->next;
	}

    // Return values
	*numReturnedLocs = loc_num;
	return visited;
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	// 1. We need to access the map :)
	Map map = dv->map;

	// create dynamically allocated array 
	PlaceId *visited = malloc(sizeof(PlaceId));

	// get connections from current location
	ConnList list = MapGetConnections(map, dv->allPlayers[PLAYER_DRACULA]->currentLocation);

	// 3. Iterate through...
	int loc_num = 0;
	visited[loc_num] = dv->allPlayers[PLAYER_DRACULA]->currentLocation;

	while (loc_num < NUM_REAL_PLACES) {

	    // Extra conditions for drac:
		if (list->p == HOSPITAL_PLACE) continue;
		if (list->type == RAIL) continue;

	    // If it is a road type and bool road is true
	    if (list->type == ROAD && road) {
	        visited[loc_num] = list->p;
	        loc_num++;
			visited = realloc(visited, loc_num * sizeof(PlaceId));
	    }

		// If it is a boat type and bool boat is true
		else if (list->type == BOAT && boat) {
	        visited[loc_num] = list->p;
	        loc_num++;
			visited = realloc(visited, loc_num * sizeof(PlaceId));
	    }

	    if (list->next == NULL) break;
	    list = list->next;
	}

    // Return values
	*numReturnedLocs = loc_num;
	return visited;
}


PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	//return GvGetReachable(dv, player, dv->roundNumber, DvGetPlayerLocation(dv, player), numReturnedLocs);
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	//return GvGetReachableByType(dv, player, dv->roundNumber, DvGetPlayerLocation(dv, PLAYER_DRACULA), road, rail, boat, numReturnedLocs);
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO


// INITIALISE PLAYER: Initialises a player to defaults, assigns memory
// -- INPUT: DraculaView, Player
// -- OUTPUT: void
static PlayerData initialisePlayer(DraculaView dv, Player player) {

	PlayerData new = malloc(sizeof(* new));
	memoryError (new);

	new -> health = GAME_START_HUNTER_LIFE_POINTS;
	new -> currentLocation = NOWHERE;

	if (player == PLAYER_DRACULA) {
		new -> health = GAME_START_BLOOD_POINTS;
	} else {
		new -> health = GAME_START_HUNTER_LIFE_POINTS;
	}

	return new;
}

// INITIALISE GAME: Assigns memory and sets values to default/null values
// -- INPUT: DraculaView
// -- OUTPUT: void
static void initialiseGame (DraculaView dv) {

	dv->roundNumber = 0;
	dv->score = GAME_START_SCORE;
	// Always starts with G
	dv->currentPlayer = PLAYER_LORD_GODALMING;

	// Allocate memory for players & Initialise starting information
	dv->allPlayers[PLAYER_LORD_GODALMING] = initialisePlayer(dv, PLAYER_LORD_GODALMING);
	dv->allPlayers[PLAYER_DR_SEWARD] = initialisePlayer(dv, PLAYER_DR_SEWARD);
	dv->allPlayers[PLAYER_VAN_HELSING] = initialisePlayer(dv, PLAYER_VAN_HELSING);
	dv->allPlayers[PLAYER_MINA_HARKER] = initialisePlayer(dv, PLAYER_MINA_HARKER);
	dv->allPlayers[PLAYER_DRACULA] = initialisePlayer(dv, PLAYER_DRACULA);

	//for (int player = 0; player < NUM_PLAYERS; player++) {
	//	initialisePlayer(dv, player);
	//}

    // No trap locations at start of game, therefore no array yet..
	// dv->trapLocations = NULL;
	dv->trapLocationsIndex = -1;
	dv->vampire = NOWHERE;

	// Nothing else to do for map- read Map.c --> the functions take care of
	// adding all connections.
	// dv->map = MapNew();
	return;

}

// PARSE MOVE: interprets a single move, calls hunter/draculaMove, updates curr_player
// -- Input: DraculaVie, move string
// -- Output: current Player
// Author: Cindy (Tara edited)
static Player parseMove (DraculaView dv, char *string) {

	char *c = string;
	Player curr_player;

	// figure out whose move it was
	// TODO implement move functions
	switch(*c){
			case 'G':
			    printf("it is Lord G\n");
			    //hunterMove(dv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;

			case 'S':
			    printf("it is Dr S\n");
			    //hunterMove(dv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_VAN_HELSING;
			    break;

			case 'H':
			    printf("it is VH\n");
			    //hunterMove(dv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_MINA_HARKER;
			    break;

			case 'M':
			   printf("it is Mina\n");
			    //hunterMove(dv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;

			case 'D':
			    printf("it is Drac\n");
			    //draculaMove(dv, string);
			    curr_player = PLAYER_LORD_GODALMING;
			    break;
		}

	return curr_player;
}