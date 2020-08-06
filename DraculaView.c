////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01    v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01    v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31    v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10    v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
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

#define PLAY_S_SIZE 7
#define LOCATION_ID_SIZE 2
#define MAX_LOC_HISTORY_SIZE (GAME_START_SCORE * 2 * 4 )
#define OUR_ARBITRARY_ARRAY_SIZE 10

// some puns just for fun
#define ITS_A_TRAP 'T'
#define CLOSE_ENCOUNTERS_OF_THE_VTH_KIND 'V'
#define MALFUNCTIONING_MACHIAVELLIAN_MACHINATIONS 'M'

// defines to make things more readable
#define  LORD_GODALMING  dv->allPlayers[PLAYER_LORD_GODALMING]
#define  DR_SEWARD       dv->allPlayers[PLAYER_DR_SEWARD]
#define  VAN_HELSING     dv->allPlayers[PLAYER_VAN_HELSING]
#define  MINA_HARKER     dv->allPlayers[PLAYER_MINA_HARKER]
#define  DRACULA         dv->allPlayers[PLAYER_DRACULA]
#define  PLAYER          dv->allPlayers[player]
#define  HUNTER          dv->allPlayers[hunter]

typedef struct playerData *PlayerData;

struct playerData {
    int health;                                         // current player health
    PlaceId locationHistory[MAX_LOC_HISTORY_SIZE];      // array of locations visited by player
    PlaceId moveHistory[MAX_LOC_HISTORY_SIZE];          //array of moves made by player (different to location history for drac)
    PlaceId currentLocation;                            // current location
    int currentLocationIndex;                           // index of current location in locationHistory
    // -------------------------- BLOOD BOIS ONLY BEYOND THIS POINT --------------------------
    int lastHidden;                                     // round in which drac last hid
    int lastDoubleback;                                 // round in which drac last doubled back
};

struct draculaView {
    Round roundNumber;                                  // current round
    int score;                                          // current game score
    PlayerData allPlayers[NUM_PLAYERS];                 // array of playerData structs
    Player currentPlayer;                               // looks like G always starts first? judging by the testfiles given G->S->H->M->D
    PlaceId trapLocations[MAX_LOC_HISTORY_SIZE];        // array of trap locations -- multiple traps in same place added seperately
    int trapLocationsIndex;                             // index of the most recently added trap location
    PlaceId vampire;                                    // only one vampire alive at any time
    Map map;                                            // graph thats been typedefed already
    int temp_round;
    PlaceId temp_place;
};

// PRIVATE FUNCTION DECLARATIONS
// ****************************************

//------------- GENERAL FUNCTIONS -------------
static int placeIdCmp(const void *ptr1, const void *ptr2);               // Qsort comparator
static void memoryError (const void * input);                            // Generalised memory error test
static void sortPlaces(PlaceId *places, int numPlaces);                  // Sorts array of PlaceIds from largest to smallest
static int PlaceIdToAsciiDoubleBack (PlaceId place);                     // Convert a doubleback placeid to doubleback value
static bool maxEncounters(DraculaView dv, PlaceId location);             //
//------------- CONSTRUCTOR/ DESTRUCTOR -------------
static void initialiseGame (DraculaView dv);                             // Initialise an empty game to fill in
static PlayerData initialisePlayer(DraculaView dv, Player player);       // Allocate the data for a player and set to default values

//------------- PARSING MOVES -------------
static Player parseMove (DraculaView dv, char *string);                  // Parse the move string
static void hunterMove(DraculaView dv, char *string, Player hunter);     // Apply hunter move from parsed string
static void draculaMove(DraculaView dv, char * string);                  // Apply dracula move from parsed string
static void trapLocationRemove(DraculaView dv, PlaceId location);        // Remove a trap location
static void trapLocationAppend(DraculaView dv, PlaceId location);        // Add a trap location

static void checkHunterHealth(DraculaView dv,Player hunter);             // Check the health of a hunter, sends them to hospital if needed

