////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "Map.h"
#include "Places.h"

#include "utils.h"
#include "Queue.h"

#define SPAIN_MOVES BARCELONA,CADIZ,ALICANTE,GRANADA,SANTANDER,LISBON,MADRID,SARAGOSSA
#define SPAIN_SIZE 8
#define IRELAND_MOVES DUBLIN,GALWAY
#define IRELAND_SIZE 2
#define BRITAIN_MOVES EDINBURGH, LIVERPOOL, MANCHESTER, LONDON, SWANSEA, PLYMOUTH
#define BRITAIN_SIZE 6
#define ITALY_MOVES CAGLIARI,NAPLES,BARI,ROME,FLORENCE
#define ITALY_SIZE 5
#define MAGIC_NUMBER_SEED 15

typedef struct moveweight *MoveWeight;
struct moveweight {
    PlaceId location;
    double weight;
} ;

// Private Function Prototypes

static void memoryError (const void * input);
//PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves);
static PlaceId getRandomMove();
static void applyHunterFactor(MoveWeight *mw, int numPossibleMoves,
    PlaceId *possibleMovesHunter, int numPossibleMovesHunter);
static PlaceId weightMovesByLocation(DraculaView dv, MoveWeight *mw,
    int numPossibleLocations, PlaceId *possibleLocations);
static void sortMVbyWeight(MoveWeight *array, int arraySize);
static int  MVWeightcompare(const void *p, const void *q);
static int placeIdCmp(const void *p, const void *q);
static void sortPlaces(PlaceId *places, int numPlaces);
static PlaceId convertBestLocToMove(DraculaView dv, MoveWeight *mvArray,
    int MvArraySize, PlaceId bestMove, int index);

// Make new MoveWeight Struct item
// INPUT: void
// OUTPUT: Newly created item
MoveWeight MVNewNode(void){
	MoveWeight new = malloc(sizeof(*new));
	memoryError(new);
	new->location = NOWHERE;
	new->weight = -1;
	return new;
}

//Free a given array of type MoveWeight *
// INPUT: array to be freed, size of array
// OUTPUT: void
void freeArray (MoveWeight * array, int size) {
	for(int i = 0; i < size; i++){
		free(array[i]);
	}
	free(array);
    return;
}

// Create a array of MoveWeight items
// INPUT: Size of array
// OUTPUT: newly created array
MoveWeight * MvArrayNew(int size) {
	MoveWeight * arrayWeightedMoves = malloc(size * sizeof(MoveWeight));
	for (int i = 0; i < size; i++){
		arrayWeightedMoves[i] = MVNewNode();
	}
	return arrayWeightedMoves;
}

//Spawn Dracula in a random location that is not in
// Ireland, Spain, Cagliari, a place a hunter is in or can reach
// INPUT: DraculaView
// OUTPUT: random spawn location with constraints
PlaceId spawnDracula (DraculaView dv) {
	PlaceId unwantedPlaces[] = {SPAIN_MOVES,IRELAND_MOVES,BRITAIN_MOVES,ITALY_MOVES,
		ATHENS,HOSPITAL_PLACE,CASTLE_DRACULA};

    //initialise array to track unwanted locations
	PlaceId placeList[NUM_REAL_PLACES] = {0};
	int numReturnedLocs = -1;
	PlaceId * hunterLocations;
	Player hunter = PLAYER_LORD_GODALMING;

    //list all hunter locations and possible hunter locations as undesirable
	while (hunter < PLAYER_DRACULA) {
		hunterLocations= DvWhereCanTheyGo(dv, hunter, &numReturnedLocs);
		memoryError(hunterLocations);
		for (int i = 0; i < numReturnedLocs; i++){
			placeList[hunterLocations[i]] = 1;
		}
		hunter++;
	}

    //certain regions are undersirable
	for(int i = 0; i < SPAIN_SIZE + ITALY_SIZE + IRELAND_SIZE + BRITAIN_SIZE + 3; i++ ){
		placeList[unwantedPlaces[i]] = 1;
	}

    //sea locations are undeusriable
	for (int i = 0; i < NUM_REAL_PLACES; i++) {
		if(placeIdToType(i) == SEA) placeList[i] = 1;
	}

    //generate a random location that is not in the array
	srand (time(0));
	int i = 0;
	PlaceId location = rand() % NUM_REAL_PLACES;
	while (placeList[location] == 1) {
		i++;
		srand (time(0));
		location = (rand() + i )% NUM_REAL_PLACES;
        //prevent loop if all possible locations are undesirable
		if(i == NUM_REAL_PLACES) {
			//make it a sea move perhaps?
			break;
		}
	}
	return location;
}

