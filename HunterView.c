////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
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
#include "HunterView.h"
#include "Map.h"
#include "Places.h"
#include "Queue.h"

#define PLAY_S_SIZE 7
#define LOCATION_ID_SIZE 2
#define MAX_LOC_HISTORY_SIZE (GAME_START_SCORE * 2 * 4 )
#define OUR_ARBITRARY_ARRAY_SIZE 10

// some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'
#define MALFUNCTIONING_MACHIAVELLIAN_MACHINATIONS 'M'

// #defines to make things more readable
#define  LORD_GODALMING hv->allPlayers[PLAYER_LORD_GODALMING]
#define  DR_SEWARD 		hv->allPlayers[PLAYER_DR_SEWARD]
#define  VAN_HELSING  	hv->allPlayers[PLAYER_VAN_HELSING]
#define  MINA_HARKER 	hv->allPlayers[PLAYER_MINA_HARKER]
#define  DRACULA 		hv->allPlayers[PLAYER_DRACULA]
#define  PLAYER 		hv->allPlayers[player]
#define  HUNTER			hv->allPlayers[hunter]


typedef struct playerData *PlayerData;

// ADT that stores all data for each player:
struct playerData {

	int health;											// Current player health
	PlaceId locationHistory[MAX_LOC_HISTORY_SIZE];		// Array of locations visited by player
	PlaceId moveHistory[MAX_LOC_HISTORY_SIZE];			// Array of moves made by player (different to loc history for drac)
	PlaceId currentLocation;							// Current location
	int currentLocationIndex;							// Index of current location in locationHistory

	// ---- ONLY FOR DRAC
	int lastHidden;										// Round in which drac last hid
	int lastDoubleback;									// Round in which drac last doubled back
};

struct hunterView {
	Round roundNumber;									// Current round
	int score;											// Current game score
	Player currentPlayer;								// The player currently having their turn
	PlayerData allPlayers[NUM_PLAYERS]; 				// Array of playerData structs
	PlaceId trapLocations[MAX_LOC_HISTORY_SIZE];		// Array of trap locations -- multiple traps in same place are seperate
	int trapLocationsIndex;								// Index of the most recently added trap location
	PlaceId vampire;									// Only one vampire alive at any time
	Map map;											// Graph thats been typedefed already
	int temp_round;										// Temp round & place variable to determine shortest path...
	PlaceId temp_place;
};

// PRIVATE FUNCTION DECLARATIONS
// ****************************************

//------------- MAKING A MOVE -------------
// Helper function for reachables:
static int Find_Rails (Map map, PlaceId place, PlaceId from, PlaceId *array, int i);
static PlaceId *HvGetReachable(HunterView hv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs);
static PlaceId *HvGetReachableByType(HunterView hv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs);

//------------- GENERAL FUNCTIONS -------------
// Qsort comparator:
static int placeIdCmp(const void *ptr1, const void *ptr2);
// Memory error test:
static void memoryError (const void * input);
// Sorts an array of PlaceIds
static void sortPlaces(PlaceId *places, int numPlaces);

//------------- CONSTRUCTOR/ DESTRUCTOR -------------
// Initialise an empty game to fill in:
static void initialiseGame (HunterView hv);
static PlayerData initialisePlayer(HunterView hv, Player player);
// Parse the move string:
static Player parseMove (HunterView hv, char *string);
// Apply hunter move from parsed string:
static void hunterMove(HunterView hv, char *string, Player hunter);
// Apply dracula move from parsed string:
static void draculaMove(HunterView hv, char * string);
// Convert a doubleback placeid to doubleback value:
static int PlaceIdToAsciiDoubleBack (PlaceId place);
// Add a trap location:
static void trapLocationAppend(HunterView hv, PlaceId location);
// Remove a trap location:
static void trapLocationRemove(HunterView hv, PlaceId location);
// Check the health of a hunter, sends them to hospital if needed:
static void checkHunterHealth(HunterView hv,Player hunter);
// Checks if maximum encounters for a location have already been placed
static bool maxEncounters(HunterView hv, PlaceId location);

