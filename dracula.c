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
#include "Queue.h"

#define SPAIN_MOVES BARCELONA,CADIZ,ALICANTE,GRANADA,SANTANDER,LISBON,MADRID,SARAGOSSA
#define SPAIN_SIZE 8
#define IRELAND_MOVES DUBLIN,GALWAY
#define IRELAND_SIZE 2
#define BRITAIN_MOVES EDINBURGH, LIVERPOOL, MANCHESTER, LONDON, SWANSEA, PLYMOUTH
#define BRITAIN_SIZE 6
#define ITALY_MOVES CAGLIARI,NAPLES,BARI,ROME,FLORENCE	//I know athens isnt in italy but theres only one way in/out thats not by sea so i dont want drac to spawn there
#define ITALY_SIZE 5
#define MAGIC_NUMBER_SEED 15
//PlaceId unwantedPlaces[] = {SPAIN_MOVES,IRELAND_MOVES,BRITAIN_MOVES,ITALY_MOVES};

typedef struct moveweight *MoveWeight;
struct moveweight {
	PlaceId location;
	double weight;
	PlaceId moveType;
} ;

static void memoryError (const void * input);
//PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves);
PlaceId getRandomMove();
void applyHunterFactor(MoveWeight *mw, int numPossibleMoves, PlaceId *possibleMovesHunter, int numPossibleMovesHunter);
void weightMovesByLocation(DraculaView dv, MoveWeight *mw, int numPossibleLocations, PlaceId *possibleLocations);
static void sortMVbyWeight(MoveWeight *array, int arraySize);
static int  MVWeightcompare(const void *p, const void *q);
static int placeIdCmp(const void *p, const void *q);
static void sortPlaces(PlaceId *places, int numPlaces);

// Make new MoveWeight Struct item
// INPUT: void
// OUTPUT: Newly created item
MoveWeight MVNewNode(void){
	MoveWeight new = malloc(sizeof(*new));
	memoryError(new);
	new->location = NOWHERE;
	new->weight = -1;
	new->moveType = NOWHERE;
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
// OUTPUT: non specific spawn location
PlaceId spawnDracula (DraculaView dv) {
	PlaceId unwantedPlaces[] = {SPAIN_MOVES,IRELAND_MOVES,BRITAIN_MOVES,ITALY_MOVES,
		ATHENS,HOSPITAL_PLACE,CASTLE_DRACULA};
	//dont reallly care about duplicates? ->check if binary search can handle it
	//sortPlaces(unwantedPlaces,SPAIN_SIZE+ITALY_SIZE+IRELAND_SIZE+BRITAIN_SIZE+1+(NUM_PLAYERS-1));
	printf("Unwanted places are:\n");
	PlaceId placeList[NUM_REAL_PLACES] = {0};
	int numReturnedLocs = -1;
	PlaceId * hunterLocations;
	Player hunter = PLAYER_LORD_GODALMING;
	while (hunter < PLAYER_DRACULA) {
		hunterLocations= DvWhereCanTheyGo(dv, hunter, &numReturnedLocs);
		memoryError(hunterLocations);
		for (int i = 0; i < numReturnedLocs; i++){
			printf("%d: %s\n", hunter, placeIdToName(hunterLocations[i]));
			placeList[hunterLocations[i]] = 1;
		}
		hunter++;
	}
	for(int i = 0; i < SPAIN_SIZE + ITALY_SIZE + IRELAND_SIZE + BRITAIN_SIZE + 3; i++ ){
		printf("%s\n", placeIdToName(unwantedPlaces[i]));
		placeList[unwantedPlaces[i]] = 1;
	}
	for (int i = 0; i < NUM_REAL_PLACES; i++) {
		if(placeIdToType(i) == SEA) placeList[i] = 1;
		printf("[%d]", placeList[i]);

	}


	srand (time(0));
	printf("\n");
	int i = 0;
	PlaceId location = rand() % NUM_REAL_PLACES;
	while (placeList[location] == 1) {
		printf("cant' be %s\n", placeIdToName(location));
		//char input [10];
		//scanf("%s",input);
		i+=MAGIC_NUMBER_SEED;
		srand (time(0));
		location = (rand() + i )% NUM_REAL_PLACES;
		printf("trying %s\n",placeIdToName(location));
	}
	return location;
}

// Debug Print function of weights and locations
// INPUT: MW array, size of array
void printMW (MoveWeight * mw, int size) {
	printf("\nMW is:   \n");
	for (int i = 0; i < size; i++) {
		printf("%s; %lf\n", placeIdToName(mw[i]->location), mw[i]->weight);
	}
	printf("\n");
}

//Decides drac move
// Changes "register best play" output string
void decideDraculaMove(DraculaView dv)
{
	if (DvGetRound(dv) == 0) {
		registerBestPlay(placeIdToAbbrev(spawnDracula(dv)), "Happy Birthday To Me!");
		return;
	}
	int numPossibleMoves = -1;
	PlaceId * possibleMoves = DvGetValidMoves(dv, &numPossibleMoves);

	printf("\nPossible moves are:       ");
	for (int i = 0; i < numPossibleMoves; i ++) {
		printf("%s, ", placeIdToName(possibleMoves[i]));
	}

	int numPossibleLocations;
	PlaceId *possibleLocations =DvWhereCanIGo(dv, &numPossibleLocations);
	printf("\nPossible locations are:   ");
	for (int i = 0; i < numPossibleLocations; i ++) {
		printf("%s, ", placeIdToName(possibleLocations[i]));
	}
	printf("\n");
	//int numPossibleMoves = -1;
	//PlaceId * possibleLocations = DvGetValidMoves(dv, &numPossibleMoves);
	//either there are no valid moves besides teleport, or drac hasnt made a move yet
	if (numPossibleLocations == 0) {
		//If it is not round 0 (where drac hasnt made a move yet) and there are no possible moves, dracula must teleport
		if (DvGetRound(dv) != 0) {
			registerBestPlay(placeIdToAbbrev(TELEPORT), "You'll never expect this!");
			return;
		//This is first round, just spawn dracula
		} else {
			registerBestPlay(placeIdToAbbrev(spawnDracula(dv)), "Happy Birthday To Me!");
			return;
		}
	}
	//sorts array alphabethically
	sortPlaces(possibleLocations,numPossibleLocations);
	printf("Number of possible moves is %d\n",numPossibleLocations);
	int MvArraySize = numPossibleLocations;
	MoveWeight * MvArray = MvArrayNew(MvArraySize);
	weightMovesByLocation(dv, MvArray, MvArraySize,possibleLocations);
	//do Stuff
	sortMVbyWeight(MvArray, MvArraySize);
	//highest weighted location is best choice
	PlaceId bestMove = MvArray[0]->location;
	printf("best move is: %s\n", placeIdToName(bestMove));
	printMW(MvArray, MvArraySize);

	//make a hide move
	if(bestMove == DvGetPlayerLocation(dv,PLAYER_DRACULA)) {
		registerBestPlay(placeIdToAbbrev(HIDE), "marco polo?");
	}


	registerBestPlay(placeIdToAbbrev(bestMove), "doing my best ");
	freeArray(MvArray,numPossibleLocations);
	return;
}



// Returns the placeid of a random place reachable by drac this turn
//-------------------------------------------------------------------------------UNUSED

PlaceId getRandomMove(DraculaView dv) {
	//for ultimate randomness. comment out if you want repeateability
	srand ( time(0) );
	int r = rand();
	if (DvGetRound(dv) == 0){
		PlaceId location = r % NUM_REAL_PLACES;
//drac doesnt like spain or ireland or on top of hunters or cagliarya
		 while(placeIdToType(location) == SEA || location == GALWAY || location == HOSPITAL_PLACE ||location == DUBLIN
		 || location == SARAGOSSA || location == ALICANTE || location == BARCELONA || location == SANTANDER
		 || location == MADRID || location == GRANADA || location == CADIZ || location == LISBON || location == CAGLIARI
	 	 || location == DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) || location
		 == DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) || location ==
		 DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) || location == DvGetPlayerLocation(dv, PLAYER_MINA_HARKER)) {
			 location = spawnDracula(dv);
			//printf("attempting to spawn at %s\n", placeIdToName(location));
		}
		//printf("successfully spawned at %s\n", placeIdToName(location));
		return location;
	}
	//printf("Current Location: %s\n", placeIdToName(DvGetPlayerLocation(dv, PLAYER_DRACULA)));
	int numPossibleMoves;
	PlaceId *possibleMoves =DvGetValidMoves(dv, &numPossibleMoves);
	//printf("\nPossible moves are:       ");
	//for (int i = 0; i < numPossibleMoves; i ++) {
	//	printf("%s, ", placeIdToName(possibleMoves[i]));
	//}
	//int numPossibleLocations;
	//PlaceId *possibleLocations =DvWhereCanIGo(dv, &numPossibleLocations);
	//printf("\nPossible locations are:   ");
	//for (int i = 0; i < numPossibleLocations; i ++) {
	//	printf("%s, ", placeIdToName(possibleLocations[i]));
	//}
	//printf("\n");
	int random = r % numPossibleMoves;
	//printf("RNG: %d\n", random);
	return possibleMoves[random];
}

