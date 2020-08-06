////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "Places.h"


#define PLAYER1_START_POS 22
#define PLAYER2_START_POS 11
#define PLAYER3_START_POS 17
#define PLAYER4_START_POS 44

// Moveweight Struct
typedef struct moveweight *MoveWeight;

struct moveweight {
    PlaceId location;
    double weight;
    PlaceId moveType;
};

// FUNCTION DEC:
PlaceId MapSleuth (HunterView hv);
PlaceId DraculaHunt (HunterView hv, PlaceId bestMove, Player current_player);

int findMoveWeightIndex(MoveWeight *mw, int mwSize, PlaceId pID);
static void sortMVbyWeight(MoveWeight *array, int arraySize);
static int MVWeightcompare(const void *p, const void *q);
void weightMovesByLocation(HunterView hv, MoveWeight *mw, int mwSize, PlaceId *possibleLocations, bool isLeader);
MoveWeight MVNewNode(void);
MoveWeight *MvArrayNew(int size);
static void memoryError (const void *input);

void decideHunterMove(HunterView hv)
{
	// First things first:
	Round round = HvGetRound(hv);
	//int current_score = HvGetScore(hv);
	Player current_player = HvGetPlayer(hv);
	PlaceId bestMove = NOWHERE;
	
	// Do we need to check if the hunter is dead lol??
	
	// If it is the first round, give everyone a position:
	if (round == 0) {    
	    if(current_player == PLAYER_LORD_GODALMING) bestMove = PARIS;
		else if(current_player == PLAYER_DR_SEWARD) bestMove = BERLIN;
		else if(current_player == PLAYER_VAN_HELSING) bestMove = MADRID;
		else if(current_player == PLAYER_MINA_HARKER) bestMove = CASTLE_DRACULA;
		
		// Checking that the player has a place to move to...
		if (bestMove == NOWHERE) printf("ERROR: No place allocated...\n");
	    
	    registerBestPlay(placeIdToAbbrev(bestMove), "Lol we moving");
	    return;
	}
    
    // SORT PLAYERS BY ROLE:
    
    // PATROL EDGES OF MAP
    // Van Hell to patrol the edges of the map...
    // Go to furthest location from hunters...
    if (current_player == PLAYER_VAN_HELSING) {
        bestMove = MapSleuth(hv);
    }
    
    // GUARD CASTLE DRAC
    // When we have getlastlocation function maybe rotate:
    // Castle_Drac->Galatz->Klausenburg and then stay there if encounter Drac.
    if (current_player == PLAYER_MINA_HARKER) {
        bestMove = CASTLE_DRACULA;
    }
    
    // CHASING DRAC
    // So if any player is closest to drac make them hunt him and leave earlier
    // job, but don't make them leave their job just to follow the leader.
    bestMove = DraculaHunt (hv, bestMove, current_player);
	
	// Return that move...
	registerBestPlay(placeIdToAbbrev(bestMove), "Lol we moving");
	return;
	
}

PlaceId DraculaHunt (HunterView hv, PlaceId bestMove, Player current_player) {
    
    // NOW FIND OUT WHERE DRAC IS FOR CHASING:
    // Basically, whoever is closest to Drac becomes Leader...
    // (or if drac is unknown, then Lord G)
    Round drac_round = -1;
    PlaceId Drac_Loc = HvGetLastKnownDraculaLocation(hv, &drac_round);
    // If drac_round < round && drac_round != 1: Maybe estimate dracs moves
    // ie. aim for a location a few places away from his last known location
    // that is not water and is not in the direction of the hunters.
    Player Leader;
    if (!placeIsReal(Drac_Loc)) Leader = PLAYER_LORD_GODALMING;
    else {
        // Find player closest to Drac.
        int temp_player = 0;
        int length = NUM_REAL_PLACES;
        int new_length = NUM_REAL_PLACES;
        while (temp_player < NUM_PLAYERS) {
            HvGetShortestPathTo(hv, temp_player, Drac_Loc, &new_length);
            if (new_length < length) {
                new_length = length;
                Leader = temp_player;
            }
            temp_player++;
        }
    }
    
    printf("Current leader is %d.\n", Leader);
	PlaceId Leader_Loc = HvGetPlayerLocation(hv, Leader);
	printf("All patrolling players move towards <%s>.\n", placeIdToName(Leader_Loc));
    
    int numPossibleLocations = 0;
    PlaceId *possible_places = HvWhereCanIGoByType(hv, true, false,
                             true, &numPossibleLocations);
    MoveWeight *MvArray = MvArrayNew(numPossibleLocations);
    bool isLeader = HvGetPlayer(hv) == Leader;
    weightMovesByLocation(hv, MvArray, numPossibleLocations, possible_places, isLeader);
    
    // sort by weight
    sortMVbyWeight(MvArray, numPossibleLocations);
    
    return MvArray[0]->location;

    /* using moveweights instead [delete once mw is working]
    if (current_player == Leader) {
		printf("I am the leader\n");
		// Basically try and move closer to dracula.
		if (placeIsReal(Drac_Loc)) {
		    int len = 0;
		    PlaceId *chaseDrac = HvGetShortestPathTo(hv, current_player, Drac_Loc, &len);
		    if (len > 0) bestMove = chaseDrac[0];
		    else bestMove = Leader_Loc;
		} else {
		    // If drac has not been found!!!
		    int len = 0;
		    PlaceId *possible_moves = HvWhereCanIGo(hv, &len);
		    int r = (rand()%(len-1))+1;
		    bestMove = possible_moves[r];
		}
	
	} else {
		if (current_player == PLAYER_MINA_HARKER || current_player == PLAYER_VAN_HELSING) {
		    // Don't follow after drac... or should u hmm...
		    return bestMove;
		}
		printf("I am a follower");
		// Basically move closer to the leader...
		int len = 0;
		PlaceId *followLead = HvGetShortestPathTo(hv, current_player, Leader_Loc, &len);
		if (len > 0) bestMove = followLead[0];
		else bestMove = Leader_Loc;
	}

	return bestMove;
    */
}