//------------- GAME HISTORY -------------
static PlaceId *HvGetMoveHistory(HunterView hv, Player player,
                          int *numReturnedMoves, bool *canFree);
//static PlaceId *HvGetLastMoves(HunterView hv, Player player, int numMoves,
//                       int *numReturnedMoves, bool *canFree);
static PlaceId *HvGetLocationHistory(HunterView hv, Player player,
                              int *numReturnedLocs, bool *canFree);
static PlaceId *HvGetLastLocations(HunterView hv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree);

// ****************************************


////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

// HV NEW: allocates memory for a new HunterView, initialises default values
// returns new HunterView
HunterView HvNew(char *pastPlays, Message messages[]) {
	// Create new HunterView:
	HunterView new = malloc(sizeof(*new));

	// Check if memory was allocated correctly:
	memoryError(new);
	new->map = MapNew();
	initialiseGame (new);

	// Number of Rounds can be determined from size of string
    // ( each play is 7 chars + space)
	new->roundNumber = (strlen(pastPlays) + 1) / ((PLAY_S_SIZE + 1)*5);

    // Need to create a copy of pastPlays.
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

// HV FREE: frees memory associated with the input HunterView
void HvFree(HunterView hv) {
	// Free player structs
	for (int i = 0; i < NUM_PLAYERS; i++)
		free(hv->allPlayers[i]);

	// Free map
	MapFree(hv->map);
	// Free hunter view
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv) {
	return hv->roundNumber;
}

Player HvGetPlayer(HunterView hv) {
	return hv->currentPlayer;
}

int HvGetScore(HunterView hv) {
	return hv->score;
}

int HvGetHealth(HunterView hv, Player player) {
	return hv->allPlayers[player]->health;
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player) {
	if (player == PLAYER_DRACULA) return hv->allPlayers[player]->currentLocation;
	else return hv->allPlayers[player]->currentLocation;
}

PlaceId HvGetVampireLocation(HunterView hv) {
	return hv->vampire;
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

// LAST KNOWN DRACULA LOCATION: returns the last known real dracula location,
// or NOWHERE if he has not yet been sighted
PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round) {
	// Head backwards through location history, != NOWHERE && == REALPLACE.
	int round_num = -1;
	for (int i = DRACULA->currentLocationIndex; i > -1; i--)
	{
	    if (DRACULA->locationHistory[i] != NOWHERE &&
	        placeIsReal(DRACULA->locationHistory[i]))
        {
            round_num = i;
            break;
        }
	}

	*round = round_num;
	if (round_num == -1) return NOWHERE;
	else return DRACULA->locationHistory[round_num];
}

// GET SHORTEST PATH TO: returns the shortest path to a location
PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength) {
	// initialise
	Map map = hv->map;
	int src = HUNTER->currentLocation;
	int des = dest;

	// check map loaded correctly
	assert (map != NULL);
	// If destination == starting position simply return start
	if (src == des) {
		PlaceId *path = malloc(2 * sizeof(PlaceId));
		memoryError(path);
		path[0] = dest;
		path[1] = '\0';

		*pathLength = 1;
	    return path;
	}

    // Make a visited array
    int visited[NUM_REAL_PLACES];
    for (int i = 0; i < NUM_REAL_PLACES; i++) {
        visited[i] = -1;
    }

    visited[0] = src;

	int found = 0;
	int pathFound = 0;
	hv->temp_round = hv->roundNumber;
	hv->temp_place = HUNTER->currentLocation;

	// Make Queue to travel breadth-first
	Queue q = newQueue();
    QueueJoin(q, src);

	// BFS
    while (!found && !QueueIsEmpty(q)) {
        int prev_place = QueueLeave(q);
		// When we create a new_place, we must create from connections of prev.
		hv->temp_place = prev_place;
		// Checking the round:
		int path_count = 0;
		int prev = prev_place;
		while (prev != src) {
		    path_count++;
		    prev = visited[prev];
        }
        hv->temp_round = hv->roundNumber + path_count;

		int numLocs;
		PlaceId *list = HvWhereCanTheyGo(hv, hunter, &numLocs);

		if (prev_place == des) {
			found = 1;
		} else for (int i = 1; i < numLocs; i++) {
			int new_place = list[i];
			if (visited[new_place] == -1 && prev_place != new_place) {
				visited[new_place] = prev_place;
				QueueJoin(q, new_place);
				if (new_place == dest) {
					pathFound = 1;
				}
			}
		}
		free(list);
    }
	dropQueue(q);

	// Reset values:
	hv->temp_place = NOWHERE;
	hv->temp_round = -1;

	if (pathFound == 0) return 0;

	int reversePath[NUM_REAL_PLACES];
	reversePath[0] = dest;

	int prev = visited[dest];
	int k = 1;
 	while (prev != src) {
		reversePath[k] = prev;
		k++;
		prev = visited[prev];
	}

	reversePath[k] = src;

    PlaceId *path = malloc ((k + 1) * sizeof(PlaceId));
	memoryError(path);
    PlaceId place;
	for (int i = 0, j = k; i <= k; i++, j--) {
		place = reversePath[j];
		path[i] = place;
	}

	// Mind blank about shortening array from front, so...
	PlaceId *path_without_src = malloc ((k) * sizeof(PlaceId));
	memoryError(path_without_src);
	for (int i = 0; i < k; i++) {
	    path_without_src[i] = path[i + 1];
	}
	free(path);
	*pathLength = k;
	return path_without_src;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

// WHERE CAN I GO: returns a list of all the locations reachable by the current
// player on their turn
PlaceId *HvWhereCanIGo (HunterView hv, int *numReturnedLocs) {
	// Set values:
	Player hunter = HvGetPlayer(hv);
	Round round = HvGetRound(hv) + 1;
	PlaceId from = HvGetPlayerLocation(hv, hunter);
	PlaceId *locs = NULL;
	int numLocs = -1;

	// Use function:
	locs = HvGetReachable (hv, hunter, round, from, &numLocs);

	// Return values:
	*numReturnedLocs = numLocs;
	return locs;
}

// WHERE CAN I GO: returns a list of all the locations reachable by the current
// player on their turn, with the ability to filter by location type
PlaceId *HvWhereCanIGoByType (HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs) {
	// Set values:
	Player hunter = HvGetPlayer(hv);
	Round round = HvGetRound(hv) + 1;
	PlaceId from = HvGetPlayerLocation(hv, hunter);
	PlaceId *locs = NULL;
	int numLocs = -1;

	// Use function:
	locs = HvGetReachableByType (hv, hunter, round, from, road, rail, boat, &numLocs);

	// Return values:
	*numReturnedLocs = numLocs;
	return locs;
}

// WHERE CAN THEY GO: returns a list of all the locations reachable by the specified
// player on their turn
PlaceId *HvWhereCanTheyGo (HunterView hv, Player player, int *numReturnedLocs) {
	// Set values:
	Round round = hv->allPlayers[player]->currentLocationIndex + 1;
	if (round == -1) round = 0;
	if (player == HvGetPlayer(hv)) round++;
	PlaceId from = HvGetPlayerLocation(hv, player);
	PlaceId *locs = NULL;
	int numLocs = -1;

	// Use function:
	locs = HvGetReachable (hv, player, round, from, &numLocs);

	// Return values:
	*numReturnedLocs = numLocs;
	return locs;
}


// WHERE CAN THEY GO BY TYPE: returns a list of all the locations reachable by the
// specified player on their turn, with the ability to filter by location type
PlaceId *HvWhereCanTheyGoByType (HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs) {
	// Set values:
	Round round = hv->allPlayers[player]->currentLocationIndex + 1;
	if (round == -1) round = 0;
	PlaceId from = HvGetPlayerLocation(hv, player);
	PlaceId *locs = NULL;
	int numLocs = -1;

	// Use function:
	locs = HvGetReachableByType (hv, player, round, from, road, rail, boat, &numLocs);

	// Return values:
	*numReturnedLocs = numLocs;
	return locs;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// ------------------ GENERAL FUNCTIONS ------------------ //
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

// COMPARATOR: Compare the order of the elements pointed to by *p and *q. Returns:
//	<0 If the element pointed by p goes before the element pointed by q,
//	0  If the element pointed by p is equivalent to the element pointed by q,
//	>0 If the element pointed by p goes after the element pointed by q.
// -- INPUT: two pointers, *p and *q
// -- OUTPUT: int
static int placeIdCmp(const void *ptr1, const void *ptr2) {
	PlaceId p1 = *(PlaceId *)ptr1;
	PlaceId p2 = *(PlaceId *)ptr2;
	return p2 - p1;
}

//Author: testUtils.c; Edited: Cindy
static void sortPlaces(PlaceId *places, int numPlaces) {
	qsort(places, (size_t)numPlaces, sizeof(PlaceId), placeIdCmp);
	return;
}

// ------------------ HELPING WITH MAIN HUNTER VIEW ------------------ //
// TRAP LOCATION APPEND: Appends input to the trap location array, updates index,
//array is sorted largest to smallest PlaceId value for easy manipulation
// -- INPUT: HunterView, PlaceId to append
// -- OUTPUT: void
static void trapLocationAppend(HunterView hv, PlaceId location) {
	int index = hv->trapLocationsIndex;
	if (index < MAX_LOC_HISTORY_SIZE) {
		hv->trapLocations[index + 1] = location;
		hv->trapLocationsIndex++;
	}
	sortPlaces(hv->trapLocations, hv->trapLocationsIndex);
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
// -- INPUT: HunterView, PlaceId
// -- OUTPUT: void
static void trapLocationRemove(HunterView hv, PlaceId location) {
	int i = 0;
	//find index of trap location (sorted largest to smallest PlaceId value)
	while (i <= hv->trapLocationsIndex) {
		if(hv->trapLocations[i] == location) break;
		i++;
	}
	//remove from location by setting to nowhere
	hv->trapLocations[i] = NOWHERE;

	//shuffle array so smallest numbers are at end (NOWHERE is smallest PlaceId value)
	//index has shrunk so NOWHERE will fall off end of array
	sortPlaces(hv->trapLocations, hv->trapLocationsIndex+1);
	hv->trapLocationsIndex--;
	return;
}

// HUNTER LOCATION HISTORY APPEND: Appends input placeid to a player's
// locationhistory, updates current location and index.
// -- INPUT: HunterView, Player, PlaceId to append
// -- OUTPUT: void
static void hunterLocationHistoryAppend(HunterView hv, Player hunter, PlaceId location) {
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



// DRACULA LOCATION HISTORY APPEND: Appends input placeid to a drac's
// locationhistory, updates current location and index. If the location is
// at sea, reduce drac's health.
// -- INPUT: HunterView, PlaceId to append
// -- OUTPUT: void
static void draculaLocationHistoryAppend(HunterView hv, PlaceId location) {

	int index = DRACULA->currentLocationIndex;
	PlaceId actualLocation = NOWHERE;
	int numReturnedLocs = 0;
	bool canFree = false;
	//Get Dracula's trail (last 6 moves)
	PlaceId *trail = HvGetLastLocations(hv, PLAYER_DRACULA , TRAIL_SIZE,
								&numReturnedLocs, &canFree);

	// Ensure the array is large enough, then append
	if (index < MAX_LOC_HISTORY_SIZE) {
		DRACULA->moveHistory[index + 1] = location;

		// Dracula's location is the same as previous
		if (location == HIDE && trail != NULL){
			actualLocation = trail[numReturnedLocs - 1];
			DRACULA->locationHistory[index + 1] = actualLocation;
			DRACULA->currentLocation = actualLocation;
		}

		// Go back to x previous locations
		else if (trail != NULL && location >= DOUBLE_BACK_1 && location <=DOUBLE_BACK_5){
			actualLocation = trail[numReturnedLocs - PlaceIdToAsciiDoubleBack(location)];
			DRACULA->locationHistory[index + 1] = actualLocation;
			DRACULA->currentLocation = actualLocation;
		}

		//Draculas location was not hidden/double back
		else {
			DRACULA->locationHistory[index + 1] = location;
			DRACULA->currentLocation = location;
		}

		if (placeIdToType(location) == SEA || placeIdToType(actualLocation) == SEA) {
			DRACULA->health -= (LIFE_LOSS_SEA);
		}
		if (location == TELEPORT || location == CASTLE_DRACULA ||
		    actualLocation == CASTLE_DRACULA)
	    {
			DRACULA->health += LIFE_GAIN_CASTLE_DRACULA;
		}
		DRACULA->currentLocationIndex++;
	}

	// Otherwise print error and exit
	else {
		fprintf(stderr, "%s", "location history indexed out of bounds, aborting");
		exit(EXIT_FAILURE);
	}
	free(trail);
	return;
}



// CHECK HUNTER HEALTH: Checks if a hunter has died, if so, moves them to hospital
// -- INPUT: HunterView, Player
// -- OUTPUT: void
static void checkHunterHealth(HunterView hv,Player hunter){
	if(hv->allPlayers[hunter]->health <= 0) {
		hv->allPlayers[hunter]->health = 0;
		hv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
		HUNTER->currentLocation = HOSPITAL_PLACE;
		//hunterLocationHistoryAppend(gv, hunter, HOSPITAL_PLACE);
	}
	return;
}

// INITIALISE PLAYER: Initialises a player to defaults, assigns memory
// -- INPUT: HunterView, Player
// -- OUTPUT: void
static PlayerData initialisePlayer(HunterView hv, Player player) {

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
		new -> health = 	GAME_START_HUNTER_LIFE_POINTS;
	}

	return new;
}

// INITIALISE GAME: Assigns memory and sets values to default/null values
// -- INPUT: HunterView
// -- OUTPUT: void
static void initialiseGame (HunterView hv) {

	hv->roundNumber = 0;
	hv->score = GAME_START_SCORE;
	// Always starts with G
	hv->currentPlayer = PLAYER_LORD_GODALMING;

	// Allocate memory for players & Initialise starting information
	for (int i = 0; i < NUM_PLAYERS; i++){
		hv->allPlayers[i] = initialisePlayer(hv, i);
	}

    // No trap locations at start of game, therefore no array yet..
	// hv->trapLocations = NULL;
	hv->trapLocationsIndex = -1;
	hv->vampire = NOWHERE;
	hv->temp_round = -1;
	hv->temp_place = NOWHERE;

	return;

}

// PARSE MOVE: interprets a single move, calls hunter/draculaMove, updates curr_player
// -- Input: HunterView, move string
// -- Output: current Player
static Player parseMove (HunterView hv, char *string){

	char *c = string;
	Player curr_player;

	// Figure out whose move it was
	switch(*c){
			case 'G':
			    hunterMove(hv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;

			case 'S':
			    hunterMove(hv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_VAN_HELSING;
			    break;

			case 'H':
			    hunterMove(hv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_MINA_HARKER;
			    break;

			case 'M':
			    hunterMove(hv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;

			case 'D':
			    draculaMove(hv, string);
			    curr_player = PLAYER_LORD_GODALMING;
			    break;
		}

	return curr_player;
}

// HUNTER MOVE: Reads through hunter's string to determine actions taken
// -- Input: GameView, pastPlays string, hunter in play
// -- Output: void
static void hunterMove(HunterView hv, char *string, Player hunter) {

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
    hunterLocationHistoryAppend(hv, hunter, curr_place);

	// Parsing through characters after location iD
	char *c;

	for (int i = 3; i < strlen(string); i++) {
		c = &string[i];
		switch(*c){
			// It's a trap!
			case ITS_A_TRAP:
				hv->allPlayers[hunter]->health -= LIFE_LOSS_TRAP_ENCOUNTER;
				checkHunterHealth(hv, hunter);
				// Remove trap
				trapLocationRemove(hv, curr_place);
				break;

			// Immature Vampire encounter
			case CLOSE_ENCOUNTERS_OF_THE_VTH_KIND:
				hv->vampire = NOWHERE;
				break;

			// Dracula encounter
			case 'D':
				hv->allPlayers[hunter]->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
				checkHunterHealth(hv, hunter);
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
static void draculaMove(HunterView hv, char *string) {

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
		draculaLocationHistoryAppend(hv, CITY_UNKNOWN);
	}

	// Unknown sea move
	else if (strcmp(city,"S?") == 0) {
		draculaLocationHistoryAppend(hv, SEA_UNKNOWN);
	}

	// Hide move ->stays in the city for another round
	else if (strcmp(city,"HI") == 0) {
		DRACULA->lastHidden = hv->roundNumber;
		draculaLocationHistoryAppend(hv, curr_place);
	}
	// Double back move
	else if (strncmp(city,"D",1) == 0) {
		draculaLocationHistoryAppend(hv, curr_place);
		DRACULA->lastDoubleback = hv->roundNumber;
	}
	// Location move that was revealed (ie all other cases)
	else {
    	draculaLocationHistoryAppend(hv, curr_place);
	}

	// Parsing through characters after location id
	char *c;
	int i = 3;

	while ( i < strlen(string)) {
		c = &string[i];

		// If there are extra characters indicating trap or immature vampire
			// Trap left the trail due to age
			if (*c == MALFUNCTIONING_MACHIAVELLIAN_MACHINATIONS) {
				int numReturnedLocs = 0;
				bool canFree = false;
				PlaceId *trail = HvGetLastLocations(hv, PLAYER_DRACULA , TRAIL_SIZE,
				                            &numReturnedLocs, &canFree);
				//remove the oldest trap in trail from trapLocations
				PlaceId brokenTrap = trail[0];
				trapLocationRemove(hv, brokenTrap);
				free(trail);
			}

			if (*c == CLOSE_ENCOUNTERS_OF_THE_VTH_KIND) {
				// Immature vampire has matured
				if( i == 5) {
					//check that vampire exists
					if (hv->vampire != NOWHERE) {
						hv->vampire = NOWHERE;
						hv->score -= SCORE_LOSS_VAMPIRE_MATURES;
					}
				}
				//immature vampire placed
				else {
					if(maxEncounters(hv,curr_place) == false) hv->vampire = curr_place;
				}
			}

			// Trap placed
			if (*c == ITS_A_TRAP) {
				if(maxEncounters(hv, curr_place) == false)	trapLocationAppend(hv, curr_place);
			}
		i++;
	}

	// Game score decreases each time drac finishes turn
    hv->score -= SCORE_LOSS_DRACULA_TURN;
    free(city);
    return;
}

// ------------------ HELPING WITH GAME HISTORY ------------------ //
// GET MOVE HISTORY: Returns complete move history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed
// -- INPUT: hv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
static PlaceId *HvGetMoveHistory(HunterView hv, Player player,
                          int *numReturnedMoves, bool *canFree)
{

	// can't free as is returning directly from data struct
	*canFree = false;
	// pass number of moves
	*numReturnedMoves = hv->allPlayers[player]->currentLocationIndex+1;
	return hv->allPlayers[player]->moveHistory;

}

// GET MOVE HISTORY: Returns last n moves of a player in dynamically allocated
// array, indicates the number of moves and that the return array can be freed
// -- INPUT: hv, player, number of moves to return, pointer to an int storing
// the number of returned moves, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
/*
static PlaceId *HvGetLastMoves(HunterView hv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{

	// unless asking for more locations than have happened, return numlocs
	if (hv->allPlayers[player]->currentLocationIndex >= numMoves) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedMoves = numMoves;
		// allocate space for return array
		PlaceId *lastNMoves = malloc(sizeof(PlaceId) *numMoves);
		memoryError(lastNMoves);
		// copy desired values from locationHistory to return array
		for (int i = 0; i < numMoves; i++) {
			lastNMoves[i] = hv->allPlayers[player]->moveHistory[hv->allPlayers[player]->currentLocationIndex - numMoves + i];
		}
		return lastNMoves;
	}

	// if asking for too many locations, only return all that exist
	return HvGetLocationHistory(hv, player, numReturnedMoves, canFree);

}
*/

// GET LOCATION HISTORY: Returns complete location history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed. Returns UNKNOWN places where relevant for dracula.
// -- INPUT: hv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
static PlaceId *HvGetLocationHistory(HunterView hv, Player player,
                              int *numReturnedLocs, bool *canFree)
{

	int index = hv->allPlayers[player]->currentLocationIndex;
	*numReturnedLocs = index + 1;
	//if there are no moves in history, return NULL
	if(index < 0) return NULL;
	*canFree = true;
	PlaceId *allLocs = malloc(sizeof(PlaceId) * *numReturnedLocs);
	memoryError(allLocs);
	if (player == PLAYER_DRACULA) {
		// todo make sure rounds when drac not visible returned as unknowns
		// depends on how visibility is parsed in dracmove

		// add locs to all locs, checking if drac visible, and if not adding as unknown
		for (int i = 0; i < *numReturnedLocs; i++) {
			// if visible
			allLocs[i] = hv->allPlayers[player]->locationHistory[i];
			/* else if not visible
				// return unknown sea
				if (placeIdToType(hv->allPlayers[player]->currentLocation) == SEA)
					return SEA_UNKNOWN;
				// or return unknown city
				else
					return CITY_UNKNOWN;*/
		}
		return allLocs;
	}
	return HvGetMoveHistory(hv, player, numReturnedLocs, canFree);
}

// GET LOCATION HISTORY: Returns last n locations of a player in dynamically allocated
// array, indicates the number of locs and that the return array can be freed
// -- INPUT: hv, player, number of locs to return, pointer to an int storing
// the number of returned locs, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
static PlaceId *HvGetLastLocations(HunterView hv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{

	// unless asking for more locations than have happened, return numlocs
	if (hv->allPlayers[player]->currentLocationIndex >= numLocs) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedLocs = numLocs;
		// allocate space for return array
		PlaceId *lastNLocs = malloc(sizeof(PlaceId) *numLocs);
		memoryError(lastNLocs);
		// loop throught location history, adding to lastn in chronological order
		for (int i = 0; i < numLocs; i++) {
			lastNLocs[i] = hv->allPlayers[player]->locationHistory[hv->allPlayers[player]->currentLocationIndex - numLocs + i];
		}
		return lastNLocs;
	}
	// if asking for too many locations, only return all that exist
	return HvGetLocationHistory(hv, player, numReturnedLocs, canFree);
}

// ------------------ HELPING WITH MAKING MOVES ------------------ //
// REACHABLE FUNCTIONS: Help to find all possible moves for the next play based
// on round score, player, starting place, and other game deets.
static PlaceId *HvGetReachable(HunterView hv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{

	// We need to access the map :)
	Map map = hv->map;

	// Make a temp round variable for use with finding shortest path...
	if (hv->temp_round > -1) round = hv->temp_round;
	if (hv->temp_place != NOWHERE) from = hv->temp_place;

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

static PlaceId *HvGetReachableByType(HunterView hv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{

	// We need to access the map :)
	Map map = hv->map;

	// Make a temp round variable for use with finding shortest path...
	if (hv->temp_round > -1) round = hv->temp_round;
	if (hv->temp_place != NOWHERE) from = hv->temp_place;
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

// FIND EXTRA RAIL LOCS: Helper function for reachables to find rail CONNECTIONS.
// -- INPUT: Map, starting place, original place to check as duplicate, array of
// visited rail locations to check for duplicates and add to, and index (since we
// are editing a used array).
// -- OUTPUT: An updated array of visited rail locations and number of locations
// added in this instance.
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

// MAX ENCOUNTERS: Reads through drac's string to determine actions taken
// -- Input: GameView, Location of encounters to be checked
// -- Output: If maximum encounters for a city has been reached
static bool maxEncounters(HunterView hv, PlaceId location){
	int counter = 0;
	if (hv->vampire == location) counter++;
	for (int i = 0; i <= hv->trapLocationsIndex; i++){
		if(hv->trapLocations[i] == location) counter++;
	}
	return (counter >= 3);
}