//------------- MOVES & HISTORIES -------------
PlaceId *DvGetLastLocations(DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree);        // Find players last n locations, return as array
PlaceId *dvGetMoveHistory(DraculaView dv, Player player,
                          int *numReturnedMoves, bool *canFree);         // Find the complete move history of a player, return as array
static void draculaLocationHistoryAppend(DraculaView dv, PlaceId curr_place);
PlaceId *dvGetLocationHistory(DraculaView dv, Player player,
                              int *numReturnedLocs, bool *canFree);
static PlaceId *DvGetReachableByType(DraculaView dv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs);
static int Find_Rails (Map map, PlaceId place, PlaceId from, PlaceId *array, int i);
static void hunterLocationHistoryAppend(DraculaView dv, Player hunter, PlaceId location);

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
    return;
}

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

// DV NEW: Initialises a new DraculaView
DraculaView DvNew(char *pastPlays, Message messages[]) {
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
        new->currentPlayer = parseMove(new, token);
        token = strtok(NULL, " ");
    }
    return new;
}

// DV FREE: Frees the current DraculaView
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
    memoryError(traps);
    for (int i = 0; i < *numTraps; i++) {
        traps[i] = dv->trapLocations[i];
    }
    return traps;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

// IS IN TRAIL: determines whether a given location is contained in dracula's trail
// -- INPUT: DraculaView, location
// -- OUTPUT: Bool
bool isInTrail(DraculaView dv, PlaceId location) {
    // only compare against as many moves as exist
    int max = (DRACULA->currentLocationIndex < 6 ? DRACULA->currentLocationIndex : 6);
    for (int i = 0; i < max; i++) {
        if (location == DRACULA->locationHistory[i]) return true;
    }
    return false;
}

// CAN HIDE: determines whether dracula can hide
// -- INPUT: DraculaView
// -- OUTPUT: Bool
bool canHide(DraculaView dv) {
    // check that he hasn't hidden in the last 5 turns, and that he isn't at sea
    if (((dv->roundNumber - DRACULA->lastHidden) > 5
        || DRACULA->lastHidden < 0)
        && placeIdToType(DRACULA->currentLocation) != SEA) return true;
    return false;
}

// CAN HIDE: determines whether dracula can doubleback
// -- INPUT: DraculaView
// -- OUTPUT: Bool
bool canDoubleBack(DraculaView dv) {
    // checks that he hasn't doubled back in the last 5 turns
    if ((dv->roundNumber - DRACULA->lastDoubleback) > 5
    || DRACULA->lastHidden < 0) return true;
    return false;
}

