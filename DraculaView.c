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
#define PLAY_S_SIZE 7
#define LOCATION_ID_SIZE 2
#define MAX_LOC_HISTORY_SIZE (GAME_START_SCORE * 2 * 4 )
//DOUBLE CHECK THIS: max number of turns * most possible avg moves per turn (rounded up)  * bytes per move stored
#define OUR_ARBITRARY_ARRAY_SIZE 10

// some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'

// defines to make things more readable
#define  LORD_GODALMING dv->allPlayers[PLAYER_LORD_GODALMING]
#define  DR_SEWARD 		dv->allPlayers[PLAYER_DR_SEWARD]
#define  VAN_HELSING  	dv->allPlayers[PLAYER_VAN_HELSING]
#define  MINA_HARKER 	dv->allPlayers[PLAYER_MINA_HARKER]
#define  DRACULA 		dv->allPlayers[PLAYER_DRACULA]
#define  PLAYER 		dv->allPlayers[player]
#define  HUNTER			dv->allPlayers[hunter]

typedef struct playerData *PlayerData;

struct playerData {
	int health;											// current player health
	PlaceId locationHistory[MAX_LOC_HISTORY_SIZE];		// array of locations visited by player
	PlaceId moveHistory[MAX_LOC_HISTORY_SIZE];			//array of moves made by player (different to location history for drac)
	PlaceId currentLocation;						  	// current location
	int currentLocationIndex;						  	// index of current location in locationHistory
	// -------------------------- BLOOD BOIS ONLY BEYOND THIS POINT --------------------------
	// do we even need these?
	int lastHidden;									  	// round in which drac last hid
	int lastDoubleback;									// round in which drac last doubled back
	// bool isVisible									// something to indicate whether hunters know drac curr location
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

//------------- GENERAL FUNCTIONS -------------
// Qsort comparator:
int comparator(const void *p, const void *q);
// Memory error test:
static void memoryError (const void * input);

//------------- CONSTRUCTOR/ DESTRUCTOR -------------
static void initialiseGame (DraculaView dv);							// Initialise an empty game to fill in
static Player parseMove (DraculaView dv, char *string);				// Parse the move string
static void hunterMove(DraculaView dv, char *string, Player hunter);	// Apply hunter move from parsed string
static void draculaMove(DraculaView dv, char * string);				// Apply dracula move from parsed string
static int PlaceIdToAsciiDoubleBack (PlaceId place);				// Convert a doubleback placeid to doubleback value
//static PlaceId asciiToPlaceIdDoubleBack (char * c);
static void trapLocationAppend(DraculaView dv, PlaceId location);		// Add a trap location
static void trapLocationRemove(DraculaView dv, PlaceId location);		// Remove a trap location
static void checkHunterHealth(DraculaView dv,Player hunter);			// Check the health of a hunter, sends them to hospital if needed

PlaceId *dvGetLastLocations(DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree);
static PlayerData initialisePlayer(DraculaView dv, Player player);
PlaceId *dvGetMoveHistory(DraculaView dv, Player player,
                          int *numReturnedMoves, bool *canFree);
static int PlaceIdToAsciiDoubleBack (PlaceId place);				// Convert a doubleback placeid to doubleback value
static void trapLocationAppend(DraculaView dv, PlaceId location);		// Add a trap location
static void sortPlaces(PlaceId *places, int numPlaces);
static void draculaLocationHistoryAppend(DraculaView dv, PlaceId curr_place);
static int placeIdCmp(const void *ptr1, const void *ptr2);
PlaceId *dvGetLocationHistory(DraculaView dv, Player player,
                              int *numReturnedLocs, bool *canFree);

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
	// free player structs
	for (int i = 0; i < NUM_PLAYERS; i++)
		free(dv->allPlayers[i]);

