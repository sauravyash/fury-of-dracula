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
#define LOCATION_ID_SIZE 2
#define MAX_LOC_HISTORY_SIZE (GAME_START_SCORE * 2 * 4 )
//DOUBLE CHECK THIS: max number of turns * most possible avg moves per turn (rounded up)  * bytes per move stored
#define OUR_ARBITRARY_ARRAY_SIZE 10

// some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'
#define MALFUNCTIONING_MACHIAVELLIAN_MACHINATIONS 'M'

// defines to make things more readable
#define  LORD_GODALMING gv->allPlayers[PLAYER_LORD_GODALMING]
#define  DR_SEWARD 		gv->allPlayers[PLAYER_DR_SEWARD]
#define  VAN_HELSING  	gv->allPlayers[PLAYER_VAN_HELSING]
#define  MINA_HARKER 	gv->allPlayers[PLAYER_MINA_HARKER]
#define  DRACULA 		gv->allPlayers[PLAYER_DRACULA]
#define  PLAYER 		gv->allPlayers[player]
#define  HUNTER			gv->allPlayers[hunter]

// typedef data structs
typedef struct playerData *PlayerData;
typedef struct vampireData *IVampire;


// ADT for player statuses
struct playerData {
	int health;											// current player health
	PlaceId locationHistory[MAX_LOC_HISTORY_SIZE];		// array of locations visited by player
	PlaceId moveHistory[MAX_LOC_HISTORY_SIZE];			//array of moves made by player (different to location history for drac)
	PlaceId currentLocation;						  	// current location
	int currentLocationIndex;						  	// index of current location in locationHistory
	// -------------------------- BLOOD BOIS ONLY BEYOND THIS POINT --------------------------
	int lastHidden;									  	// round in which drac last hid
	int lastDoubleback;									// round in which drac last doubled back
};

struct gameView {
	Round roundNumber;									// current round
	int score;											// current game score
	Player currentPlayer;								// looks like G always starts first? judging by the testfiles given G->S->H->M->D
	PlayerData allPlayers[NUM_PLAYERS];					// array of playerData structs
	PlaceId trapLocations[MAX_LOC_HISTORY_SIZE];		// array of trap locations -- multiple traps in same place added seperately
	int trapLocationsIndex;								// index of the most recently added trap location
	PlaceId vampire;									// only one vampire alive at any time
	Map map; 											// graph thats been typedefed already
};


// PRIVATE FUNCTION DECLARATIONS
// ****************************************

//------------- MAKING A MOVE -------------
// Helper function for reachables:
static int Find_Rails (Map map, PlaceId place, PlaceId from, PlaceId *array, int i);
//------------- GENERAL FUNCTIONS -------------
static int placeIdCmp(const void *ptr1, const void *ptr2);			//Qsort comparator function
static void memoryError (const void * input);						//Checks if memory was allocated properly
static void sortPlaces(PlaceId *places, int numPlaces);				// Sorts an array of PlaceIds
static int PlaceIdToAsciiDoubleBack (PlaceId place);				// Convert a doubleback placeid to doubleback value
static void checkHunterHealth(GameView gv,Player hunter);			// Check the health of a hunter, sends them to hospital if needed
//------------- Parsing Player Moves -------------
static Player parseMove (GameView gv, char *string);				// Parse the move string
static void hunterMove(GameView gv, char *string, Player hunter);	// Apply hunter move from parsed string
static void draculaMove(GameView gv, char * string);				// Apply dracula move from parsed string
//------------- CONSTRUCTOR/ DESTRUCTOR -------------
static void initialiseGame (GameView gv);							// Initialise an empty game to fill in
static PlayerData initialisePlayer(GameView gv, Player player);		// Initialise PlayerData ADT
//------------- Location & Move Histories -------------
static void trapLocationAppend(GameView gv, PlaceId location);		// Add a trap location
static void trapLocationRemove(GameView gv, PlaceId location);		// Remove a trap location
static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location);
static void draculaLocationHistoryAppend(GameView gv, PlaceId location);