// GET VALID MOVES: Returns all valid moves that dracula can currently make
// returns dynamically allocated array of moves
PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves) {
    // load map
    Map map = dv->map;

    // create dynamically allocated array
    PlaceId *possibleMoves = malloc(sizeof(PlaceId));
    memoryError(possibleMoves);

//THIS DOESNT WORK IF DRACULA IS NOT ON MAP YET! SEG FAULTS NEEDS FIX
    // get connections from current location
    ConnList list = MapGetConnections(map, dv->allPlayers[PLAYER_DRACULA]->currentLocation);

    // iterate through connections
    int moveIndex = 0;

    while (moveIndex < NUM_REAL_PLACES) {
        // exclude locations in his trail
        if (!isInTrail(dv, list->p)) {
            // Extra conditions for drac:
            if (list->p == HOSPITAL_PLACE) {
                list = list->next;
                continue;
            }
            if (list->type == RAIL) {
                list = list->next;
                continue;
            }

            // If it is a road type.
            if (list->type == ROAD) {
                possibleMoves[moveIndex] = list->p;
                moveIndex++;
                possibleMoves = realloc(possibleMoves, (moveIndex + 1) * sizeof(PlaceId));
                memoryError(possibleMoves);
            }

            // If it is a boat type.
            else if (list->type == BOAT) {
                possibleMoves[moveIndex] = list->p;
                moveIndex++;
                possibleMoves = realloc(possibleMoves, (moveIndex + 1) * sizeof(PlaceId));
                memoryError(possibleMoves);
            }
        }
        // check whether he can doubleback to places further back than his last turn
        else if (canDoubleBack(dv)) {
            // checks location against trail
            int max = (DRACULA->currentLocationIndex < 6 ? DRACULA->currentLocationIndex : 6);
            for (int i = 1; i < max; i++) {
                if (list->p == DRACULA->locationHistory[DRACULA->currentLocationIndex - i]) {
                    // add appropriate doubleback move to list
                    possibleMoves[moveIndex] = DOUBLE_BACK_1 + i;
                    moveIndex++;
                    possibleMoves = realloc(possibleMoves, (moveIndex + 1) * sizeof(PlaceId));
                    memoryError(possibleMoves);
                }
            }
        }

        if (list->next == NULL) break;
        list = list->next;
    }
    // check if drac can take a hide move
    if (canHide(dv)) {
        possibleMoves[moveIndex] = HIDE;
        moveIndex++;
        possibleMoves = realloc(possibleMoves, (moveIndex + 1) * sizeof(PlaceId));
        memoryError(possibleMoves);
    }

    // check if he can doubleback to his last location (i.e. stay where he is)
    if (canDoubleBack(dv)) {
        possibleMoves[moveIndex] = DOUBLE_BACK_1;
        moveIndex++;
        possibleMoves = realloc(possibleMoves, (moveIndex + 1) * sizeof(PlaceId));
        memoryError(possibleMoves);
    }

    // Return values
    *numReturnedMoves = moveIndex;
    if(moveIndex == 0) {
        free(possibleMoves);
        return NULL;
    }
    return possibleMoves;
}

// WHERE CAN I GO: Lists all locations dracula can reach on his turn
// returns array of locations
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs) {
    PlaceId *locs = DvWhereCanIGoByType(dv, 1, 1, numReturnedLocs);
    return locs;
}

// WHERE CAN I GO: Lists all locations dracula can reach on his turn,
// with the option to specify type
// returns array of locations
PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs) {
    int numReturnedMoves = 0;
    PlaceId *moves = DvGetValidMoves(dv, &numReturnedMoves);
    PlaceId *locs = malloc(sizeof(PlaceId));
    memoryError(locs);
    int locsIndex = 0;

    // sort through all moves, rejecting duplicate locs from hide/doubleback
    for (int i = 0; i < numReturnedMoves; i++) {
        // if move is not a doubleback or hide add to possible locations
        if (moves[i] != HIDE && (moves[i] < DOUBLE_BACK_1 || moves[i] > DOUBLE_BACK_5)) {
            // only add roads if specified in call
            if (road && placeIdToType(moves[i]) == LAND) {
                locs[locsIndex] = moves[i];
                locsIndex++;
                locs = realloc(locs, (locsIndex + 1) * sizeof(PlaceId));
                memoryError(locs);
            }
            // only add boats if specified in call
            else if (boat && placeIdToType(moves[i]) == SEA) {
                locs[locsIndex] = moves[i];
                locsIndex++;
                locs = realloc(locs, (locsIndex + 1) * sizeof(PlaceId));
                memoryError(locs);
            }
        }
        // check whether he can doubleback to places further back than his last turn
        else if (canDoubleBack(dv)) {
            // checks location against trail
            int max = (DRACULA->currentLocationIndex < 6 ? DRACULA->currentLocationIndex : 6);
            for (int j = 1; j < max; j++) {
                if (moves[i] == DRACULA->locationHistory[DRACULA->currentLocationIndex - j]) {
                    // add appropriate doubleback move to list
                    locs[locsIndex] = DRACULA->locationHistory[DRACULA->currentLocationIndex - j];
                    locsIndex++;
                    locs = realloc(locs, (locsIndex + 1) * sizeof(PlaceId));
                    memoryError(locs);
                }
            }
        }
    }

    // check if drac can take a hide move
    if (canHide(dv)) {
        locs[locsIndex] = DvGetPlayerLocation(dv, PLAYER_DRACULA);
        locsIndex++;
        locs = realloc(locs, (locsIndex + 1) * sizeof(PlaceId));
        memoryError(locs);
    }

    // check if he can doubleback to his last location (i.e. stay where he is)
    else if (canDoubleBack(dv)) {
        locs[locsIndex] = DvGetPlayerLocation(dv, PLAYER_DRACULA);
        locsIndex++;
        locs = realloc(locs, (locsIndex + 1) * sizeof(PlaceId));
        memoryError(locs);
    }

    // Return values
    free(moves);
    *numReturnedLocs = locsIndex;
    if(locsIndex == 0) {
        free(locs);
        return NULL;
    }
    return locs;
}

