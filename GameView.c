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
#include <string.h>

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
#define LOCATION_ID_SIZE 2
#define MAX_LOCATION_HISTORY_SIZE (MAX_GAME_SCORE * 2 * 4)
//DOUBLE CHECK THIS: max number of turns * most possible avg moves per turn (rounded up)  * bytes per move stored

//some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'

//#defines to make things more readable?
//#define gv->allPlayers[PLAYER_LORD_GODALMING] LORD_GODALMING or LordGodalming?
//#define gv->allPlayers[hunter]->currentLocationIndex locationIndex

typedef struct playerData *PlayerData;
typedef struct vampireData *IVampire;


//ADT for player statuses
struct playerData {
	int health;
	PlaceId locationHistory[MAX_LOCATION_HISTORY_SIZE];
	PlaceId currentLocation;					// current location
	int currentLocationIndex;					// index of current location in locationHistory
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
static void initialiseGame (GameView gv);								//initialise an empty game to fill in
static void parseMove (GameView gv, char * string);						//parse through that string
static void hunterMove(GameView gv, char * string, Player hunter);
static void draculaMove(GameView gv, char * string);
//these are here for now for easy access, will move them to bottom later
static void memoryError (const void * input){
	if (input == NULL) {
		fprintf(stderr, "Couldn't allocate Memory!\n");
		exit(EXIT_FAILURE);
	}
}

// appends input placeid to locationhistory, updates current location and index
static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location) {
	int index = gv->allPlayers[hunter]->currentLocationIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->allPlayers[hunter]->locationHistory[index + 1] = location;
		gv->allPlayers[hunter]->currentLocation = location;
	}
}
static void vampireLocationHistoryAppend(GameView gv, Player hunter, char *location);


static void initialiseGame (GameView gv) {
	gv->roundNumber = 0;
	gv->score = MAX_GAME_SCORE;
	gv->currentPlayer = PLAYER_LORD_GODALMING; 		//always starts with G
	//Allocate memory for players
	gv->allPlayers[PLAYER_LORD_GODALMING] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_LORD_GODALMING]);
	gv->allPlayers[PLAYER_LORD_GODALMING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_LORD_GODALMING] -> currentLocationIndex = -1;
	//gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory = NULL;

	gv->allPlayers[PLAYER_DR_SEWARD] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_DR_SEWARD]);
	gv->allPlayers[PLAYER_DR_SEWARD] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_DR_SEWARD] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_DR_SEWARD] -> currentLocationIndex = -1;
	//gv->allPlayers[PLAYER_DR_SEWARD] -> locationHistory = NULL;

	gv->allPlayers[PLAYER_VAN_HELSING] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_VAN_HELSING]);
	gv->allPlayers[PLAYER_VAN_HELSING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_VAN_HELSING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_VAN_HELSING] -> currentLocationIndex = -1;
	//gv->allPlayers[PLAYER_VAN_HELSING] -> locationHistory = NULL;

	gv->allPlayers[PLAYER_MINA_HARKER] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_MINA_HARKER]);
	gv->allPlayers[PLAYER_MINA_HARKER] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_MINA_HARKER] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_MINA_HARKER] -> currentLocationIndex = -1;
	//gv->allPlayers[PLAYER_MINA_HARKER] -> locationHistory = NULL;

	gv->allPlayers[PLAYER_DRACULA] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_DRACULA]);
	gv->allPlayers[PLAYER_DRACULA] -> health = START_DRAC_POINT;
	gv->allPlayers[PLAYER_DRACULA] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_DRACULA] -> currentLocationIndex = -1;
	//gv->allPlayers[PLAYER_DRACULA] -> locationHistory = NULL;

	gv->trapLocations = NULL; 		//no trap locations at start of game, therefore no array yet
	gv->vampire = NOWHERE;
	gv->map = MapNew();				//do we have to do anything else with map?

	// allocate initial memory for locationHistory
}

static void parseMove (GameView gv, char * string){
	char * c = string;
	//figure out who's move it was
	switch(*c){
			case 'G':
			printf("it is Lord G\n");
			hunterMove(gv, string, PLAYER_LORD_GODALMING);
			case 'S':
			printf("it is Dr S\n");
			hunterMove(gv, string, PLAYER_DR_SEWARD);
			case 'H':
			printf("it is VH\n");
			hunterMove(gv, string, PLAYER_VAN_HELSING);
			case 'M':
			printf("it is Mina\n");
			hunterMove(gv, string, PLAYER_MINA_HARKER);
			case 'D':
			printf("it is Drac\n");
			draculaMove(gv, string);
		}
}

static void hunterMove(GameView gv, char * string, Player hunter){
	assert(strlen(string) > 3);
	int i = 1;

	//first 2 characters after name always gives us the location abbreviation
	char * location = malloc(sizeof(char *) * 2);
	strncpy(location, string + i, LOCATION_ID_SIZE);
	//find the placeID number for abbreviation and assign to hunter
	//add to the location history, update current location and index
	//hunterLocationHistoryAppend(gv, hunter, PLACEID);

	//check the next characters
	char * c;
	while ( i < strlen(string)){
		c = string[i];
		switch(*c){
			case ITS_A_TRAP:
				//its a trap!
				break;
			case CLOSE_ENCOUNTERS_OF_THE_VTH_KIND:
				//vampire encounter
				break;
			case 'D':
				//dracula
				break;
			case '.':
				//other characters include trialing '.'
				break;
		}
		c++;
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
	new->roundNumber = strlen(pastPlays) / (PLAY_S_SIZE + 1);				//each round info is 7 chars + 1 space deliminator
	new->currentPlayer = new->roundNumber % 5;									//5 players that always go in order. returns 0 - 4

	char * string = pastPlays;
	const char * delim = " ";
	//int i = 0;
	char * token = strtok(string, " ");
	while (token != NULL) {											//while not end of string
		parseMove(new, token);
		token = strtok(NULL, " ");
	}

	//fill out map???

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
	return gv->roundNumber;
}

Player GvGetPlayer(GameView gv)
{
	return gv->currentPlayer;
}

int GvGetScore(GameView gv)
{
	return gv->score;
}

int GvGetHealth(GameView gv, Player player)
{
	return gv->allPlayers[player]->health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	return gv->allPlayers[player]->currentLocation;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->vampire;
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