// ****************************************

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor


// GV NEW: Allocate memory for new GameView
// -- INPUT: pastPlays string, messages
// -- OUTPUT: new GameView
GameView GvNew(char *pastPlays, Message messages[]) {
	// Allocate memory for new GV
	GameView new = malloc(sizeof(* new));
	memoryError(new);
	new->map = MapNew();
	initialiseGame (new);

    // Number of Rounds can be determined from size of string
    // (each play is 7 chars + space)
    // Note last move has no space...
    // Note each round is 5 plays...
	new->roundNumber = (strlen(pastPlays) + 1) / ((PLAY_S_SIZE + 1)*5);

    // Need to create a copy of pastPlays rather than just pointing to it.
	char string[strlen(pastPlays)];
	strcpy(string, pastPlays);

	// Iterate through past plays...
	char *token = strtok(string, " ");
	while (token != NULL) {
		new->currentPlayer = parseMove(new, token);
		token = strtok(NULL, " ");
	}
	return new;
}

// GV FREE: Frees the current GameView
void GvFree(GameView gv) {
	// free player structs
	for (int i = 0; i < NUM_PLAYERS; i++)
		free(gv->allPlayers[i]);
	// free map
	MapFree(gv->map);
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv) {
	return gv->roundNumber;
}

Player GvGetPlayer(GameView gv) {
	return gv->currentPlayer;
}

int GvGetScore(GameView gv) {
	return gv->score;
}

int GvGetHealth(GameView gv, Player player) {
	return gv->allPlayers[player]->health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player) {
	return gv->allPlayers[player]->currentLocation;
}

PlaceId GvGetVampireLocation(GameView gv) {
	return gv->vampire;
}


PlaceId *GvGetTrapLocations(GameView gv, int *numTraps){
	// pass number of moves; index + 1 since index starts at 0
	*numTraps = gv->trapLocationsIndex + 1;
	PlaceId *traps = malloc(sizeof(PlaceId) * *numTraps);
	memoryError(traps);
	for (int i = 0; i < *numTraps; i++){
		traps[i] = gv->trapLocations[i];
	}
	return traps;
}

////////////////////////////////////////////////////////////////////////
// Game History

// GET MOVE HISTORY: Returns complete move history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed
// -- INPUT: gv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree) {

	// can't free as is returning directly from data struct
	*canFree = false;
	// pass number of moves
	*numReturnedMoves = gv->allPlayers[player]->currentLocationIndex+1;
	return gv->allPlayers[player]->moveHistory;
}

// GET MOVE HISTORY: Returns last n moves of a player in dynamically allocated
// array, indicates the number of moves and that the return array can be freed
// -- INPUT: gv, player, number of moves to return, pointer to an int storing
// the number of returned moves, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree) {

	// unless asking for more locations than have happened, return numlocs
	if (gv->allPlayers[player]->currentLocationIndex >= numMoves) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedMoves = numMoves;
		// allocate space for return array
		PlaceId *lastNMoves = malloc(sizeof(PlaceId) *numMoves);
		memoryError(lastNMoves);
		// copy desired values from locationHistory to return array
		for (int i = 0; i < numMoves; i++) {
			lastNMoves[i] = gv->allPlayers[player]->moveHistory[gv->allPlayers[player]->currentLocationIndex - numMoves + i];
		}
		return lastNMoves;
	}
	// if asking for too many locations, only return all that exist
	return GvGetLocationHistory(gv, player, numReturnedMoves, canFree);
}

// GET MOVE HISTORY: Returns complete location history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed. Returns UNKNOWN places where relevant for dracula.
// -- INPUT: gv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree) {

	// pass number of moves
	int index = gv->allPlayers[player]->currentLocationIndex;
	*numReturnedLocs = index + 1;
	//if there are no moves in history, return NULL
	if(index < 0) return NULL;
	// can free as allocating new array
	*canFree = true;
	PlaceId *allLocs = malloc(sizeof(PlaceId) * *numReturnedLocs);
	memoryError(allLocs);
	if (player == PLAYER_DRACULA) {
		for (int i = 0; i < *numReturnedLocs; i++) {
			allLocs[i] = gv->allPlayers[player]->locationHistory[i];
		}
		return allLocs;
	}
	free(allLocs);
	//location history == move history for hunters
	return GvGetMoveHistory(gv, player, numReturnedLocs, canFree);
}

