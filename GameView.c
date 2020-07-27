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

//ADT for dracula statuses
struct draculaData {
	int health;
	PlaceId locationHistory[MAX_LOCATION_HISTORY_SIZE / 2]; // since drac can move at most 1 tile/round
	PlaceId currentLocation;					// current location
	int currentLocationIndex;					// index of current location in locationHistory
	bool canHide;								// can only hide when he hasnt hidden in 5 rounds, can't while at sea
	bool canDoubleback;							// can only doubleback when he hasnt hidden in 5 rounds

};

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
int comparator(const void *p, const void *q);

static void memoryError (const void * input);							//not sure if this works, but for sake of being lazy and not having to write this multiple times
static void initialiseGame (GameView gv);								//initialise an empty game to fill in
static Player parseMove (GameView gv, char *string);
//parse through that string
static void hunterMove(GameView gv, char *string, Player hunter);
//static void draculaMove(GameView gv, char * string);
//these are here for now for easy access, will move them to bottom later
static void memoryError (const void * input){
	if (input == NULL) {
		fprintf(stderr, "Couldn't allocate Memory!\n");
		exit(EXIT_FAILURE);
	}
}

int comparator(const void *p, const void *q)
{
	//Return value meaning:
	//<0 The element pointed by p goes before the element pointed by q
	//0  The element pointed by p is equivalent to the element pointed by q
	//>0 The element pointed by p goes after the element pointed by q

    // Get the values at given addresses
    char * l = (char *)p;
    char * r = (char *)q;
 	return (strcmp(l,r));
}
// appends input placeid to locationhistory, updates current location and index
/*static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location) {
	int index = gv->allPlayers[hunter]->currentLocationIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->allPlayers[hunter]->locationHistory[index + 1] = location;
		gv->allPlayers[hunter]->currentLocation = location;
	}
}
static void vampireLocationHistoryAppend(GameView gv, Player hunter, char *location); */


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
	gv->allPlayers[PLAYER_LORD_GODALMING] -> locationHistory[0] = '\0';

	gv->allPlayers[PLAYER_DR_SEWARD] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_DR_SEWARD]);
	gv->allPlayers[PLAYER_DR_SEWARD] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_DR_SEWARD] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_DR_SEWARD] -> currentLocationIndex = -1;
	gv->allPlayers[PLAYER_DR_SEWARD] -> locationHistory[0] = '\0';

	gv->allPlayers[PLAYER_VAN_HELSING] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_VAN_HELSING]);
	gv->allPlayers[PLAYER_VAN_HELSING] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_VAN_HELSING] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_VAN_HELSING] -> currentLocationIndex = -1;
	gv->allPlayers[PLAYER_VAN_HELSING] -> locationHistory[0] = '\0';

	gv->allPlayers[PLAYER_MINA_HARKER] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_MINA_HARKER]);
	gv->allPlayers[PLAYER_MINA_HARKER] -> health = MAX_HUNTER_HEALTH;
	gv->allPlayers[PLAYER_MINA_HARKER] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_MINA_HARKER] -> currentLocationIndex = -1;
	gv->allPlayers[PLAYER_MINA_HARKER] -> locationHistory[0] = '\0';

	gv->allPlayers[PLAYER_DRACULA] = malloc(sizeof(PlayerData));
	memoryError (gv->allPlayers[PLAYER_DRACULA]);
	gv->allPlayers[PLAYER_DRACULA] -> health = START_DRAC_POINT;
	gv->allPlayers[PLAYER_DRACULA] -> currentLocation = NOWHERE;
	gv->allPlayers[PLAYER_DRACULA] -> currentLocationIndex = -1;
	gv->allPlayers[PLAYER_DRACULA] -> locationHistory[0] = '\0';

	gv->trapLocations = NULL; 		//no trap locations at start of game, therefore no array yet
	gv->vampire = NOWHERE;
	gv->map = MapNew();				//do we have to do anything else with map?

	// allocate initial memory for locationHistory
}

