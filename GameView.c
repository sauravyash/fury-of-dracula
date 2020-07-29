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
#define MAX_LOC_HISTORY_SIZE (GAME_START_SCORE * 2 * 4)
//DOUBLE CHECK THIS: max number of turns * most possible avg moves per turn (rounded up)  * bytes per move stored
#define OUR_ARBITRARY_ARRAY_SIZE 10

// some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'

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
	PlaceId currentLocation;						  	// current location
	int currentLocationIndex;						  	// index of current location in locationHistory
	// -------------------------- BLOOD BOIS ONLY BEYOND THIS POINT --------------------------
	// do we even need these?
	int lastHidden;									  	// round in which drac last hid
	int lastDoubleback;									// round in which drac last doubled back
	// bool isVisible									// something to indicate whether hunters know drac curr location
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
// Qsort comparator:
int comparator(const void *p, const void *q);
// Memory error test:
static void memoryError (const void * input);

//------------- CONSTRUCTOR/ DESTRUCTOR -------------
static void initialiseGame (GameView gv);							// Initialise an empty game to fill in
static Player parseMove (GameView gv, char *string);				// Parse the move string
static void hunterMove(GameView gv, char *string, Player hunter);	// Apply hunter move from parsed string
static void draculaMove(GameView gv, char * string);				// Apply dracula move from parsed string
static int PlaceIdToAsciiDoubleBack (PlaceId place);				// Convert a doubleback placeid to doubleback value
//static PlaceId asciiToPlaceIdDoubleBack (char * c);
static void trapLocationAppend(GameView gv, PlaceId location);		// Add a trap location
static void trapLocationRemove(GameView gv, PlaceId location);		// Remove a trap location
static void checkHunterHealth(GameView gv,Player hunter);			// Check the health of a hunter, sends them to hospital if needed

// ****************************************


// MEMORY ERROR: Helper function to check correct memory allocation. Exits if
// memory was not correctly allocated
// -- INPUT: pointer to a malloced object
// -- OUTPUT: void, but prints error message and exits code 1
static void memoryError (const void * input) {
	if (input == NULL) {
		fprintf(stderr, "Couldn't allocate Memory!\n");
		exit(EXIT_FAILURE);
	}
}

// COMPARATOR: Compare the order of the elements pointed to by *p and *q. Returns:
//	<0 If the element pointed by p goes before the element pointed by q,
//	0  If the element pointed by p is equivalent to the element pointed by q,
//	>0 If the element pointed by p goes after the element pointed by q.
// -- INPUT: two pointers, *p and *q
// -- OUTPUT: int
int comparator(const void *p, const void *q) {
    // Get the values at given addresses
    char * l = (char *)p;
    char * r = (char *)q;
 	return (strcmp(l,r));
}

// TRAP LOCATION APPEND: Appends input to the trap location array, updates
// current location and index.
// -- INPUT: GameView, PlaceId to append
// -- OUTPUT: void
static void trapLocationAppend(GameView gv, PlaceId location) {
	int index = gv->trapLocationsIndex;
	if (index < MAX_LOC_HISTORY_SIZE) {
		gv->trapLocations[index + 1] = location;
		gv->trapLocationsIndex++;
	}
	return;
}
// static PlaceId asciiToPlaceIdDoubleBack (char * c) {
// 	int doubleBack = atoi(c);
// 	//Does the same thing as IF block; This is more compact codewise but probably more confusing
// 	//return DOUBLE_BACK_1 + (doubleBack - 1);
// 	if(doubleBack == 1) return DOUBLE_BACK_1;
// 	if(doubleBack == 2) return DOUBLE_BACK_2;
// 	if(doubleBack == 3) return DOUBLE_BACK_3;
// 	if(doubleBack == 4) return DOUBLE_BACK_4;
// 	if(doubleBack == 5) return DOUBLE_BACK_5;
// 	return NOWHERE;
// }

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

// TRAP LOCATION REMOVE: Removes a specified location from trap location array
// -- INPUT: GameView, PlaceId
// -- OUTPUT: void
static void trapLocationRemove(GameView gv, PlaceId location) {
	// todo
	//find index of trap location
	//remove from location
	//shuffle array
	return;
}

