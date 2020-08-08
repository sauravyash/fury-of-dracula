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
#include <time.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "Places.h"

// Moveweight Struct
typedef struct moveweight *MoveWeight;

struct moveweight {
    PlaceId location;
    double weight;
    PlaceId moveType;
};

// FUNCTION DEC:
PlaceId DraculaHunt (HunterView hv, PlaceId bestMove, Player current_player);

void printMwArray(MoveWeight *mw, int size);
Player findClosestPlayer(HunterView hv, PlaceId pID);
Player *findClosestPlayers(HunterView hv, PlaceId pID);
int findMoveWeightIndex(MoveWeight *mw, int mwSize, PlaceId pID);
static void sortMVbyWeight(MoveWeight *array, int arraySize);
static int MVWeightcompare(const void *p, const void *q);
void weightMovesByLocation(HunterView hv, MoveWeight *mw, int mwSize, PlaceId *possibleLocations);
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

	// We need to check if the hunter is dead
    //int health = HvGetHealth(hv, current_player);
    //printf("health: %d, score: %d\n", health, HvGetScore(hv));
    //if (health < 1) {
    //    registerBestPlay(placeIdToAbbrev(ST_JOSEPH_AND_ST_MARY), "rip i died");
    //    return;
    //}

	// If it is the first round, give everyone a position:
	if (round == 0) {
	    if(current_player == PLAYER_LORD_GODALMING) bestMove = PARIS;
		else if(current_player == PLAYER_DR_SEWARD) bestMove = BERLIN;
		else if(current_player == PLAYER_VAN_HELSING) bestMove = MADRID;
		else if(current_player == PLAYER_MINA_HARKER) bestMove = KLAUSENBURG;

		// Checking that the player has a place to move to...
		if (bestMove == NOWHERE) printf("ERROR: No place allocated...\n");

	    registerBestPlay(placeIdToAbbrev(bestMove), "Lol we moving");
	    return;
	}

    // CHASING DRAC
    // So if any player is closest to drac make them hunt him and leave earlier
    // job, but don't make them leave their job just to follow the leader.
    bestMove = DraculaHunt(hv, bestMove, current_player);

	// Return that move...
	registerBestPlay(placeIdToAbbrev(bestMove), "Lol we moving");
	return;

}

