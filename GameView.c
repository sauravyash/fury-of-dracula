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
#define OUR_ARBITRARY_ARRAY_SIZE 10

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
	int trapLocationsIndex;
	PlaceId vampire;							//only one vampire alive at any time
	Map map; 									//graph thats been typedefed already

};


// Private function declarations:

//------------- MAKING A MOVE
// Helper function for reachables:
static int Find_Rails (Map map, PlaceId place, PlaceId from, PlaceId *array, int i);

//------------- GENERAL FUNCTIONS
// Qsort comparator:
int comparator(const void *p, const void *q);
// Memory error test:
// not sure if this works, but for sake of being lazy and not having to write
// this multiple times
static void memoryError (const void * input);

//------------- CONSTRUCTOR/ DESTRUCTOR
// Initialise an empty game to fill in:
static void initialiseGame (GameView gv);
// Parse through that string
static Player parseMove (GameView gv, char *string);
static void hunterMove(GameView gv, char *string, Player hunter);
static void draculaMove(GameView gv, char * string);




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
// COMMENTED OUT FOR NOW TO AVOID NOT USED WARNING
// appends input placeid to locationhistory, updates current location and index
static void trapLocationAppend(GameView gv, PlaceId location) {
	int index = gv->trapLocationsIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->trapLocations[index + 1] = location;
		gv->trapLocationsIndex++;
	}
	return;
}

static void trapLocationRemove(GameView gv, PlaceId location) {
	//find index of trap location
	//remove from location
	//shuffle array
	return;
}
// appends input placeid to locationhistory, updates current location and index
static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location) {
	int index = gv->allPlayers[hunter]->currentLocationIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->allPlayers[hunter]->locationHistory[index + 1] = location;
		gv->allPlayers[hunter]->currentLocation = location;
		gv->allPlayers[hunter]->currentLocationIndex++;
	}
	return;
}
//Only for revealed moves (doesnt include C?, S?, Hi, Dn, Tp)
static void vampireLocationHistoryAppend(GameView gv, PlaceId location) {
	int index = gv->allPlayers[PLAYER_DRACULA]->currentLocationIndex;
	if (index < MAX_LOCATION_HISTORY_SIZE) {
		gv->allPlayers[PLAYER_DRACULA]->locationHistory[index + 1] = location;
		gv->allPlayers[PLAYER_DRACULA]->currentLocation = location;
		gv->allPlayers[PLAYER_DRACULA]->currentLocationIndex++;
	}
}


static void initialiseGame (GameView gv) {

	gv->roundNumber = 0;
	gv->score = GAME_START_SCORE;
	//Always starts with G
	gv->currentPlayer = PLAYER_LORD_GODALMING;

	// Allocate memory for players & Initialise starting information
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

    // No trap locations at start of game, therefore no array yet..
	gv->trapLocations = NULL;
	gv->vampire = NOWHERE;

	// Nothing else to do for map- read Map.c --> the functions take care of
	// adding all connections.
	gv->map = MapNew();

}