// TARA EDITED IN A PLAYER RETURN SO YOU CAN SIMULTANEOUSLY CHECK PLAYER BASED ON STRING OF PAST PLAYS...
static Player parseMove (GameView gv, char *string){
	char *c = string;
	// THIS HEREEE...
	Player curr_player;

	//figure out who's move it was
	switch(*c){
			case 'G':
			    printf("it is Lord G\n");
			    hunterMove(gv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;
			case 'S':
			    printf("it is Dr S\n");
			    hunterMove(gv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_MINA_HARKER;
			    break;
			case 'H':
			    printf("it is VH\n");
			    hunterMove(gv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_VAN_HELSING;
			    break;
			case 'M':
			    printf("it is Mina\n");
			    hunterMove(gv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;
			case 'D':
			    printf("it is Drac\n");
			    //draculaMove(gv, string);
			    curr_player = PLAYER_LORD_GODALMING;
			    break;
			default: break;
		}

	return curr_player;
}

static void hunterMove(GameView gv, char *string, Player hunter) {

	assert (strlen(string) > LOCATION_ID_SIZE);

	char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
	city[0] = string[1];
	city[1] = string[2];
	city[2] = '\0';

	PlaceId curr_place = NOWHERE;

	for (int i = 0; i < NUM_REAL_PLACES; i++) {
	    Place row = PLACES[i];
	    if (strcmp(row.abbrev, city) == 0) {
	        curr_place = row.id;
	        break;
	    }
	}

	gv->allPlayers[hunter]->currentLocation = curr_place;
	gv->allPlayers[hunter]->currentLocationIndex ++;
	// If this is the first move.
	if (gv->allPlayers[hunter]->locationHistory[0] == '\0') {
	    gv->allPlayers[hunter]->locationHistory[0] = curr_place;
	} else {
	    printf("yet to do..\n");
	    // Basically shuffle the array back :)
	}


	/*
	assert(strlen(string) > 3);
	int i = 1;

	//first 2 characters after name always gives us the location abbreviation
	char *location = malloc(sizeof(char *) * 2);
	strncpy(location, string + i, LOCATION_ID_SIZE);
	//find the placeID number for abbreviation and assign to hunter
	//add to the location history, update current location and index
	//hunterLocationHistoryAppend(gv, hunter, PLACEID);

	//check the next characters
	char *c;
	while ( i < strlen(string)){
		c = string;
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
		i++;
	}*/

    return;

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

	char *string = pastPlays;
	//const char * delim = " ";
	//int i = 0;
	char *token = strtok(string, " ");
	while (token != NULL) {											//while not end of string
		new->currentPlayer = parseMove(new, token);
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

// Tara was here :)
PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION

	// 1. We need to access the map :)
	Map map = gv->map;

	// 2. Calculate the number of rails a hunter can travel.
	int railDist = 0;
	if (player != PLAYER_DRACULA) railDist = (round + player) % 4;

	// 3. Create temp array and store the locations within 1 path.
	PlaceId temp_loc[NUM_REAL_PLACES];
	for (int i = 0; i < NUM_REAL_PLACES; i++) {
	    temp_loc[i] = '\0';
	}

	// Get the connections from that point.
	ConnList list = MapGetConnections(map, from);

	// Store viable connections in temp.
	int loc = 0;
	temp_loc[loc] = from;
	loc = 1;

	while (loc < NUM_REAL_PLACES && list != NULL) {

	    // Extra conditions for drac:
	    if (player == PLAYER_DRACULA) {
	        if (list->p == HOSPITAL_PLACE) continue;
	        if (list->type == RAIL) continue;
	    }

	    // If it is a road type.
	    if (list->type == ROAD) {
	        temp_loc[loc] = list->p;
	        // If it is a rail type check for hunter.
	    } else if (list->type == RAIL && railDist >= 1) {
	        temp_loc[loc] = list->p;
	        // If it is a boat type.
	    } else if (list->type == BOAT) {
	        temp_loc[loc] = list->p;
	    }
	    loc++;
	    list = list->next;
	}
	int array_len = sizeof(temp_loc) / sizeof(temp_loc[0]);
	qsort(temp_loc,array_len, sizeof(temp_loc[0]), comparator);
	// 4. Now alphabeticalizeee:
	int i = 0;
	while (i < NUM_REAL_PLACES && temp_loc[i] != '\0') {
	    int j = i + 1;
	    while (j < NUM_REAL_PLACES && temp_loc[j] != '\0') {
	        if (temp_loc[j] < temp_loc[i]) {
	            int change = temp_loc[i];
	            temp_loc[i] = temp_loc[j];
	            temp_loc[j] = change;
	        }
	        j++;
	    }
	    i++;
	}

	// 5. Now copy into the dynamically allocated array.
	int size = i - 1;
	PlaceId *final_loc_list = malloc(size*sizeof(PlaceId));
	i = 0;
	while (i < size) {
	    final_loc_list[i] = temp_loc[i];
	    i++;
	}

	*numReturnedLocs = size;
	return final_loc_list;
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
