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

#define PLAY_S_SIZE 7
//#define MAX_GAME_SCORE 366             GAME_START_SCORE is #defined in game.h
//#define MAX_HUNTER_HEALTH 9						GAME_START_HUNTER_LIFE_POINTS  is #defined in game.h
//#define START_DRAC_POINT 40						GAME_START_BLOOD_POINTS is #defined in game.h
#define LOCATION_ID_SIZE 2
#define MAX_LOCATION_HISTORY_SIZE (GAME_START_SCORE * 2 * 4)
//DOUBLE CHECK THIS: max number of turns * most possible avg moves per turn (rounded up)  * bytes per move stored

//some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'

//#defines to make things more readable?
//C: I like this, makes things a bit more readable
#define  LORD_GODALMING gv->allPlayers[PLAYER_LORD_GODALMING]
#define  DR_SEWARD gv->allPlayers[PLAYER_DR_SEWARD]
#define  VAN_HELSING  gv->allPlayers[PLAYER_VAN_HELSING]
#define  MINA_HARKER gv->allPlayers[PLAYER_MINA_HARKER]
#define  DRACULA gv->allPlayers[PLAYER_DRACULA]
#define  PLAYER gv->allPlayers[PLAYER]

typedef struct playerData *PlayerData;
typedef struct vampireData *IVampire;


//ADT for player statuses
struct playerData {
	int health;
	PlaceId locationHistory[MAX_LOCATION_HISTORY_SIZE];
	PlaceId currentLocation;					// current location
	int currentLocationIndex;					// index of current location in locationHistory
	// BLOOD BOIS ONLY BEYOND THIS POINT
	int lastHidden;								// round in which drac last hid
	int lastDoubleback;							// round in which drac last doubled back
	// bool isVisible							// something to indicate whether hunters know drac curr location
};

struct gameView {
	Round roundNumber;
	int score;
	Player currentPlayer;						// looks like G always starts first? judging by the testfiles given G->S->H->M->D
	PlayerData allPlayers[NUM_PLAYERS];
	PlaceId *trapLocations;
	PlaceId vampire;							//only one vampire alive at any time
	Map map; 									//graph thats been typedefed already

};


// private functions
static PlaceId binarySearchPlaceId ( int l, int r, char * string);				//Iterative binsary search for PLACES[]
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

// appends input placeid to locationhistory, updates current location and index
static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location) {
	int index = gv->allPlayers[hunter]->currentLocationIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->allPlayers[hunter]->locationHistory[index + 1] = location;
		gv->allPlayers[hunter]->currentLocation = location;
	}
}

static void vampireLocationHistoryAppend(GameView gv, char *location) {
	int index = gv->allPlayers[PLAYER_DRACULA]->currentLocationIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->allPlayers[PLAYER_DRACULA]->locationHistory[index + 1] = location;
		gv->allPlayers[PLAYER_DRACULA]->currentLocation = location;
	}

static PlaceId binarySearchPlaceId ( int l, int r, char * city){
	Place row;
	while ( l <= r){
		int m = 1 + (r-1) /2;
		row = PLACES[m];
		//Check if x is preset at mid
		if (strcmp(row.abbrev, city) == 0) return row.id;
		//If x is greater, ignore left half
		if (strcmp(row.abbrev, city) < 0) {
			l = m + 1;
		//If x is smaller, ignore right half
		} else {
			r = m - 1;
		}
	}
    //City not found!
	return NOWHERE;
}