//Checks if any hunter is present at a particula locations
// Potentially unused?
bool isHunterPresent (DraculaView dv, PlaceId location) {
	Player hunter = PLAYER_LORD_GODALMING;
	while (hunter < PLAYER_DRACULA) {
		if(DvGetPlayerLocation(dv,hunter) == location) return true;
		hunter++;
	}
	return false;
}

// weight dracula's possible moves based on if a hunter is currently there or reaachable by
void weightMovesByLocation(DraculaView dv, MoveWeight * mw, int mwSize, PlaceId *possibleLocations) {

	assert(mwSize > 0);
	//associate a certain weight to each location
	for (int i = 0; i < mwSize; i++) {
		mw[i]->location = possibleLocations[i];
		mw[i]->weight = 10;
		//if hunter is already at location, decrease weight even further
		if (isHunterPresent(dv,possibleLocations[i]) == true) mw[i]->weight *= 0.3;
		if (placeIdToType(possibleLocations[i]) == SEA) mw[i]->weight *= 0.8;
	}

	// Factor in possible hunter move collisions
	PlaceId *possibleMovesHunter;
	int numHunterLocations = -1;
	for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
		numHunterLocations = -1;
		possibleMovesHunter = DvWhereCanTheyGo(dv, hunter, &numHunterLocations);
		if (numHunterLocations != 0) {
			sortPlaces(possibleMovesHunter,numHunterLocations);
			//both MW already sorted as possibleLocations is sorted.
			applyHunterFactor(mw, mwSize, possibleMovesHunter,numHunterLocations);
		}

	}
    return;
}

//slightly better than n^2 complexity now since both pointers are bumped
// INPUT: two sorted arrays, oneof MoveWeight type and oneof PlaceId
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

    // if we reach here, then element was
    // not present
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