// WHERE CAN THEY GO: list all possible locations that a player can reach
PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs) {
    // dracula case
    if (player == PLAYER_DRACULA) {
        return DvWhereCanIGo(dv, numReturnedLocs);
    }
    // else for hunters
    // Some stuff to handle round number
    Round round = dv->allPlayers[player]->currentLocationIndex + 1;
    if (round == -1) round = 0;
    if (player == dv->currentPlayer) round++;
    // other values
    PlaceId from = DvGetPlayerLocation(dv, player);
    //printf("Current hunter location is %s\n", placeIdToName(from));
    PlaceId *locs = NULL;
    int numLocs = -1;

    // Use function:
    locs = DvGetReachableByType(dv, player, round, from, 1, 1, 1, &numLocs);
    printf("\nLocations returned in DvWhereCanTheyGo:       ");
	for (int i = 0; i < numLocs; i ++) {
		printf("%s, \n", placeIdToName(locs[i]));
	}
    printf("\n");
    // Return values:
    *numReturnedLocs = numLocs;
    return locs;
}

// WHERE CAN THEY GO: list all possible locations that a player can reach,
// with the option to sort by type
PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs) {
    // Set values:
    // some stuff to make sure round is ok
    Round round = dv->allPlayers[player]->currentLocationIndex + 1;
    if (round == -1) round = 0;
    if (player == dv->currentPlayer) round++;
    // other values
    PlaceId from = DvGetPlayerLocation(dv, player);
    PlaceId *locs = NULL;
    int numLocs = -1;

    // Use function:
    locs = DvGetReachableByType(dv, player, round, from, road, rail, boat, numReturnedLocs);

    // Return values:
    *numReturnedLocs = numLocs;
    return locs;
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
    for (int i = 0; i < NUM_PLAYERS; i++){
        dv->allPlayers[i] = initialisePlayer(dv, i);
    }

    dv->trapLocationsIndex = -1;
    dv->vampire = NOWHERE;

    return;
}

// PARSE MOVE: interprets a single move, calls hunter/draculaMove, updates curr_player
// -- Input: DraculaVie, move string
// -- Output: current Player
static Player parseMove (DraculaView dv, char *string) {

    char *c = string;
    Player curr_player;

    // figure out whose move it was
    switch(*c){
            case 'G':
                hunterMove(dv, string, PLAYER_LORD_GODALMING);
                curr_player = PLAYER_DR_SEWARD;
                break;

            case 'S':
                hunterMove(dv, string, PLAYER_DR_SEWARD);
                curr_player = PLAYER_VAN_HELSING;
                break;

            case 'H':
                hunterMove(dv, string, PLAYER_VAN_HELSING);
                curr_player = PLAYER_MINA_HARKER;
                break;

            case 'M':
                hunterMove(dv, string, PLAYER_MINA_HARKER);
                curr_player = PLAYER_DRACULA;
                break;

            case 'D':
                draculaMove(dv, string);
                curr_player = PLAYER_LORD_GODALMING;
                break;
        }

    return curr_player;
}