// Debug Print function of locations and corresponding weights
// INPUT: MW array, size of array
void printMW (MoveWeight * mw, int size) {
	printf("\nMW is:   \n");
	for (int i = 0; i < size; i++) {
		printf("%s; %lf\n", placeIdToName(mw[i]->location), mw[i]->weight);
	}
	printf("\n");
}

//Decides drac move
// INPUT: draculaView
// OUTPUT: Changes "register best play" output string
void decideDraculaMove(DraculaView dv)
{
    //if it is the first round, call spawn drac function
    if (DvGetRound(dv) == 0) {
        registerBestPlay(placeIdToAbbrev(spawnDracula(dv)), "Happy Birthday To Me!");
        return;
    }

    int numPossibleLocations;
    PlaceId *possibleLocations =DvWhereCanIGo(dv, &numPossibleLocations);
    printf("\nPossible locations are:   ");
    for (int i = 0; i < numPossibleLocations; i ++) {
        printf("%s, ", placeIdToName(possibleLocations[i]));
    }

    //either there are no valid moves besides teleport, or drac hasnt made a move yet
    if (numPossibleLocations == 0) {
        //If it is not round 0 & there are no possible moves, dracula must teleport
        if (DvGetRound(dv) != 0) {
            registerBestPlay(placeIdToAbbrev(TELEPORT), "You'll never expect this!");
            return;
        //This is first round, just spawn dracula; already coveered  by first clase
        } else {
            registerBestPlay(placeIdToAbbrev(spawnDracula(dv)), "Happy Birthday To Me!");
            return;
        }
    }

    //sorts array alphabethically
    sortPlaces(possibleLocations,numPossibleLocations);
    int MvArraySize = numPossibleLocations;
    MoveWeight * MvArray = MvArrayNew(MvArraySize);
    PlaceId healMove = weightMovesByLocation(dv, MvArray, MvArraySize,possibleLocations);
    //sorts array in descending weight
    sortMVbyWeight(MvArray, MvArraySize);
    //highest weighted location is best choice
    PlaceId bestMove = MvArray[0]->location;

    //if healing move was chosen
    if (healMove != NOWHERE) {
        int found = false;
        //check if it is in the possible moves array
        for (int i = 0; i <= numPossibleLocations; i++) {
            if (healMove == possibleLocations[i]) {
                found = true;
                break;
            }
        }
        //only register the heal move as best move if it is a possible location
        if (found == true)  bestMove = healMove;
    }

    printMW(MvArray, MvArraySize);
    //convert location to a move (ie HI or Double backs)
    bestMove = convertBestLocToMove(dv, MvArray, MvArraySize, bestMove, 0);

    registerBestPlay(placeIdToAbbrev(bestMove), "doing my best ");
    freeArray(MvArray,numPossibleLocations);
    return;
}