	// free map
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
	//the immature vampire location
	return dv->vampire;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
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
			    hunterMove(dv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;

			case 'S':
			    printf("it is Dr S\n");
			    hunterMove(dv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_VAN_HELSING;
			    break;

			case 'H':
			    printf("it is VH\n");
			    hunterMove(dv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_MINA_HARKER;
			    break;

			case 'M':
			   printf("it is Mina\n");
			    hunterMove(dv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;

			case 'D':
			    printf("it is Drac\n");
			    draculaMove(dv, string);
			    curr_player = PLAYER_LORD_GODALMING;
			    break;
		}

	return curr_player;
}


// HUNTER MOVE:
// -- Input:
// -- Output:
// Author: Cindy (Tara edited)
static void hunterMove(DraculaView dv, char *string, Player hunter) {

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

 	if (curr_place == NOWHERE) printf("Error: Place not found...\n");

    // Append history and current location:
    HUNTER -> currentLocation = curr_place;
	/*	Removed this: this is taken care of by pastPlays string "GGEVD"; This bit should probably be in the hunt implementation
    // If Dracula is currently in same city, run...

    PlaceId Drac_pos = GvGetVampireLocation(gv);
    if (Drac_pos == curr_place) {
        DRACULA->health -= LIFE_LOSS_HUNTER_ENCOUNTER;
        gv->allPlayers[hunter]->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
    }
	*/
	// Parsing through characters after location iD
	char *c;

	for (int i = 3; i < strlen(string); i++) {
		c = &string[i];
		switch(*c){
			// It's a trap!
			case ITS_A_TRAP:
				printf("Hunter encountered trap!\n");
				dv->allPlayers[hunter]->health -= LIFE_LOSS_TRAP_ENCOUNTER;
				checkHunterHealth(dv, hunter);
				//remove trap
				trapLocationRemove(dv, curr_place);
				break;

			// Immature Vampire encounter
			case CLOSE_ENCOUNTERS_OF_THE_VTH_KIND:
				printf("Hunter encountered immature vampire!\n");
				dv->vampire = NOWHERE;
				break;

			// Dracula encounter
			case 'D':
				printf("Hunter encountered dracula!\n");
				//i think this part is needed? judging by test line 238 of testDraculaView, makes it seem like this is whats meant to happen
				//dracula must be in this city!
				printf("current health is %d\n", dv->allPlayers[hunter]->health);
				dv->allPlayers[hunter]->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
				checkHunterHealth(dv, hunter);
				DRACULA->health -= LIFE_LOSS_HUNTER_ENCOUNTER;
				printf("hunter health is now %d\n", dv->allPlayers[hunter]->health);
				draculaLocationHistoryAppend(dv, curr_place);
				break;

			// other characters include trailing '.'
			case '.':
				break;
		}
	}
    return;
}

// DRACULA MOVE:
// -- Input:
// -- Output:
// Author: Cindy
static void draculaMove(DraculaView dv, char *string) {

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


    // Append history and current location:

	//Unknown city move
	if (strcmp(city, "C?") == 0) {
		printf("unknown city move\n");
		DRACULA -> currentLocation = CITY_UNKNOWN;
	}

	// Unknown sea move
	else if (strcmp(city,"S?") == 0) {
		printf("unknown sea move\n");
		//DRACULA->health -= (LIFE_LOSS_SEA);
		DRACULA -> currentLocation = SEA_UNKNOWN;
	}

	// Hide move ->stays in the city for another round
	else if (strcmp(city,"HI") == 0) {
		printf("hide move\n");
		DRACULA->lastHidden = dv->roundNumber;
		DRACULA -> currentLocation = curr_place;

	}
	// Double back move
	else if (strncmp(city,"D",1) == 0) {
		//convert ascii number to int

		//convert int to #define
		draculaLocationHistoryAppend(dv, curr_place);
		//if double back is to sea, remove health
		//retrieve dracula's trail
		int numReturnedLocs = 0;

		bool canFree = false;
		PlaceId * trail = dvGetLastLocations(dv, PLAYER_DRACULA , TRAIL_SIZE,
									&numReturnedLocs, &canFree);
		PlaceId returnPlace = trail[PlaceIdToAsciiDoubleBack(curr_place)-1];
		printf("returned place is %s\n",placeIdToName(returnPlace));
		draculaLocationHistoryAppend(dv, returnPlace);
		//if(trail[PlaceIdToAsciiDoubleBack(curr_place)-1] == SEA_UNKNOWN) DRACULA->health -= (LIFE_LOSS_SEA);
		DRACULA -> currentLocation = curr_place;
		DRACULA->lastDoubleback = dv->roundNumber;
	}

	// Teleports to castle dracula
	else if (curr_place == TELEPORT) {
		printf("Drac teleported\n");
		DRACULA -> currentLocation = TELEPORT;

	}

	else if (curr_place == CASTLE_DRACULA){
		printf("Drac is at home\n");
    	DRACULA -> currentLocation = curr_place;
	}
	//Location move that was revealed (ie all other cases)
	else {
    	DRACULA -> currentLocation = curr_place;
	}

	// Parsing through characters after location id
	char *c;
	int i = 3;

	while ( i < strlen(string)) {
		c = &string[i];

		// if there are extra characters indicating trap or immature vampire
			// trap left the trail due to age
			if (*c == 'M') {
				printf("Trap has left trail!\n");
				int numReturnedLocs = 0;
				bool canFree = false;
				PlaceId *trail = dvGetLastLocations(dv, PLAYER_DRACULA , TRAIL_SIZE,
				                            &numReturnedLocs, &canFree);
				PlaceId brokenTrap = trail[0];
				trapLocationRemove(dv, brokenTrap);
				//remove from trapLocations
				//trap leaves trail (from the move that left trail?)
				free(trail);
			}

			// immature vampire has matured
			if (*c == CLOSE_ENCOUNTERS_OF_THE_VTH_KIND) {
				if( i == 5) {
					//vampire matures
					printf("Vampire matured! -%d game points\n", SCORE_LOSS_VAMPIRE_MATURES);
					dv->vampire = NOWHERE;
					dv->score -= SCORE_LOSS_VAMPIRE_MATURES;
				}
				else {
					printf("vampire placed");
					dv->vampire = curr_place;
				}
			}

			// Trap placed
			if (*c == ITS_A_TRAP) trapLocationAppend(dv, curr_place);
			// Immature vampire placed


		i++;
	}
	// game score decreases each time drac finishes turn
	printf("game score decreased by drac turn\n");
    dv->score -= SCORE_LOSS_DRACULA_TURN;
    free(city);
    return;

}

// TRAP LOCATION REMOVE: Removes a specified location from trap location array
// -- INPUT: GameView, PlaceId
// -- OUTPUT: void
static void trapLocationRemove(DraculaView dv, PlaceId location) {
	printf("trap removed!\n");
	int i = 0;
	//find index of trap location (sorted largest to smallest PlaceId value)
	while (i <= dv->trapLocationsIndex) {
		if(dv->trapLocations[i] == location) break;
		i++;
	}
	//remove from location by setting to nowhere
	dv->trapLocations[i] = NOWHERE;

	//shuffle array so smallest numbers are at end (NOWHERE is smallest PlaceId value)
	//index has shrunk so NOWHERE will fall off end of array
	sortPlaces(dv->trapLocations, dv->trapLocationsIndex+1);
	dv->trapLocationsIndex--;
	return;
}


// HUNTER LOCATION HISTORY APPEND: Appends input placeid to a player's
// locationhistory, updates current location and index.
// -- INPUT: GameView, Player, PlaceId to append
// -- OUTPUT: void
static void hunterLocationHistoryAppend(DraculaView dv, Player hunter, PlaceId location) {
	int index = HUNTER->currentLocationIndex;
	// ensure the array is large enough, then append
	if (index < MAX_LOC_HISTORY_SIZE) {
		HUNTER->locationHistory[index + 1] = location;
		HUNTER->moveHistory[index + 1] = location;
		HUNTER->currentLocation = location;
		HUNTER->currentLocationIndex++;
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
// -- INPUT: GameView, PlaceId to append
// -- OUTPUT: void
static void draculaLocationHistoryAppend(DraculaView dv, PlaceId location) {

	int index = DRACULA->currentLocationIndex;
	printf("appending location %s\n",placeIdToName(location));
	//if dracula is at sea, he loses health
	PlaceId actualLocation = NOWHERE;
	int numReturnedLocs = 0;
	bool canFree = false;
	PlaceId *trail = dvGetLastLocations(dv, PLAYER_DRACULA , TRAIL_SIZE,
								&numReturnedLocs, &canFree);
	if (trail == NULL) printf("nothing in trail!\n");
	// ensure the array is large enough, then append
	if (index < MAX_LOC_HISTORY_SIZE) {
		printf("appended to move history\n");
		DRACULA->moveHistory[index + 1] = location;

		//dracula's location is the same as previous
		if(location == HIDE && trail != NULL){
			actualLocation = trail[numReturnedLocs-1];
			printf("dracula is hiding at %s\n",placeIdToName(actualLocation));
			DRACULA->locationHistory[index + 1] = actualLocation;
			DRACULA->currentLocation = actualLocation;
		}
		//go back to x previous locations
		else if (trail != NULL && location >= DOUBLE_BACK_1 && location <=DOUBLE_BACK_5){

			actualLocation = trail[numReturnedLocs-PlaceIdToAsciiDoubleBack(location)];
			printf("dracula is doubled back at %s\n",placeIdToName(actualLocation));
			DRACULA->locationHistory[index + 1] = actualLocation;
			DRACULA->currentLocation = actualLocation;
		}
		//Draculas location was not hidden/double back
		else {
			DRACULA->locationHistory[index + 1] = location;
			DRACULA->currentLocation = location;
		}

		if(placeIdToType(location) == SEA || placeIdToType(actualLocation) == SEA) {
			printf("dracula is getting seasick!\n");
			DRACULA->health -= (LIFE_LOSS_SEA);
		}
		if(location == TELEPORT || location == CASTLE_DRACULA || actualLocation == CASTLE_DRACULA) 		{
			printf("Drac hp +");
			DRACULA->health += LIFE_GAIN_CASTLE_DRACULA;
		}



		DRACULA->currentLocationIndex++;
	}
	// otherwise print error and exit
	else {
		fprintf(stderr, "%s", "location history indexed out of bounds, aborting");
		exit(EXIT_FAILURE);
	}
	return;
}



// CHECK HUNTER HEALTH: Checks if a hunter has died, if so, moves them to hospital
// -- INPUT: GameView, Player
// -- OUTPUT: void
static void checkHunterHealth(DraculaView dv,Player hunter){
	if(dv->allPlayers[hunter]->health <= 0) {
		printf("hunter died!\n");
		dv->allPlayers[hunter]->health = 0;
		dv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
		hunterLocationHistoryAppend(dv, hunter, HOSPITAL_PLACE);
		// todo need to update hunter HP?
	}
	return;
}

// INITIALISE PLAYER: Initialises a player to defaults, assigns memory
// -- INPUT: GameView, Player
// -- OUTPUT: void
static PlayerData initialisePlayer(DraculaView dv, Player player) {

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

// GET MOVE HISTORY: Returns last n locations of a player in dynamically allocated
// array, indicates the number of locs and that the return array can be freed
// -- INPUT: dv, player, number of locs to return, pointer to an int storing
// the number of returned locs, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *dvGetLastLocations(DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	printf("getting last locations\n");
	// unless asking for more locations than have happened, return numlocs
	if (dv->allPlayers[player]->currentLocationIndex >= numLocs) {
		// can free as returning a separate array
		*canFree = true;
		*numReturnedLocs = numLocs;
		// allocate space for return array
		PlaceId *lastNLocs = malloc(sizeof(PlaceId) *numLocs);
		memoryError(lastNLocs);
		// loop throught location history, adding to lastn in chronological order
		for (int i = 0; i < numLocs; i++) {
			lastNLocs[i] = dv->allPlayers[player]->locationHistory[dv->allPlayers[player]->currentLocationIndex - numLocs + i];
		}
		return lastNLocs;
	}
	// if asking for too many locations, only return all that exist
	return dvGetLocationHistory(dv, player, numReturnedLocs, canFree);
}

// GET MOVE HISTORY: Returns complete location history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed. Returns UNKNOWN places where relevant for dracula.
// -- INPUT: dv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *dvGetLocationHistory(DraculaView dv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// can free as allocating new array


	// pass number of moves

	int index = dv->allPlayers[player]->currentLocationIndex;
	printf("index is %d\n", index);
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
			allLocs[i] = dv->allPlayers[player]->locationHistory[i];
			/* else if not visible
				// return unknown sea
				if (placeIdToType(dv->allPlayers[player]->currentLocation) == SEA)
					return SEA_UNKNOWN;
				// or return unknown city
				else
					return CITY_UNKNOWN;*/
		}
		return allLocs;
	}
	return dvGetMoveHistory(dv, player, numReturnedLocs, canFree);
}

// GET MOVE HISTORY: Returns complete move history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed
// -- INPUT: gv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *dvGetMoveHistory(DraculaView dv, Player player,
                          int *numReturnedMoves, bool *canFree)
{

	// can't free as is returning directly from data struct
	*canFree = false;
	// pass number of moves
	*numReturnedMoves = dv->allPlayers[player]->currentLocationIndex+1;
	return dv->allPlayers[player]->moveHistory;

}

// PLACE ID TO ASCII DOUBLEBACK: Converts doubleback placeids to the enumed
// doubleback values
// -- INPUT: PlaceId
// -- OUTPUT: void
static int PlaceIdToAsciiDoubleBack (PlaceId place) {
	//Does the same thing as IF block; This is more compact codewise but probably more confusing
	//return 1 + (place - DOUBLE_BACK_1);
	if(place == DOUBLE_BACK_1) return 1;
	if(place == DOUBLE_BACK_2) return 2;
	if(place == DOUBLE_BACK_3) return 3;
	if(place == DOUBLE_BACK_4) return 4;
	if(place == DOUBLE_BACK_5) return 5;
	return NOWHERE;
}

// TRAP LOCATION APPEND: Appends input to the trap location array, updates index,
//array is sorted largest to smallest PlaceId value for easy manipulation
// -- INPUT: GameView, PlaceId to append
// -- OUTPUT: void
static void trapLocationAppend(DraculaView dv, PlaceId location) {
	printf("trap added!\n");
	int index = dv->trapLocationsIndex;
	if (index < MAX_LOC_HISTORY_SIZE) {
		dv->trapLocations[index + 1] = location;
		dv->trapLocationsIndex++;
	}
	sortPlaces(dv->trapLocations, dv->trapLocationsIndex);
	return;
}

//Author: testUtils.c; Edited: Cindy
static void sortPlaces(PlaceId *places, int numPlaces) {
	qsort(places, (size_t)numPlaces, sizeof(PlaceId), placeIdCmp);
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