// HUNTER LOCATION HISTORY APPEND: Appends input placeid to a player's
// locationhistory, updates current location and index.
// -- INPUT: GameView, Player, PlaceId to append
// -- OUTPUT: void
static void hunterLocationHistoryAppend(GameView gv, Player hunter, PlaceId location) {
	int index = HUNTER->currentLocationIndex;
	// ensure the array is large enough, then append
	if (index < MAX_LOC_HISTORY_SIZE) {
		HUNTER->locationHistory[index + 1] = location;
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

// HUNTER LOCATION HISTORY APPEND: Appends input placeid to a drac's
// locationhistory, updates current location and index. If the location is
// at sea, reduce drac's health.
// -- INPUT: GameView, PlaceId to append
// -- OUTPUT: void
static void draculaLocationHistoryAppend(GameView gv, PlaceId location) {
	int index = DRACULA->currentLocationIndex;
	printf("appending location %s\n",placeIdToName(location));
	//if dracula is at sea, he loses health
	if(placeIdToType(location) == SEA) {
		printf("dracula is getting seasick!\n");
		DRACULA->health -= (LIFE_LOSS_SEA);
	}
	// ensure the array is large enough, then append
	if (index < MAX_LOC_HISTORY_SIZE) {
		DRACULA->locationHistory[index + 1] = location;
		DRACULA->currentLocation = location;
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
static void checkHunterHealth(GameView gv,Player hunter){
	if(gv->allPlayers[hunter]->health <= 0) {
		printf("hunter died!\n");
		gv->allPlayers[hunter]->health = 0;
		gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
		hunterLocationHistoryAppend(gv, hunter, HOSPITAL_PLACE);
		// todo need to update hunter HP?
	}
	return;
}

// INITIALISE PLAYER: Initialises a player to defaults, assigns memory
// -- INPUT: GameView, Player
// -- OUTPUT: void
static void initialisePlayer(GameView gv, Player player) {
	if (player == PLAYER_DRACULA) {
		DRACULA = malloc(sizeof(PlayerData));
		memoryError (DRACULA);
		DRACULA -> health = GAME_START_BLOOD_POINTS;
		DRACULA -> currentLocation = NOWHERE;
		DRACULA -> currentLocationIndex = -1;
		DRACULA -> locationHistory[0] = '\0';
	}
	else {
		PLAYER = malloc(sizeof(PlayerData));
		memoryError (LORD_GODALMING);
		PLAYER -> health = 	GAME_START_HUNTER_LIFE_POINTS;
		PLAYER -> currentLocation = NOWHERE;
		PLAYER -> currentLocationIndex = -1;
		PLAYER -> locationHistory[0] = '\0';
	}
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
	for (int player = 0; player < NUM_PLAYERS; player++) {
		initialisePlayer(gv, player);
	}

    // No trap locations at start of game, therefore no array yet..
	// gv->trapLocations = NULL;
	gv->vampire = NOWHERE;

	// Nothing else to do for map- read Map.c --> the functions take care of
	// adding all connections.
	// gv->map = MapNew();
	return;

}

// PARSE MOVE: interprets a single move, calls hunter/draculaMove, updates curr_player
// -- Input: GameView, move string
// -- Output: current Player
// Author: Cindy (Tara edited)
static Player parseMove (GameView gv, char *string){
	
	char *c = string;
	Player curr_player;

	// figure out whose move it was
	switch(*c){
			case 'G':
			    printf("it is Lord G\n");
			    hunterMove(gv, string, PLAYER_LORD_GODALMING);
			    curr_player = PLAYER_DR_SEWARD;
			    break;
			
			case 'S':
			    printf("it is Dr S\n");
			    hunterMove(gv, string, PLAYER_DR_SEWARD);
			    curr_player = PLAYER_VAN_HELSING;
			    break;
			
			case 'H':
			    printf("it is VH\n");
			    hunterMove(gv, string, PLAYER_VAN_HELSING);
			    curr_player = PLAYER_MINA_HARKER;
			    break;
			
			case 'M':
			   printf("it is Mina\n");
			    hunterMove(gv, string, PLAYER_MINA_HARKER);
			    curr_player = PLAYER_DRACULA;
			    break;
			
			case 'D':
			    printf("it is Drac\n");
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

 	if (curr_place == NOWHERE) printf("Error: Place not found...\n");

    // Append history and current location:
    hunterLocationHistoryAppend(gv, hunter, curr_place);
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
				gv->allPlayers[hunter]->health -= LIFE_LOSS_TRAP_ENCOUNTER;
				checkHunterHealth(gv, hunter);
				//remove trap
				trapLocationRemove(gv, curr_place);
				break;

			// Immature Vampire encounter
			case CLOSE_ENCOUNTERS_OF_THE_VTH_KIND:
				printf("Hunter encountered immature vampire!\n");
				gv->vampire = NOWHERE;
				break;

			// Dracula encounter	
			case 'D':
				printf("Hunter encountered dracula!\n");
				//i think this part is needed? judging by test line 238 of testGameView, makes it seem like this is whats meant to happen
				//dracula must be in this city!
				printf("current health is %d\n", gv->allPlayers[hunter]->health);
				gv->allPlayers[hunter]->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
				checkHunterHealth(gv, hunter);
				DRACULA->health -= LIFE_LOSS_HUNTER_ENCOUNTER;
				printf("hunter health is now %d\n", gv->allPlayers[hunter]->health);
				draculaLocationHistoryAppend(gv, curr_place);
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



    // Append history and current location:
	
	//Unknown city move
	if (strcmp(city, "C?") == 0) {
		printf("unknown city move\n");
		draculaLocationHistoryAppend(gv, CITY_UNKNOWN);
	
	}

	// Unknown sea move
	else if (strcmp(city,"S?") == 0) {
		printf("unknown sea move\n");
		//DRACULA->health -= (LIFE_LOSS_SEA);
		draculaLocationHistoryAppend(gv, SEA_UNKNOWN);
	
	}

	// Hide move ->stays in the city for another round
	else if (strcmp(city,"HI") == 0) {
		printf("hide move\n");
		DRACULA->lastHidden = gv->roundNumber;
		draculaLocationHistoryAppend(gv, curr_place);
	
	}
	// Double back move
	else if (strncmp(city,"D",1) == 0) {
		//convert ascii number to int

		//convert int to #define
		//draculaLocationHistoryAppend(gv, curr_place);
		//if double back is to sea, remove health
		//retrieve dracula's trail
		int numReturnedLocs = 0;
		//printf("i think something is wrong with trail[] here\n");
		bool canFree = false;
		PlaceId * trail = GvGetLastLocations(gv, PLAYER_DRACULA , TRAIL_SIZE,
									&numReturnedLocs, &canFree);
		PlaceId returnPlace = trail[PlaceIdToAsciiDoubleBack(curr_place)-1];
		draculaLocationHistoryAppend(gv, returnPlace);
		//if(trail[PlaceIdToAsciiDoubleBack(curr_place)-1] == SEA_UNKNOWN) DRACULA->health -= (LIFE_LOSS_SEA);
		DRACULA->lastDoubleback = gv->roundNumber;
	}

	// Teleports to castle dracula
	else if (strcmp(city,"TP") == 0) {
		draculaLocationHistoryAppend(gv, TELEPORT);
	}
	
	//Location move that was revealed (ie all other cases)
	else {
    	draculaLocationHistoryAppend(gv, curr_place);
	}

	// Parsing through characters after location id
	char *c;
	int i = 3;

	while ( i < strlen(string)) {
		c = &string[i];
		
		// if there are extra characters indicating trap or immature vampire
		if ( i > 4) {
			// trap left the trail due to age
			if (strcmp(c,"M") == 0) {
				printf("Trap has left trail!\n");
				int numReturnedLocs = 0;
				bool canFree = false;
				PlaceId * trail = GvGetLastLocations(gv, PLAYER_DR_SEWARD , TRAIL_SIZE,
				                            &numReturnedLocs, &canFree);
				PlaceId brokenTrap = trail[0];
				trapLocationRemove(gv, brokenTrap);
				//remove from trapLocations
				//trap leaves trail (from the move that left trail?)
				free(trail);
			}
			
			// immature vampire has matured
			else if (strcmp(c,"V") == 0) {
				//vampire matures
				printf("Vampire matured! -%d game points\n", SCORE_LOSS_VAMPIRE_MATURES);
				gv->vampire = NOWHERE;
				gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
			}
		}
		else {
			// Trap placed
			if (*c == ITS_A_TRAP) trapLocationAppend(gv, curr_place);
			// Immature vampire placed
			if (*c == CLOSE_ENCOUNTERS_OF_THE_VTH_KIND) gv->vampire = curr_place;
		}
		i++;
	}
	// game score decreases each time drac finishes turn
	printf("game score decreased by drac turn\n");
    gv->score -= SCORE_LOSS_DRACULA_TURN;
    return;

}
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor


// GV NEW: Allocate memory for new GameView
// -- INPUT: pastPlays string, messages
// -- OUTPUT: new GameView 
GameView GvNew(char *pastPlays, Message messages[])
{
	// Allocate memory for new GV
	GameView new = malloc(sizeof(*new));

	// Check if memory was allocated correctly
	memoryError(new);
	new->map = MapNew();
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

// GV FREE: Frees the current GameView
void GvFree(GameView gv)
{
	// free player structs
	for (int i = 0; i < NUM_PLAYERS; i++)
		free(gv->allPlayers[i]);

	// free map
	MapFree(gv->map);

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

// GET MOVE HISTORY: Returns complete move history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed
// -- INPUT: gv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{

	// can't free as is returning directly from data struct
	*canFree = false;
	// pass number of moves
	*numReturnedMoves = gv->allPlayers[player]->currentLocationIndex;
	return gv->allPlayers[player]->locationHistory;

}

// GET MOVE HISTORY: Returns last n moves of a player in dynamically allocated
// array, indicates the number of moves and that the return array can be freed
// -- INPUT: gv, player, number of moves to return, pointer to an int storing
// the number of returned moves, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
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

// GET MOVE HISTORY: Returns complete location history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed. Returns UNKNOWN places where relevant for dracula.
// -- INPUT: gv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
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

// GET MOVE HISTORY: Returns last n locations of a player in dynamically allocated
// array, indicates the number of locs and that the return array can be freed
// -- INPUT: gv, player, number of locs to return, pointer to an int storing
// the number of returned locs, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	printf("getting last locations\n");
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
            array[i] = list->p;
            places_added++;
        }

        // Increment.
        if (list->next == NULL) break;
        list = list->next;
    }

    return places_added;
}