// HUNTER MOVE: Reads through hunter's string to determine actions taken
// -- Input: DraculaView, pastPlays string, hunter in play
// -- Output: void
static void hunterMove(DraculaView dv, char *string, Player hunter) {

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
     //printf("they are in %s\n", city);
    // Append history and current location:
    hunterLocationHistoryAppend(dv, hunter, curr_place);

    // Parsing through characters after location to determine actions
    char *c;
    for (int i = 3; i < strlen(string); i++) {
        c = &string[i];
        switch(*c){
            // It's a trap!
            case ITS_A_TRAP:
                dv->allPlayers[hunter]->health -= LIFE_LOSS_TRAP_ENCOUNTER;
                checkHunterHealth(dv, hunter);
                //remove trap
                trapLocationRemove(dv, curr_place);
                break;

            // Immature Vampire encounter
            case CLOSE_ENCOUNTERS_OF_THE_VTH_KIND:
                dv->vampire = NOWHERE;
                break;

            // Dracula encounter
            case 'D':
                dv->allPlayers[hunter]->health -= LIFE_LOSS_DRACULA_ENCOUNTER;
                checkHunterHealth(dv, hunter);
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

// DRACULA MOVE:
// -- Input: DraculaView, movestring
// -- Output: void
static void draculaMove(DraculaView dv, char *string) {

    // String must be of valid size
    assert (strlen(string) > LOCATION_ID_SIZE);

    // Store locationID into city[]:
    char *city = malloc((LOCATION_ID_SIZE + 1)*sizeof(char));
    memoryError(city);
    city[0] = string[1];
    city[1] = string[2];
    city[2] = '\0';

    // Compare and find city by abbreviation:
    // careful when doublebacks, this turns into a doubleback code
    PlaceId curr_place = placeAbbrevToId(city);
    
    // hide is last city
    if (curr_place == HIDE) {
        curr_place = DRACULA->locationHistory[DRACULA->currentLocationIndex];
    }

    //Unknown city move
    if (strcmp(city, "C?") == 0) {
        draculaLocationHistoryAppend(dv, CITY_UNKNOWN);
    }
    // Unknown sea move
    else if (strcmp(city,"S?") == 0) {
        //DRACULA->health -= (LIFE_LOSS_SEA);
        draculaLocationHistoryAppend(dv, SEA_UNKNOWN);
    }

    // all this assumes that the string is correct i.e. move is legal
    // Hide move ->stays in the city for another round
    else if (strcmp(city,"HI") == 0) {
        DRACULA->lastHidden = dv->roundNumber;
        draculaLocationHistoryAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex]);
    }
    // Double back move
    else if (strcmp(city,"D1") == 0) {
        draculaLocationHistoryAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex]);  
        DRACULA->lastDoubleback = dv->roundNumber;
    }
    // Double back move
    else if (strcmp(city,"D2") == 0) {
        draculaLocationHistoryAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex - 1]);
        DRACULA->lastDoubleback = dv->roundNumber;
    }
    // Double back move
    else if (strcmp(city,"D3") == 0) {
        draculaLocationHistoryAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex - 2]);
        DRACULA->lastDoubleback = dv->roundNumber;
    }
    // Double back move
    else if (strcmp(city,"D4") == 0) {
        draculaLocationHistoryAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex - 3]);
        DRACULA->lastDoubleback = dv->roundNumber;
    }
    // Double back move
    else if (strcmp(city,"D5") == 0) {
        draculaLocationHistoryAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex - 4]);
        DRACULA->lastDoubleback = dv->roundNumber;
    }


    //Location move that was revealed (ie all other cases)
    else {
        draculaLocationHistoryAppend(dv, curr_place);
    }

    // Parsing through characters after location id
    char *c;
    int i = 3;
    while ( i < strlen(string)) {
        c = &string[i];
        // if there are extra characters indicating trap or immature vampire
            // trap left the trail due to age
            if (*c == MALFUNCTIONING_MACHIAVELLIAN_MACHINATIONS) {
                int numReturnedLocs = 0;
                bool canFree = false;
                //retrieve draculas trail
                PlaceId *trail = DvGetLastLocations(dv, PLAYER_DRACULA , TRAIL_SIZE,
                                            &numReturnedLocs, &canFree);
                //remove the oldest trap in trail from trapLocations
                PlaceId brokenTrap = trail[0];
                trapLocationRemove(dv, brokenTrap);
                free(trail);
            }


            if (*c == CLOSE_ENCOUNTERS_OF_THE_VTH_KIND) {
                // immature vampire has matured
                if( i == 5) {
                    //check that vampire exists
                    if (dv->vampire != NOWHERE) {
                        dv->vampire = NOWHERE;
                        dv->score -= SCORE_LOSS_VAMPIRE_MATURES;
                    }
                }
                //immature vampire placed
                else {
                    if(maxEncounters(dv,curr_place) == false) dv->vampire = curr_place;
                }
            }

            // Trap placed
            if (*c == ITS_A_TRAP) {
                //PlaceId lastLoc = DRACULA->locationHistory[DRACULA->currentLocationIndex];
                //trapLocationAppend(dv, lastLoc);
                if(maxEncounters(dv, curr_place) == false)
                    trapLocationAppend(dv, DRACULA->locationHistory[DRACULA->currentLocationIndex]);
            }
        i++;
    }
    // game score decreases each time drac finishes turn
    dv->score -= SCORE_LOSS_DRACULA_TURN;
    free(city);
    return;
}
// TRAP LOCATION REMOVE: Removes a specified location from trap location array
// -- INPUT: DraculaView, PlaceId
// -- OUTPUT: void
static void trapLocationRemove(DraculaView dv, PlaceId location) {
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
// -- INPUT: DraculaView, Player, PlaceId to append
// -- OUTPUT: void
static void hunterLocationHistoryAppend(DraculaView dv, Player hunter, PlaceId location) {
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
// -- INPUT: DraculaView, PlaceId to append
// -- OUTPUT: void
static void draculaLocationHistoryAppend(DraculaView dv, PlaceId location) {

    int index = DRACULA->currentLocationIndex;
    PlaceId actualLocation = NOWHERE;
    int numReturnedLocs = 0;
    bool canFree = false;
    //Get Dracula's trail (last 6 moves)
    PlaceId *trail = DvGetLastLocations(dv, PLAYER_DRACULA , TRAIL_SIZE,
                                &numReturnedLocs, &canFree);
    // ensure the array is large enough, then append
    if (index < MAX_LOC_HISTORY_SIZE) {
        DRACULA->moveHistory[index + 1] = location;
        //HIDE: dracula's location is the same as previous
        if(location == HIDE && trail != NULL){
            actualLocation = trail[numReturnedLocs-1];
            DRACULA->locationHistory[index + 1] = actualLocation;
            DRACULA->currentLocation = actualLocation;
        }
        //go back to x previous locations
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

        if(placeIdToType(location) == SEA || placeIdToType(actualLocation) == SEA) {
            DRACULA->health -= (LIFE_LOSS_SEA);
        }
        if(location == TELEPORT || location == CASTLE_DRACULA || actualLocation == CASTLE_DRACULA)         {
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
// -- INPUT: DraculaView, Player
// -- OUTPUT: void
static void checkHunterHealth(DraculaView dv,Player hunter) {
    if(dv->allPlayers[hunter]->health <= 0) {
        dv->allPlayers[hunter]->health = 0;
        dv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
        HUNTER->currentLocation = HOSPITAL_PLACE;
        //hunterLocationHistoryAppend(gv, hunter, HOSPITAL_PLACE);
    }
    return;
}

// INITIALISE PLAYER: Initialises a player to defaults, assigns memory
// -- INPUT: DraculaView, Player
// -- OUTPUT: void
static PlayerData initialisePlayer(DraculaView dv, Player player) {

    PlayerData new = malloc(sizeof(* new));
    memoryError (new);

    new -> health = GAME_START_HUNTER_LIFE_POINTS;
    new -> currentLocation = NOWHERE;
    new -> currentLocationIndex = -1;
    new -> locationHistory[0] = '\0';
    new -> moveHistory[0] = '\0';
    new -> lastHidden = -100;
    new -> lastDoubleback = -1;
    if (player == PLAYER_DRACULA) {
        new -> health = GAME_START_BLOOD_POINTS;
    } else {
        new -> health =     GAME_START_HUNTER_LIFE_POINTS;
    }
    return new;
}

// GET MOVE HISTORY: Returns last n locations of a player in dynamically allocated
// array, indicates the number of locs and that the return array can be freed
// -- INPUT: dv, player, number of locs to return, pointer to an int storing
// the number of returned locs, pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *DvGetLastLocations(DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree) {
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
                              int *numReturnedLocs, bool *canFree) {
    // can free as allocating new array


    // pass number of moves

    int index = dv->allPlayers[player]->currentLocationIndex;
    *numReturnedLocs = index + 1;
    //if there are no moves in history, return NULL
    if(index < 0) return NULL;
    *canFree = true;

    if (player == PLAYER_DRACULA) {
        // todo make sure rounds when drac not visible returned as unknowns
        // depends on how visibility is parsed in dracmove
        PlaceId *allLocs = malloc(sizeof(PlaceId) * *numReturnedLocs);
        memoryError(allLocs);
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
    //free(allLocs);
    return dvGetMoveHistory(dv, player, numReturnedLocs, canFree);
}

// GET MOVE HISTORY: Returns complete move history of a player directly from
// locationHistory, indicates the number of moves and that the array cannot be
// freed
// -- INPUT: gv, player, pointer to an int storing the number of returned moves,
// pointer to a bool canFree
// -- OUTPUT: array of PlaceIds
PlaceId *dvGetMoveHistory(DraculaView dv, Player player,
                          int *numReturnedMoves, bool *canFree) {

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
    return 1 + (place - DOUBLE_BACK_1);
}

// TRAP LOCATION APPEND: Appends input to the trap location array, updates index,
//array is sorted largest to smallest PlaceId value for easy manipulation
// -- INPUT: DraculaView, PlaceId to append
// -- OUTPUT: void
static void trapLocationAppend(DraculaView dv, PlaceId location) {
    int index = dv->trapLocationsIndex;
    if (index < MAX_LOC_HISTORY_SIZE) {
        dv->trapLocations[index + 1] = location;
        dv->trapLocationsIndex++;
    }
    sortPlaces(dv->trapLocations, dv->trapLocationsIndex);
    return;
}

// COMPARATOR: Compare the order of the elements pointed to by *p and *q. Returns:
//    <0 If the element pointed by p goes before the element pointed by q,
//    0  If the element pointed by p is equivalent to the element pointed by q,
//    >0 If the element pointed by p goes after the element pointed by q.
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


static PlaceId *DvGetReachableByType(DraculaView dv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs) {

    // We need to access the map :)
    Map map = dv->map;

    // Make a temp round variable for use with finding shortest path...
    if (dv->temp_round > -1) round = dv->temp_round;
    if (dv->temp_place != NOWHERE) from = dv->temp_place;

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

// MAX ENCOUNTERS: Reads through drac's string to determine actions taken
// -- Input: GameView, Location of encounters to be checked
// -- Output: If maximum encounters for a city has been reached
static bool maxEncounters(DraculaView dv, PlaceId location) {
    int counter = 0;
    if (dv->vampire == location) counter++;
    for (int i = 0; i <= dv->trapLocationsIndex; i++){
        if(dv->trapLocations[i] == location) counter++;
    }
    return (counter >= 3);
}