// GET MOVE HISTORY: Returns last n locations of a player in dynamically allocated
// array, indicates the number of locs and that the return array can be freed
// -- INPUT: gv, player, number of locs to return, pointer to an int storing
// the number of returned locs, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree) {

	// unless asking for more locations than have happened, return numlocs
	if (gv->allPlayers[player]->currentLocationIndex >= numLocs) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedLocs = numLocs;
		// allocate space for return array
		PlaceId *lastNLocs = malloc(sizeof(PlaceId) *numLocs);
		memoryError(lastNLocs);
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
// Status: Iterative Fully functioning for hunter- passed all given tests plus own tests
// and works with any railDist, player and round. Need to test Drac...
// TODO: Recursive version?
PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{

	// We need to access the map :)
	Map map = gv->map;

	// Calculate the number of rails a hunter can travel.
	int railDist = 0;
	if (player != PLAYER_DRACULA) railDist = (round + player) % 4;

	// Create temp array to keep track of  locations visited in this function.
	PlaceId visited[NUM_REAL_PLACES];
	PlaceId *visited_rail = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	memoryError(visited_rail);
	for (int j = 0; j < NUM_REAL_PLACES; j++) {
	    visited_rail[j] = '\0';
	    visited[j] = '\0';
	}

	// Get the connections from that point.
	ConnList list = MapGetConnections(map, from);

	// Iterate through...
	int loc_num = 0;
	int rail_num = 0;
	visited[loc_num] = from;
	loc_num = 1;

	while (loc_num < NUM_REAL_PLACES && rail_num < NUM_REAL_PLACES) {

	    // Extra conditions for drac:
	    if (player == PLAYER_DRACULA) {
	        if (list->p == HOSPITAL_PLACE) {
	            if (list->next == NULL) break;
	            list = list->next;
	            continue;
            }
	        if (list->type == RAIL) {
	            if (list->next == NULL) break;
	            list = list->next;
	            continue;
            }
	    }

	    // If it is a road type.
	    if (list->type == ROAD) {
	        visited[loc_num] = list->p;
	        loc_num++;
	        // If it is a rail type check for hunter.
	    } else if (list->type == RAIL && railDist > 0) {
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

	// Combine arrays!!
	// So we know the number of locs/ rails in each array;
	i = 0;
	int j = loc_num;
	while (i < rail_num && j < NUM_REAL_PLACES) {

	    visited[j] = visited_rail[i];
	    j++;
	    i++;

	}
	int total_locs = j;

	// Now copy into the dynamically allocated array.
	PlaceId *final_loc_list = malloc(total_locs * sizeof(PlaceId));
	memoryError(final_loc_list);
	i = 0;
	while (i < total_locs) {
	    final_loc_list[i] = visited[i];
	    i++;
	}

    // Memory
    free(visited_rail);

    // Return values...
	*numReturnedLocs = total_locs;
	return final_loc_list;

}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{

	// We need to access the map :)
	Map map = gv->map;

	// Calculate the number of rails a hunter can travel.
	int railDist = 0;
	if (player != PLAYER_DRACULA) railDist = (round + player) % 4;

	// Create temp array to keep track of  locations visited in this function.
	PlaceId visited[NUM_REAL_PLACES];
	PlaceId *visited_rail = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	memoryError(visited_rail);
	for (int j = 0; j < NUM_REAL_PLACES; j++) {
	    visited_rail[j] = '\0';
	    visited[j] = '\0';
	}

	// Get the connections from that point.
	ConnList list = MapGetConnections(map, from);

	// Iterate through...
	int loc_num = 0;
	int rail_num = 0;
	visited[loc_num] = from;
	loc_num = 1;

	while (loc_num < NUM_REAL_PLACES && rail_num < NUM_REAL_PLACES) {

	    // Extra conditions for drac:
	    if (player == PLAYER_DRACULA) {
	        if (list->p == HOSPITAL_PLACE) {
	            if (list->next == NULL) break;
	            list = list->next;
	            continue;
            }
	        if (list->type == RAIL) {
	            if (list->next == NULL) break;
	            list = list->next;
	            continue;
            }
	    }

	    // If it is a road type.
	    if (list->type == ROAD && road == true) {
	        visited[loc_num] = list->p;
	        loc_num++;

	        // If it is a rail type check for hunter.
	    } else if (list->type == RAIL && railDist > 0 && rail == true) {
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


	// Combine arrays!!
	// So we know the number of locs/ rails in each array;
	i = 0;
	int j = loc_num;
	while (i < rail_num && j < NUM_REAL_PLACES) {

	    visited[j] = visited_rail[i];
	    j++;
	    i++;

	}
	int total_locs = j;

	// Now copy into the dynamically allocated array.
	PlaceId *final_loc_list = malloc(total_locs * sizeof(PlaceId));
	memoryError(final_loc_list);
	i = 0;
	while (i < total_locs) {
	    final_loc_list[i] = visited[i];
	    i++;
	}

    // Memory
    free(visited_rail);

    // Return values...
	*numReturnedLocs = total_locs;
	return final_loc_list;
}

////////////////////////////////////////////////////////////////////////
//Private Functions



// MEMORY ERROR: Helper function to check correct memory allocation. Exits if
// memory was not correctly allocated
// -- INPUT: pointer to a malloced object
// -- OUTPUT: void, but prints error message and exits code 1
static void memoryError (const void * input) {
	if (input == NULL) {
		fprintf(stderr, "Couldn't Allocate Memory!\n");
		exit(EXIT_FAILURE);
	}
	return;
}

// COMPARATOR: Compare the order of the elements pointed to by *p and *q.
// -- INPUT: two pointers, *p and *q
// -- OUTPUT:
//	<0 If the element pointed by p goes before the element pointed by q,
//	0  If the element pointed by p is equivalent to the element pointed by q,
//	>0 If the element pointed by p goes after the element pointed by q.
static int placeIdCmp(const void *p, const void *q) {
	PlaceId p1 = *(PlaceId *)p;
	PlaceId p2 = *(PlaceId *)q;
	return p2 - p1;
}
//SORT PLACES: Sorts an array of PlaceId's from largest value to smallest
// -- INPUT: Array of PlaceId's, number of items in array to be sorted
// -- OUTPUT: void
//Author: testUtils.c; Edited: Cindy
static void sortPlaces(PlaceId *places, int numPlaces) {
	qsort(places, (size_t)numPlaces, sizeof(PlaceId), placeIdCmp);
	return;
}
// TRAP LOCATION APPEND: Appends input to the trap location array, updates index,
// array is sorted largest to smallest PlaceId value for easy manipulation
// -- INPUT: GameView, PlaceId to append
// -- OUTPUT: void
static void trapLocationAppend(GameView gv, PlaceId location) {
	int index = gv->trapLocationsIndex;
	if (index < MAX_LOC_HISTORY_SIZE) {
		gv->trapLocations[index + 1] = location;
		gv->trapLocationsIndex++;
	}
	sortPlaces(gv->trapLocations, gv->trapLocationsIndex);
	return;
}


// PLACE ID TO ASCII DOUBLEBACK: Converts doubleback placeids to the enumed
// doubleback values
// -- INPUT: PlaceId
// -- OUTPUT: void
static int PlaceIdToAsciiDoubleBack (PlaceId place) {
	return 1 + (place - DOUBLE_BACK_1);
}

// TRAP LOCATION REMOVE: Removes a specified location from trap location array
// -- INPUT: GameView, PlaceId
// -- OUTPUT: void
static void trapLocationRemove(GameView gv, PlaceId location) {
	int i = 0;
	//find index of trap location (sorted largest to smallest PlaceId value)
	while (i <= gv->trapLocationsIndex) {
		if(gv->trapLocations[i] == location) break;
		i++;
	}
	//remove from location by setting to nowhere
	gv->trapLocations[i] = NOWHERE;

	//shuffle array so smallest numbers are at end (NOWHERE is smallest PlaceId value)
	//index has shrunk so NOWHERE will fall off end of array
	sortPlaces(gv->trapLocations, gv->trapLocationsIndex+1);
	gv->trapLocationsIndex--;
	return;
}

// HUNTER LOCATION HISTORY APPEND: Appends input placeid to a player's
// locationhistory & move history, updates current location and index.
// Note: hunter's location and move history are always the smae
// -- INPUT: GameView, Player, PlaceId to append
// -- OUTPUT: void
static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location) {
	int index = HUNTER->currentLocationIndex;
	// ensure the array is large enough, then append
	if (index < MAX_LOC_HISTORY_SIZE) {
		HUNTER->locationHistory[index + 1] = location;
		HUNTER->moveHistory[index + 1] = location;
		//Hunters gain health when resting at city
		PlaceId previousLocation = HUNTER->currentLocation;
		if (previousLocation == location) HUNTER->health += LIFE_GAIN_REST;
		HUNTER->currentLocation = location;
		HUNTER->currentLocationIndex++;
		//Ensure hunter health is capped
		if(HUNTER->health > GAME_START_HUNTER_LIFE_POINTS) HUNTER->health = GAME_START_HUNTER_LIFE_POINTS;
	}
	// otherwise print error and exit
	else {
		fprintf(stderr, "%s", "location history indexed out of bounds, aborting");
		exit(EXIT_FAILURE);
	}
	return;
}



// DRACULA LOCATION HISTORY APPEND: Appends moves (including unknown locations)
// to Dracula's movehistory, updates current location and index.
// Appends actual locations to Dracula's locationhistory (such as hide & double back)
// Adjust health if Drac is at sea or in Castle Dracula
// -- INPUT: GameView, PlaceId to append
// -- OUTPUT: void
static void draculaLocationHistoryAppend(GameView gv, PlaceId location) {

	int index = DRACULA->currentLocationIndex;
	PlaceId actualLocation = NOWHERE;
	int numReturnedLocs = 0;
	bool canFree = false;
	//Get Dracula's trail (last 6 moves)
	PlaceId *trail = GvGetLastLocations(gv, PLAYER_DRACULA , TRAIL_SIZE,
								&numReturnedLocs, &canFree);
	// ensure the array is large enough, then append to move history
	if (index < MAX_LOC_HISTORY_SIZE) {
		DRACULA->moveHistory[index + 1] = location;
		//HIDE: dracula's location is the same as previous
		if(location == HIDE && trail != NULL){
			actualLocation = trail[numReturnedLocs-1];
			DRACULA->locationHistory[index + 1] = actualLocation;
			DRACULA->currentLocation = actualLocation;
		}
		//DOUBLE_BACK_X: dracula is now at city he was in x moves ago
		else if (trail != NULL && location >= DOUBLE_BACK_1 && location <=DOUBLE_BACK_5){
			actualLocation = trail[numReturnedLocs-PlaceIdToAsciiDoubleBack(location)];
			DRACULA->locationHistory[index + 1] = actualLocation;
			DRACULA->currentLocation = actualLocation;
		}
		//Draculas location was not hidden/double back
		else {
			DRACULA->locationHistory[index + 1] = location;
			DRACULA->currentLocation = location;
		}
		//Dracula loses health when at sea
		if(placeIdToType(location) == SEA || placeIdToType(actualLocation) == SEA) {
			DRACULA->health -= (LIFE_LOSS_SEA);
		}
		//Dracula gains health when he teleports home to CASTLE_DRACULA
		if(location == TELEPORT || location == CASTLE_DRACULA || actualLocation == CASTLE_DRACULA) 		{
			DRACULA->health += LIFE_GAIN_CASTLE_DRACULA;
		}
		DRACULA->currentLocationIndex++;
	}
	// otherwise print error and exit
	else {
		fprintf(stderr, "%s", "location history indexed out of bounds, aborting");
		exit(EXIT_FAILURE);
	}
	free(trail);
	return;
}



// CHECK HUNTER HEALTH: Checks if a hunter has died, if so, moves them to hospital
// -- INPUT: GameView, Player
// -- OUTPUT: void
static void checkHunterHealth(GameView gv,Player hunter){
	if(gv->allPlayers[hunter]->health <= 0) {
		gv->allPlayers[hunter]->health = 0;
		gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
		hunterLocationHistoryAppend(gv, hunter, HOSPITAL_PLACE);
		// todo need to update hunter hp? -> check at start of hunter round if they are in hospital, then restore health?
	}
	return;
}

// INITIALISE PLAYER: Initialises a player to defaults, assigns memory
// -- INPUT: GameView, Player
// -- OUTPUT: Address to new initialised memory
static PlayerData initialisePlayer(GameView gv, Player player) {

	PlayerData new = malloc(sizeof(* new));
	memoryError (new);

	new -> health = GAME_START_HUNTER_LIFE_POINTS;
	new -> currentLocation = NOWHERE;
	new -> currentLocationIndex = -1;
	new -> locationHistory[0] = '\0';
	new -> moveHistory[0] = '\0';
	new -> lastHidden = -1;
	new -> lastDoubleback = -1;
	if (player == PLAYER_DRACULA) {
		new -> health = GAME_START_BLOOD_POINTS;
	} else {
		new -> health = GAME_START_HUNTER_LIFE_POINTS;
	}
	return new;
}

// INITIALISE GAME: Assigns memory and sets values to default/null values
// -- INPUT: GameView
// -- OUTPUT: void
static void initialiseGame (GameView gv) {

	gv->roundNumber = 0;
	gv->score = GAME_START_SCORE;
	// Always starts with G
	gv->currentPlayer = PLAYER_LORD_GODALMING;

	// Allocate memory for players & Initialise starting information
	for (int i = 0; i < NUM_PLAYERS; i++){
		gv->allPlayers[i] = initialisePlayer(gv, i);
	}

	gv->trapLocationsIndex = -1;
	gv->vampire = NOWHERE;

	return;
}

// PARSE MOVE: interprets a single move, calls hunter/draculaMove, updates curr_player
// -- Input: GameView, move string
// -- Output: current Player
// Author: Cindy (Tara edited)
static Player parseMove (GameView gv, char *string){

	char *c = string;
	Player curr_player;

	// Determine last player from string; update next player
	// 5 players that always go in order of G->S->H->M->D
	switch(*c){
			case 'G':
			    hunterMove(gv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;

			case 'S':
			    hunterMove(gv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_VAN_HELSING;
			    break;

			case 'H':
			    hunterMove(gv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_MINA_HARKER;
			    break;

			case 'M':
			    hunterMove(gv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;

			case 'D':
			    draculaMove(gv, string);
			    curr_player = PLAYER_LORD_GODALMING;
			    break;
		}

	return curr_player;
}

// HUNTER MOVE: Reads through hunter's string to determine actions taken
// -- Input: GameView, pastPlays string, hunter in play
// -- Output: void
// Author: Cindy (Tara edited)
static void hunterMove(GameView gv, char *string, Player hunter) {

	// String must be of valid size
	assert (strlen(string) > LOCATION_ID_SIZE);

	// Store locationID into city[]:
	char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
	memoryError(city);
	city[0] = string[1];
	city[1] = string[2];
	city[2] = '\0';
	//If hunter was in hospital, restore health points
	if(HUNTER->currentLocation == HOSPITAL_PLACE) HUNTER->health = GAME_START_HUNTER_LIFE_POINTS;
    // Compare and find city by abbreviation:
	PlaceId curr_place = placeAbbrevToId(city);

 	if (curr_place == NOWHERE) printf("Error: Place not found...\n");

    // Append history and current location:
    hunterLocationHistoryAppend(gv, hunter, curr_place);

	// Parsing through characters after location to determine actions
	char *c;
	for (int i = 3; i < strlen(string); i++) {
		c = &string[i];
		switch(*c){
			// It's a trap!
			case ITS_A_TRAP:
				gv->allPlayers[hunter]->health -= LIFE_LOSS_TRAP_ENCOUNTER;
				checkHunterHealth(gv, hunter);
				//remove trap
				trapLocationRemove(gv, curr_place);
				break;

			// Immature Vampire encounter
			case CLOSE_ENCOUNTERS_OF_THE_VTH_KIND:
				gv->vampire = NOWHERE;
				break;

			// Dracula encounter
			case 'D':
				gv->allPlayers[hunter]->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
				checkHunterHealth(gv, hunter);
				DRACULA->health -= LIFE_LOSS_HUNTER_ENCOUNTER;
				break;
			// other characters include trailing '.'
			case '.':
				break;
		}
	}
	free(city);
    return;
}

// DRACULA MOVE: Reads through drac's string to determine actions taken
// -- Input: GameView, pastPlays string
// -- Output: void
// Author: Cindy
static void draculaMove(GameView gv, char *string) {

	// String must be of valid size
	assert (strlen(string) > LOCATION_ID_SIZE);

	// Store locationID into city[]:
	char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
	memoryError(city);
	city[0] = string[1];
	city[1] = string[2];
	city[2] = '\0';

    // Compare and find city by abbreviation:
	PlaceId curr_place = placeAbbrevToId(city);

	//Unknown city move
	if (strcmp(city, "C?") == 0) {
		draculaLocationHistoryAppend(gv, CITY_UNKNOWN);
	}
	// Unknown sea move
	else if (strcmp(city,"S?") == 0) {
		draculaLocationHistoryAppend(gv, SEA_UNKNOWN);
	}
	// Hide move ->stays in the city for another round
	else if (strcmp(city,"HI") == 0) {
		DRACULA->lastHidden = gv->roundNumber;
		draculaLocationHistoryAppend(gv, curr_place);
	}
	// Double back move
	else if (strncmp(city,"D",1) == 0) {
		draculaLocationHistoryAppend(gv, curr_place);
		DRACULA->lastDoubleback = gv->roundNumber;
	}
	//Location move that was revealed (ie all other cases)
	else {
    	draculaLocationHistoryAppend(gv, curr_place);
	}

	// Parsing through action characters after location id
	char *c;
	int i = 3;
	while ( i < strlen(string)) {
		c = &string[i];
		// if there are extra characters indicating trap or immature vampire
			// trap left the trail due to age (after 6 moves)
			if (*c ==MALFUNCTIONING_MACHIAVELLIAN_MACHINATIONS) {
				int numReturnedLocs = 0;
				bool canFree = false;
				//retrieve dracula's trail
				PlaceId *trail = GvGetLastLocations(gv, PLAYER_DRACULA , TRAIL_SIZE,
				                            &numReturnedLocs, &canFree);
				//remove the oldest trap in trail from trapLocations
				PlaceId brokenTrap = trail[0];
				trapLocationRemove(gv, brokenTrap);
				free(trail);
			}

			if (*c == CLOSE_ENCOUNTERS_OF_THE_VTH_KIND) {
				// Immature vampire has matured
				if( i == 5) {
					gv->vampire = NOWHERE;
					gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
				}
				//immature vampire placed
				else {
					gv->vampire = curr_place;
				}
			}

			// Trap placed
			if (*c == ITS_A_TRAP) trapLocationAppend(gv, curr_place);
		i++;
	}
	// Game score decreases each time drac finishes turn
    gv->score -= SCORE_LOSS_DRACULA_TURN;
    free(city);
    return;
}

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
    while (i < NUM_REAL_PLACES && list != NULL) {

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
            array[i] = list->p;
            i++;
            places_added++;
        }

        // Increment.
        if (list->next == NULL) break;
        list = list->next;
    }
    return places_added;
}