PlaceId MapSleuth (HunterView hv) {
    int numPossibleLocations = 0;
    PlaceId *possible_places = HvWhereCanIGoByType(hv, true, false,
                             true, &numPossibleLocations);
    
    /*MoveWeight *MvArray = MvArrayNew(MvArraySize);
    weightMovesByLocation(dv, MvArray, numPossibleLocations, possible_places);
    
    // sort by weight
    sortMVbyWeight(MvArray, numPossibleLocations);
    */
    
    // Select one for now... randomly:
    if (numPossibleLocations > 1) return possible_places[1];
    else return possible_places[0];
}

// weight dracula's possible moves based on if dracula is currently there or reachable
// Usage: 
/* */
void weightMovesByLocation(HunterView hv, MoveWeight *mw, int mwSize, PlaceId *possibleLocations, 
        bool isLeader) {
    
    assert(mwSize > 0);

    Player currentPlayer = HvGetPlayer(hv);
    //associate a certain weight to each location
    for (int i = 0; i < mwSize; i++) {
        mw[i]->location = possibleLocations[i];
        mw[i]->weight = 10;
        
        // if dracula is already at location, increase weight
        int *round = malloc(sizeof(int));
        PlaceId LastKnownDracLoc = HvGetLastKnownDraculaLocation(hv, round);
        
        // Leader Chases if last known loc is within 10 moves
        if (LastKnownDracLoc == possibleLocations[i] && (HvGetRound(hv) - *round) < 10 && isLeader) {
            if (placeIdToType(possibleLocations[i]) == SEA) {
                mw[i]->weight *= 2;
            }
            mw[i]->weight *= 5 - (HvGetRound(hv) - *round);
        }

        free(round);
    }

    int currPlayerPlaceIndex = findMoveWeightIndex(mw, mwSize, HvGetPlayerLocation(hv,currentPlayer));
    //leader moves
    if (isLeader) {
        if (currPlayerPlaceIndex == -1) {
            fprintf(stderr, "couldn't find hunter's location in moveweights\n");
        } else if (HvGetHealth(hv, currentPlayer) < 4) {
            // Should hunter heal?
            mw[currPlayerPlaceIndex]->weight *= 4;
        } else if (HvGetHealth(hv, currentPlayer) < 7) {
            mw[currPlayerPlaceIndex]->weight *= 1.5;
        } else {
            mw[currPlayerPlaceIndex]->weight *= 0.5;
        }
    } else {
        if (HvGetHealth(hv, currentPlayer) < 7) {
            // Should hunter heal?
            mw[currPlayerPlaceIndex]->weight *= 4;
        } 
        
        /*int pathLen;
        PlaceId *shortestPath = HvGetShortestPathTo(hv, currentPlayer, ,&pathLen);
        if (*/
    }
    
    // research
    // int healthArray[4] = {0};


    return;
}


// ALL FUNCTIONS BELOW COPIED FROM dracula.c
MoveWeight *MvArrayNew(int size) {
    MoveWeight *arrayWeightedMoves = malloc(size * sizeof(MoveWeight));
    for (int i = 0; i < size; i++){
        arrayWeightedMoves[i] = MVNewNode();
    }
    return arrayWeightedMoves;
}

MoveWeight MVNewNode(void){
    MoveWeight new = malloc(sizeof(*new));
    memoryError(new);
    new->location = NOWHERE;
    new->weight = -1;
    new->moveType = NOWHERE;
    return new;
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

int findMoveWeightIndex(MoveWeight *mw, int mwSize, PlaceId pID) {
    for (int i = 0; i < mwSize; i++) {
        if (mw[i]->location == pID) return i; 
    }
    return -1;
}

// MEMORY ERROR: Helper function to check correct memory allocation. Exits if
// memory was not correctly allocated
// -- INPUT: pointer to a malloced object
// -- OUTPUT: void, but prints error message and exits code 1
static void memoryError (const void *input) {
    if (input == NULL) {
        fprintf(stderr, "Couldn't Allocate Memory!\n");
        exit(EXIT_FAILURE);
    }
    return;
}