PlaceId DraculaHunt (HunterView hv, PlaceId bestMove, Player current_player) {
    // NOW FIND OUT WHERE DRAC IS FOR CHASING:
    // Basically, whoever is closest to Drac becomes Leader...
    // (or if drac is unknown, then Lord G)

    int numPossibleLocations = 0;
    PlaceId *possible_places = HvWhereCanIGoByType(hv, true, false,
                             true, &numPossibleLocations);
    MoveWeight *MvArray = MvArrayNew(numPossibleLocations);

    weightMovesByLocation(hv, MvArray, numPossibleLocations, possible_places);

    // sort by weight
    sortMVbyWeight(MvArray, numPossibleLocations);
    printMwArray(MvArray, numPossibleLocations);
    PlaceId best = MvArray[0]->location;
    return best;
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
void weightMovesByLocation(HunterView hv, MoveWeight *mw, int mwSize, PlaceId *possibleLocations) {
    srand(time(NULL));
    assert(mwSize > 0);
    
    bool isLeader = false;
    Player currentPlayer = HvGetPlayer(hv);
    int round;
    PlaceId dracLastKnownLoc = HvGetLastKnownDraculaLocation(hv, &round);
    Player leader = findClosestPlayer(hv, dracLastKnownLoc);
    if (leader == currentPlayer) isLeader = true;

    // associate a certain weight to each location
    for (int i = 0; i < mwSize; i++) {
        //srand(time(NULL));
        mw[i]->location = possibleLocations[i];
        mw[i]->weight = 10; //* ((rand() % 50) / 10);

        // if dracula is already at location, increase weight
        int round;
        PlaceId LastKnownDracLoc = HvGetLastKnownDraculaLocation(hv, &round);

        // Leader Chases if last known loc is within 10 moves
        if (LastKnownDracLoc == possibleLocations[i] && (HvGetRound(hv) - round) < 4 && isLeader) {
            if (placeIdToType(possibleLocations[i]) == SEA) {
                mw[i]->weight *= 2;
            }
            mw[i]->weight *= (5 - (HvGetRound(hv) - round)) * 2;
        }
    }

    int currPlayerPlaceIndex = 
        findMoveWeightIndex(mw, mwSize, HvGetPlayerLocation(hv,currentPlayer));
    
    // leader moves
    if (isLeader) {
        if (currPlayerPlaceIndex == -1) {
            fprintf(stderr, "couldn't find hunter's location in moveweights\n");
        } else if (HvGetHealth(hv, currentPlayer) < 4) {
            // Should hunter heal?
            mw[currPlayerPlaceIndex]->weight *= 4;
        } else if (HvGetHealth(hv, currentPlayer) < 7) {
            mw[currPlayerPlaceIndex]->weight *= 1.5;
        } else {
            mw[currPlayerPlaceIndex]->weight *= 0.3;
        }
    } else {
        if (HvGetHealth(hv, currentPlayer) < 4) {
            // Should hunter heal?
            mw[currPlayerPlaceIndex]->weight *= 5;
        } else if (HvGetHealth(hv, currentPlayer) < 7) {
            mw[currPlayerPlaceIndex]->weight *= 2;
        } else {
            mw[currPlayerPlaceIndex]->weight *= 0.5;
        }
    }

    // kill immature vampire
    PlaceId immatureVamp = HvGetVampireLocation(hv);
    if (placeIsReal(immatureVamp) && !isLeader) {
        Player *closestPlayers = findClosestPlayers(hv, immatureVamp);
        Player closestNonLeader = 
            (closestPlayers[0] == leader) ? closestPlayers[0] : closestPlayers[1];
        free(closestPlayers);
        if (currentPlayer == closestNonLeader) {
            int len;
            PlaceId *path = HvGetShortestPathTo(hv, currentPlayer, immatureVamp, &len);
            printf("Path pos 0: %s", placeIdToName(path[0]));
            int index = findMoveWeightIndex(mw, mwSize, path[0]);
            if (index != -1) mw[index]->weight *= 5;
        }
    }
    
    // keep mina close to castle
    if (HvGetPlayer(hv) == PLAYER_MINA_HARKER) {
        int len, round;
        PlaceId *route = HvGetShortestPathTo(hv, currentPlayer, CASTLE_DRACULA, &len);
        PlaceId lastknown = HvGetLastKnownDraculaLocation(hv, &round);
        Player p = findClosestPlayer(hv, lastknown);
        if (len > 4 || (HvGetRound(hv) - round < 4 && p != currentPlayer)) {
            int i = findMoveWeightIndex(mw, mwSize, route[0]);
            if (i != -1) {
                mw[i]->weight *= 10;
            }
        }
    }  
    
    // research if last known drac move was before 7 moves dynamically
    int rounds;
    HvGetLastKnownDraculaLocation(hv, &rounds);
    if (HvGetRound(hv) - rounds > 7) {
        int healthArray[4] = {0};
        int notOptimal = 0;
        for (int i = PLAYER_LORD_GODALMING; i < PLAYER_DRACULA; i++) {
            healthArray[i] = HvGetHealth(hv, (Player) i);
            if (healthArray[i] < 4) notOptimal++;
        }
        // min factor 7/3 and increases as unknown time increases
        mw[currPlayerPlaceIndex]->weight *= (HvGetRound(hv) - rounds) / 3;
        mw[currPlayerPlaceIndex]->weight *= notOptimal > 1 ? 1.75 :
            notOptimal > 0 ? 1.25 : 0.5;
    }

    // avoid traps
    /*
    int numTraps;
    PlaceId *traps = HvGetTrapLocations(hv->gv, numTraps);
    for (int i = 0; i < numTraps; i++) {
        int mwIndex = findMoveWeightIndex(mw, mwSize, traps[i]);
        if (mwIndex != -1) mw[i]->weight *= 0.01;
    }
    */

    // discourage retracing previous steps
    int num;
    bool isFree;
    PlaceId *lastMoves = HvGetMoveHistory(hv, currentPlayer, &num, &isFree);
    for (int i = 0; i < num; i++) {
        float factor = 0.05 * (num - i);
        factor = factor > 1 ? 1 : factor;

        printf("lastmove: %s, factor: %f\n", placeIdToName(lastMoves[i]), factor);
        int index = findMoveWeightIndex(mw, mwSize, lastMoves[i]);
        if (index != -1) mw[index]->weight *= factor;
    } 
    
    if (isFree) free(lastMoves);

    return;
}

void printMwArray(MoveWeight *mw, int size) {
    for (int i = 0; i < size; i++) {
        printf("Loc: %s, Weight: %f\n",
                placeIdToName(mw[i]->location),
                mw[i]->weight);
    }
}

Player findClosestPlayer(HunterView hv, PlaceId pID) {
    if (!placeIsReal(pID)) return PLAYER_LORD_GODALMING; 
    Player *list = findClosestPlayers(hv, pID);
    Player leader = list[0];
    free(list);
    return leader;
}

// returns a list of the closest players
// return MUST BE FREED
Player *findClosestPlayers(HunterView hv, PlaceId pID) {
    Player *closest = malloc(sizeof(Player) * (NUM_PLAYERS - 1));
    int playerMoveLen[NUM_PLAYERS - 1] = {0,1,2,3};
    
    if (!placeIsReal(pID)) return NULL;
    
    int lenShortest;
    for (int i = 0; i < NUM_PLAYERS - 1; i++) {
        PlaceId *p = HvGetShortestPathTo(hv, i, pID, &lenShortest);
        playerMoveLen[i] = lenShortest;
        free(p);
    }
    
    for (int i = 0; i < NUM_PLAYERS - 1; i++) {
        bool used[4] = { false };
        int highestIndex = 0;
        for (int player = 0; player < NUM_PLAYERS - 1; player++) {
            if (playerMoveLen[player] > playerMoveLen[highestIndex] && !used[player]) {
                highestIndex = player;
            }
        }
        closest[i] = highestIndex;
    }
    return closest;
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