// PARSE MOVE:
// -- Input:
// -- Output:
// Author: Cindy (Tara edited)
static Player parseMove (GameView gv, char *string){

	char *c = string;
	Player curr_player = false;

	//figure out who's move it was
	switch(*c){
			case 'G':
			    //printf("it is Lord G\n");
			    hunterMove(gv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;
			case 'S':
			    //printf("it is Dr S\n");
			    hunterMove(gv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_VAN_HELSING;
			    break;
			case 'H':
			    //printf("it is VH\n");
			    hunterMove(gv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_MINA_HARKER;
			    break;
			case 'M':
			    //printf("it is Mina\n");
			    hunterMove(gv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;
			case 'D':
			    //printf("it is Drac\n");
			    draculaMove(gv, string);
			    curr_player = PLAYER_LORD_GODALMING;
			    break;
		}

	return curr_player;
}

// HUNTER MOVE:
// -- Input:
// -- Output:
// Author: Cindy (Tara edited)
static void hunterMove(GameView gv, char *string, Player hunter) {

	// String must be of valid size
	assert (strlen(string) > LOCATION_ID_SIZE);

	// Store locationID into city[]:
	char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
	city[0] = string[1];
	city[1] = string[2];
	city[2] = '\0';

    // Compare and find city by abbreviation:
	PlaceId curr_place = NOWHERE;
	//Using provided function from Place.h
	curr_place =  placeAbbrevToId(city);
	// This find works (until you can get the binary working..?
	/*for (int i = 0; i < NUM_REAL_PLACES; i++) {
 	    Place row = PLACES[i];
 	    if (strcmp(row.abbrev, city) == 0) {
 	        curr_place = row.id;
 	        break;
 	    }
 	}
	*/
    // Append history and current location:
    hunterLocationHistoryAppend(gv, hunter, curr_place);

	// Parsing through characters after location iD
	// check the next characters
	char *c;
	for (int i = 3; i < strlen(string); i++) {
		c = &string[i];
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
	}

    return;
}

// DRACULA MOVE:
// -- Input:
// -- Output:
// Author:
static void draculaMove(GameView gv, char *string) {

	// String must be of valid size
	assert (strlen(string) > LOCATION_ID_SIZE);

	// Store locationID into city[]:
	char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
	city[0] = string[1];
	city[1] = string[2];
	city[2] = '\0';

    // EDIT THIS FOR DRAC FOR CITY UNKNOWN ETC..
    // Compare and find city by abbreviation:
	PlaceId curr_place = NOWHERE;
	//Using provided function from Place.h
	curr_place =  placeAbbrevToId(city);
	// This find works (until you can get the binary working..?
	/*for (int i = 0; i < NUM_REAL_PLACES; i++) {
 	    Place row = PLACES[i];
 	    if (strcmp(row.abbrev, city) == 0) {
 	        curr_place = row.id;
 	        break;
 	    }
 	}
	*/

    // Append history and current location:
	//Unknown city move
	if (strcmp(city, "C?") == 0) {
		printf("unknown city move\n");
	//Unknown sea move
	} else if (strcmp(city,"S?") == 0) {
		printf("unknown sea move\n");
	//Hide move ->stays in the city for another round
	} else if (strcmp(city,"HI") == 0) {
		printf("hide move\n");
		DRACULA->lastHidden = gv->roundNumber;
		vampireLocationHistoryAppend(gv, curr_place);
	//Double back move
} else if (strncmp(city,"D",1) == 0) {
		//ascii 0 is 48
		int doubleBack = atoi(&city[1]);
		DRACULA->lastDoubleback = gv->roundNumber;
		doubleBack++;								//just to make compile ignore unused variable
		//convert this to the #define DOUBLE_BACK_1 etc?

	//TPs to castle dracula
	} else if (strcmp(city,"TP") == 0) {
		vampireLocationHistoryAppend(gv, TELEPORT);
	//Location move that was revealed (ie all other cases)
	} else {
    	vampireLocationHistoryAppend(gv, curr_place);
	}

	// Parsing through characters after location iD
	// check the next characters
	char *c;
	int i = 3;

	while ( i < strlen(string)) {
		c = &string[i];
		if ( i > 4) {
			if (strcmp(c,"M") == 0) {
				printf("Trap has left trail!\n");
				int * numReturnedLocs = NULL;
				PlaceId * trail = GvGetLastLocations(gv, PLAYER_DR_SEWARD , TRAIL_SIZE,
				                            numReturnedLocs, false);
				PlaceId brokenTrap = trail[0];
				trapLocationRemove(gv, brokenTrap);
				//remove from trapLocations
				//trap leaves trail (from the move that left trail?)
			} else if (strcmp(c,"V") == 0) {
				//vampire matures
				printf("Vampire matured! -%d game points\n", SCORE_LOSS_VAMPIRE_MATURES);
				gv->vampire = NOWHERE;
				gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
			}
		} else {
			if (*c == ITS_A_TRAP) trapLocationAppend(gv, curr_place);
			if (*c == CLOSE_ENCOUNTERS_OF_THE_VTH_KIND) gv->vampire = curr_place;
		}
		i++;
	}
	//gamescore decreases by 1 when dracula finishes his turns
	gv->score--;
    return;

}
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[])
{
	// Allocate memory for new GV
	GameView new = malloc(sizeof(*new));

	// Check if memory was allocated correctly
	memoryError(new);
	initialiseGame (new);

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
	// todo visibility of dracula depends on draculamove implementation
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
		// copy desired values from locationHistory to return array
		for (int i = 0; i < numMoves; i++) {
			lastNMoves[i] = gv->allPlayers[player]->locationHistory[gv->allPlayers[player]->currentLocationIndex - numMoves + i];
		}
		return lastNMoves;
	}

	// if asking for too many locations, only return all that exist
	return GvGetLocationHistory(gv, player, numReturnedMoves, canFree);

}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// can free as allocating new array
	*canFree = true;

	// pass number of moves
	int index = gv->allPlayers[player]->currentLocationIndex;
	*numReturnedLocs = index;
	PlaceId *allLocs = malloc(sizeof(PlaceId) * index);

	if (player == PLAYER_DRACULA) {
		// todo make sure rounds when drac not visible returned as unknowns
		// depends on how visibility is parsed in dracmove

		// add locs to all locs, checking if drac visible, and if not adding as unknown
		for (int i = 0; i <= index; i++) {
			// if visible
			allLocs[i] = gv->allPlayers[player]->locationHistory[i];
			/* else if not visible
				// return unknown sea
				if (placeIdToType(gv->allPlayers[player]->currentLocation) == SEA)
					return SEA_UNKNOWN;
				// or return unknown city
				else
					return CITY_UNKNOWN;*/
		}
		return allLocs;
	}
	return GvGetMoveHistory(gv, player, numReturnedLocs, canFree);
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
		return lastNLocs;
	}
	// if asking for too many locations, only return all that exist
	return GvGetLocationHistory(gv, player, numReturnedLocs, canFree);
}