static void initialiseGame (GameView gv) {
	gv->roundNumber = 0;
	gv->score = GAME_START_SCORE;
	gv->currentPlayer = PLAYER_LORD_GODALMING; 		//always starts with G

	//Allocate memory for players & Initialise starting information
	LORD_GODALMING = malloc(sizeof(PlayerData));
	memoryError (LORD_GODALMING);
	LORD_GODALMING -> health = 	GAME_START_HUNTER_LIFE_POINTS;
	LORD_GODALMING -> currentLocation = NOWHERE;
	LORD_GODALMING -> currentLocationIndex = -1;
	LORD_GODALMING -> locationHistory[0] = '\0';

	DR_SEWARD = malloc(sizeof(PlayerData));
	memoryError (DR_SEWARD);
	DR_SEWARD -> health = 	GAME_START_HUNTER_LIFE_POINTS;
	DR_SEWARD -> currentLocation = NOWHERE;
	DR_SEWARD -> currentLocationIndex = -1;
	DR_SEWARD -> locationHistory[0] = '\0';

	VAN_HELSING = malloc(sizeof(PlayerData));
	memoryError (VAN_HELSING);
	VAN_HELSING -> health = 	GAME_START_HUNTER_LIFE_POINTS;
	VAN_HELSING -> currentLocation = NOWHERE;
	VAN_HELSING -> currentLocationIndex = -1;
	VAN_HELSING -> locationHistory[0] = '\0';

	MINA_HARKER = malloc(sizeof(PlayerData));
	memoryError (MINA_HARKER);
	MINA_HARKER -> health = 	GAME_START_HUNTER_LIFE_POINTS;
	MINA_HARKER -> currentLocation = NOWHERE;
	MINA_HARKER -> currentLocationIndex = -1;
	MINA_HARKER -> locationHistory[0] = '\0';

	DRACULA = malloc(sizeof(PlayerData));
	memoryError (DRACULA);
	DRACULA -> health = GAME_START_BLOOD_POINTS;
	DRACULA -> currentLocation = NOWHERE;
	DRACULA -> currentLocationIndex = -1;
	DRACULA -> locationHistory[0] = '\0';

	gv->trapLocations = NULL; 		//no trap locations at start of game, therefore no array yet -> will need to assign
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
	//string must be of valid size
	assert (strlen(string) > LOCATION_ID_SIZE);

	//store locationID into city[]
	char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
	city[0] = string[1];
	city[1] = string[2];
	city[2] = '\0';

	PlaceId curr_place = NOWHERE;


	curr_place = binarySearchPlaceId ( 0, NUM_REAL_PLACES, city);
    if (curr_place == NOWHERE) {
        fprintf(stderr, "City not found!\n");
		exit(EXIT_FAILURE);
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

	//Parsing through characters after location iD

	//check the next characters

	char *c;
	for ( int i = 3; i < strlen(string); i++) {
		c = string;
		switch(*c){
			case ITS_A_TRAP:
            //i++;
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
	//Allocate memory for new GV
	GameView new = malloc(sizeof(*new));
	//Check if memory was allocated correctly
	memoryError(new);
	initialiseGame (new);

	new->roundNumber = strlen(pastPlays) / (PLAY_S_SIZE + 1);				//Number of Rounds can be determined from size of string ( each play is 7 chars + space)
	new->currentPlayer = new->roundNumber % 5;									//5 players that always go in order of G->S->H->M->D. returns 0 - 4

	char *string = pastPlays;
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
	if (player == PLAYER_DRACULA) {
		// todo if has been revealed
		return gv->allPlayers[player]->currentLocation;
		// else return SEA/CITY
		if (placeIdToType(gv->allPlayers[player]->currentLocation) == SEA)
			return SEA_UNKNOWN;
		else
			return CITY_UNKNOWN;
	}
	else
		return gv->allPlayers[player]->currentLocation;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	return gv->vampire;
}


/// INCOMPLETE
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
	// can't free as is returning directly from data struct
	*canFree = false;
	// pass number of moves
	*numReturnedMoves = gv->allPlayers[player]->currentLocationIndex;
	return gv->allPlayers[player]->locationHistory;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	
	// unless asking for more locations than have happened, return numlocs
	if (gv->allPlayers[player]->currentLocationIndex >= numMoves) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedMoves = numMoves;
		// allocate space for return array
		PlaceId *lastNMoves = malloc(sizeof(PlaceId) *numMoves);
		// TODO most recent move first or last????
		for (int i = 0; i < numMoves; i++) {
			lastNMoves[i] = gv->allPlayers[player]->locationHistory[gv->allPlayers[player]->currentLocationIndex - numMoves + i];
		}
	}
	// if asking for too many locations, only return all that exist
	else
		return GvGetLocationHistory(gv, player, *numReturnedMoves, *canFree);
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	
	// can't free as is returning directly from data struct
	*canFree = false;
	// pass number of moves
	*numReturnedLocs = gv->allPlayers[player]->currentLocationIndex;
	// dracula case should be handled by this
	return GvGetPlayerLocation(gv, player);
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	
	// unless asking for more locations than have happened, return numlocs
	if (gv->allPlayers[player]->currentLocationIndex >= numLocs) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedLocs = numLocs;
		// allocate space for return array
		PlaceId *lastNLocs = malloc(sizeof(PlaceId) *numLocs);
		// loop throught location history, adding to lastn in chronological order
		for (int i = 0; i < numLocs; i++) {
			lastNLocs[i] = gv->allPlayers[player]->locationHistory[gv->allPlayers[player]->currentLocationIndex - numLocs + i];
		}
	}
	// if asking for too many locations, only return all that exist
	else
		return GvGetLocationHistory(gv, player, *numReturnedLocs, *canFree);
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
	        loc++;
	        // If it is a rail type check for hunter.
	    } else if (list->type == RAIL && railDist >= 1) {
	        temp_loc[loc] = list->p;
	        loc++;
	        // If it is a boat type.
	    } else if (list->type == BOAT) {
	        temp_loc[loc] = list->p;
	        loc++;
	    }

	    list = list->next;
	}

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