// convert hide/doubleback locations to moves: HIDE/DOUBLEBACK_N
// INPUT: Dracula View, Array of Weighted Moves/locations , Size of Array,
// Location of best move, Index of best move in array (should always be one)
// OUPTUT: move in either City name format or as a hide/ double back
PlaceId convertBestLocToMove(DraculaView dv, MoveWeight *MvArray, int MvArraySize,
    PlaceId bestMove, int index) {
    int trailSize;
    bool canFree;
    PlaceId *trail = DvGetLocationHistory(dv, PLAYER_DRACULA, &trailSize, &canFree);
    for (int loc = trailSize; loc > trailSize - 5 && loc > -1; loc--) {
        // if the best move is in the trail
        if (bestMove == trail[loc]) {
            // if drac hasn't hidden in the last 5 turns and bestMove is his curr location
            if (canHide(dv) && loc == trailSize - 1) {
                bestMove = HIDE;
                break;
            }
            printf("Best DBack: %s \n", placeIdToName(bestMove));
            // if drac hasn't dbed in the last 5 turns
            if (canDoubleBack(dv)) {
                // chooses right db based on index of matched move in trail
                switch (trailSize - loc - 1) {
                    case 0:
                        bestMove = DOUBLE_BACK_1;
                        break;
                    case 1:
                        bestMove = DOUBLE_BACK_2;
                        break;
                    case 2:
                        bestMove = DOUBLE_BACK_3;
                        break;
                    case 3:
                        bestMove = DOUBLE_BACK_4;
                        break;
                    case 4:
                        bestMove = DOUBLE_BACK_5;
                        break;
                    default:
                        break;
                }
                printf("Best DBack: %s, trailSize: %d, loc: %d\n", placeIdToName(bestMove), trailSize, loc);
                break;
            }
            // if drac can't do the best move
            else {
                if (index < MvArraySize - 1) index++;
                else return bestMove; // none of the mvarray moves can be done, this case shouldn't happen
                return convertBestLocToMove(dv, MvArray, MvArraySize,
                    MvArray[index-1]->location, index);
            }
        }
    }
    if (canFree) free(trail);
    return bestMove;
}


//Checks if any hunter is present at a particula locations
// INPUT: Dracview, Location
// OUTPUT: true if hunter is in the location, false otherwise
// Potentially unused?
bool isHunterPresent (DraculaView dv, PlaceId location) {
	Player hunter = PLAYER_LORD_GODALMING;
	while (hunter < PLAYER_DRACULA) {
		if(DvGetPlayerLocation(dv,hunter) == location) return true;
		hunter++;
	}
	return false;
}

// Weight dracula's possible moves based on hunters & other environment factors
// INPUT: newly initialised moveweight array & size, list of possible locations
// OUPUT: NOWHERE if a heal move is not necessary, PlaceID location for the best
// heal move if required
PlaceId weightMovesByLocation(DraculaView dv, MoveWeight * mw, int mwSize,
    PlaceId *possibleLocations) {

	assert(mwSize > 0);
	//associate a certain weight to each location
	for (int i = 0; i < mwSize; i++) {
		mw[i]->location = possibleLocations[i];
		mw[i]->weight = 10;
		//if hunter is already at location, decrease weight even further
		if (isHunterPresent(dv,possibleLocations[i]) == true) {
            mw[i]->weight *= 0.3;
            //if it is a location dracula is in as well as hunter, decrease even more!
            if (mw[i]->location == DvGetPlayerLocation(dv, PLAYER_DRACULA)) {
                mw[i]->weight *= 0.1;
            }
        }
        //make sea locations undersirable
		if (placeIdToType(possibleLocations[i]) == SEA) mw[i]->weight *= 0.8;

        //conditions for drac move if he is at CD
        if (DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA ) {
            //If drac is at CD, make double backs worth more (just to get away)
            if (possibleLocations[i] >= DOUBLE_BACK_2 &&
                possibleLocations[i] <= DOUBLE_BACK_5) {
                mw[i]->weight *= ( 1 + 0.1 * (possibleLocations[i] - DOUBLE_BACK_1));
            }
            //If drac is at castle dracula, never make a d1 or hide move
            if (possibleLocations[i] == HIDE ||  possibleLocations[i] == DOUBLE_BACK_1) {
                mw[i]->weight = 0;
            }
        }
	}

    int pathLength = -1;
    PlaceId * pathToCD = DvGetShortestPathTo(dv, CASTLE_DRACULA,
                                 &pathLength);

	// Factor in possible hunter move collisions
	PlaceId *possibleMovesHunter;
	int numHunterLocations = -1;
    int found = 0;
    int j = 0;

	for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
		numHunterLocations = -1;
		possibleMovesHunter = DvWhereCanTheyGo(dv, hunter, &numHunterLocations);
		if (numHunterLocations != 0) {
            //MW is already sorted, sort possible moves for easy searching
			sortPlaces(possibleMovesHunter,numHunterLocations);

            //certain conditions for drac to consider healing
            if (DvGetHealth(dv,PLAYER_DRACULA) <= 30 && DvGetHealth(dv,PLAYER_DRACULA)
                >= 20 && pathLength != -1) {
                //checking for number of collisions with hunter
                while (j < numHunterLocations) {
                //hunter is in the location or reachable
                if(pathToCD[0] == possibleMovesHunter[j]) found++;
                printf("path is %s\n", placeIdToName(pathToCD[0]));
                printf("comparing to %s\n", placeIdToName(possibleMovesHunter[j]));
                j++;
              }

            }
            //adjust move weights by location of hunters
			applyHunterFactor(mw, mwSize, possibleMovesHunter,numHunterLocations);
		}
    }

    //if the number of hunter collisions is higher than the amount of moves to get to CD
    // and pathLength is non negative, less than 4 possible interactions with hunters
    if ((found < pathLength || found <= 4) && pathLength >= 0 ) {
        //checking that these calculations are valid and assessed against a valid
        //array of possible hunter mvoes (non zero)
        if (j > 0) {
            //return function with best place to move to heal
            return pathToCD[0];
        }
    }
    //otherwise heal move is not needed, return nowhere
    return NOWHERE;
}