////////////////////////////////////////////////////////////////////////
// Making a Move

// REACHABLE FUNCTIONS: Help to find all possible moves for the next play based
// on round score, player, starting place, and other game deets.
// Author: Tara
// Status: Fully functioning for hunter- passed all given tests plus own tests
// and works with any railDist, player and round. Need to test Drac...
PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{

	// 1. We need to access the map :)
	Map map = gv->map;

	// 2. Calculate the number of rails a hunter can travel.
	int railDist = 0;
	if (player != PLAYER_DRACULA) railDist = (round + player) % 4;

	// Create temp array to keep track of  locations visited in this function.
	PlaceId visited[NUM_REAL_PLACES];
	PlaceId *visited_rail = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	for (int j = 0; j < NUM_REAL_PLACES; j++) {
	    visited_rail[j] = '\0';
	    visited[j] = '\0';
	}

	// Get the connections from that point.
	ConnList list = MapGetConnections(map, from);

	// 3. Iterate through...
	int loc_num = 0;
	int rail_num = 0;
	visited[loc_num] = from;
	loc_num = 1;

	while (loc_num < NUM_REAL_PLACES && rail_num < NUM_REAL_PLACES) {

	    // Extra conditions for drac:
	    if (player == PLAYER_DRACULA) {
	        if (list->p == HOSPITAL_PLACE) continue;
	        if (list->type == RAIL) continue;
	    }

	    // If it is a road type.
	    if (list->type == ROAD) {
	        visited[loc_num] = list->p;
	        loc_num++;
	        // If it is a rail type check for hunter.
	    } else if (list->type == RAIL && railDist >= 1) {
	        visited_rail[rail_num] = list->p;
	        rail_num++;
	        // If it is a boat type.
	    } else if (list->type == BOAT) {
	        visited[loc_num] = list->p;
	        loc_num++;
	    }

	    if (list->next == NULL) break;
	    list = list->next;
	}

	// Consider more rails...
	// 2 Rails:
	int i = 0;
	int conn_new = 0;
	if (railDist > 1) {
	    while (i < rail_num) {
	        int index = rail_num + conn_new;
	        conn_new = conn_new + Find_Rails (map, visited_rail[i], from, visited_rail, index);
	        i++;
	    }
	    rail_num = rail_num + conn_new;

	    // 3 Rails:
	    i = 0;
	    if (railDist > 2) {
	    i = rail_num - conn_new;
	    conn_new = 0;
	        while (i < rail_num) {
	            int index = rail_num + conn_new;
	            conn_new = conn_new + Find_Rails (map, visited_rail[i], from, visited_rail, index);
	            i++;
	        }
	        rail_num = rail_num + conn_new;
	    }
	}

	// 4. Combine arrays!!
	// So we know the number of locs/ rails in each array;
	i = 0;
	int j = loc_num;
	while (i < rail_num && j < NUM_REAL_PLACES) {

	    visited[j] = visited_rail[i];
	    j++;
	    i++;

	}
	int total_locs = j;

	// 5. Now copy into the dynamically allocated array.
	PlaceId *final_loc_list = malloc(total_locs * sizeof(PlaceId));
	i = 0;
	while (i < total_locs) {
	    final_loc_list[i] = visited[i];
	    i++;
	}

    // Return values...
	*numReturnedLocs = total_locs;
	return final_loc_list;

}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{

	// 1. We need to access the map :)
	Map map = gv->map;

	// 2. Calculate the number of rails a hunter can travel.
	int railDist = 0;
	if (player != PLAYER_DRACULA) railDist = (round + player) % 4;

	// Create temp array to keep track of  locations visited in this function.
	PlaceId visited[NUM_REAL_PLACES];
	PlaceId *visited_rail = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	for (int j = 0; j < NUM_REAL_PLACES; j++) {
	    visited_rail[j] = '\0';
	    visited[j] = '\0';
	}

	// Get the connections from that point.
	ConnList list = MapGetConnections(map, from);

	// 3. Iterate through...
	int loc_num = 0;
	int rail_num = 0;
	visited[loc_num] = from;
	loc_num = 1;

	while (loc_num < NUM_REAL_PLACES && rail_num < NUM_REAL_PLACES) {

	    // Extra conditions for drac:
	    if (player == PLAYER_DRACULA) {
	        if (list->p == HOSPITAL_PLACE) continue;
	        if (list->type == RAIL) continue;
	    }

	    // If it is a road type.
	    if (list->type == ROAD && road == true) {
	        visited[loc_num] = list->p;
	        loc_num++;
	        // If it is a rail type check for hunter.
	    } else if (list->type == RAIL && railDist >= 1 && rail == true) {
	        visited_rail[rail_num] = list->p;
	        rail_num++;
	        // If it is a boat type.
	    } else if (list->type == BOAT && boat == true) {
	        visited[loc_num] = list->p;
	        loc_num++;
	    }

	    if (list->next == NULL) break;
	    list = list->next;
	}

	// Consider more rails...
	// 2 Rails:
	int i = 0;
	int conn_new = 0;
	if (railDist > 1) {
	    while (i < rail_num) {
	        int index = rail_num + conn_new;
	        conn_new = conn_new + Find_Rails (map, visited_rail[i], from, visited_rail, index);
	        i++;
	    }
	    rail_num = rail_num + conn_new;

	    // 3 Rails:
	    i = 0;
	    if (railDist > 2) {
	    i = rail_num - conn_new;
	    conn_new = 0;
	        while (i < rail_num) {
	            int index = rail_num + conn_new;
	            conn_new = conn_new + Find_Rails (map, visited_rail[i], from, visited_rail, index);
	            i++;
	        }
	        rail_num = rail_num + conn_new;
	    }
	}

	// 4. Combine arrays!!
	// So we know the number of locs/ rails in each array;
	i = 0;
	int j = loc_num;
	while (i < rail_num && j < NUM_REAL_PLACES) {

	    visited[j] = visited_rail[i];
	    j++;
	    i++;

	}
	int total_locs = j;

	// 5. Now copy into the dynamically allocated array.
	PlaceId *final_loc_list = malloc(total_locs * sizeof(PlaceId));
	i = 0;
	while (i < total_locs) {
	    final_loc_list[i] = visited[i];
	    i++;
	}

    // Return values...
	*numReturnedLocs = total_locs;
	return final_loc_list;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// FIND EXTRA RAIL LOCS: Helper function for reachables to find rail CONNECTIONS.
// -- INPUT: Map, starting place, original place to check as duplicate, array of
// visited rail locations to check for duplicates and add to, and index (since we
// are editing a used array).
// -- OUTPUT: An updated array of visited rail locations and number of locations
// added in this instance.
// -- Author: Tara
static int Find_Rails (Map map, PlaceId place, PlaceId from, PlaceId *array, int i) {

    // Check things exist:
    if (place == '\0') return 0;
    if (array[0] == '\0') return 0;

    // Find list of connections to current place.
    ConnList list = MapGetConnections(map, place);
    int places_added = 0;

    // Iterate through to find rail types...
    while (i < NUM_REAL_PLACES) {

        int skip = 0;
        // Check for repeats;
        if (list->p == from) {
            skip = 1;
        }

        int repeat = 0;
        for (int j = 0; array[j] != '\0'; j++) {
            if (array[j] == list->p) {
                repeat++;
            }
        }
        if (repeat > 0) skip = 1;

        // Now, add to array.
        if (list->type == RAIL && skip == 0) {
            printf("CASE\n");
            array[i] = list->p;
            places_added++;
        }

        // Increment.
        if (list->next == NULL) break;
        list = list->next;
    }

    return places_added;
}