// Looks for any matches with hunter locations and reduce weighting of that location
// O(n) complexity at max
// INPUT: two sorted arrays, oneof MoveWeight type and oneof PlaceId
// OUTPUT: void; adjusts weight of each moved based on distance from hunters
void applyHunterFactor(MoveWeight *mw, int mwSize,
	PlaceId *hunterArray, int hunterSize) {
	int i = 0;
	int j = 0;
	while ( i < mwSize && j < hunterSize) {
		//matches
		if(mw[i]->location == hunterArray[j]) {
			//half the current weighting
			mw[i]->weight *= 0.5;
			//advance both pointers
			i++;
			j++;
		//if MW is less, advance MW pointer
		} else if (mw[i]->location < hunterArray[j]) {
			i++;
		//if MW is more than hunterA, advance HunterA pointer
		} else {
			j++;
		}
	}

}

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

// Iterative binary Search - unused
int binarySearch(MoveWeight * array, int l, int r, PlaceId match)
{
    while (l <= r) {
        int m = l + (r - l) / 2;
        // Check if x is present at mid
        if (array[m]->location == match) return m;
        // If x greater, ignore left half
        if (array[m]->location < match) l = m + 1;
        // If x is smaller, ignore right half
        else r = m - 1;
    }
    // if we reach here, then element was not present
	return -1;
}


// COMPARATOR: Compare the order of the elements pointed to by *p and *q.
// -- INPUT: two pointers, *p and *q
// -- OUTPUT:
//    <0 If the element pointed by p goes before the element pointed by q,
//    0  If the element pointed by p is equivalent to the element pointed by q,
//    >0 If the element pointed by p goes after the element pointed by q.
static int MVWeightcompare(const void *p, const void *q) {
    MoveWeight p1 = *(MoveWeight *)p;
    MoveWeight p2 = *(MoveWeight *)q;
	return p2->weight - p1->weight;
}
//SORT PLACES: Sorts an array of PlaceId's from largest value to smallest
// -- INPUT: Array of PlaceId's, number of items in array to be sorted
// -- OUTPUT: void
//Author: testUtils.c; Edited: Cindy
static void sortMVbyWeight(MoveWeight *array, int arraySize) {
    qsort(array, (size_t)arraySize, sizeof(MoveWeight), MVWeightcompare);
    return;
}

// COMPARATOR: Compare the order of the elements pointed to by *p and *q.
// -- INPUT: two pointers, *p and *q
// -- OUTPUT:
//    <0 If the element pointed by p goes before the element pointed by q,
//    0  If the element pointed by p is equivalent to the element pointed by q,
//    >0 If the element pointed by p goes after the element pointed by q.
static int placeIdCmp(const void *p, const void *q) {
    PlaceId p1 = *(PlaceId *)p;
    PlaceId p2 = *(PlaceId *)q;
    return p1 - p2;
}
//SORT PLACES: Sorts an array of PlaceId's from largest value to smallest
// -- INPUT: Array of PlaceId's, number of items in array to be sorted
// -- OUTPUT: void
//Author: testUtils.c; Edited: Cindy
static void sortPlaces(PlaceId *places, int numPlaces) {
    qsort(places, (size_t)numPlaces, sizeof(PlaceId), placeIdCmp);
    return;
}
