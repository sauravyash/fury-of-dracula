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

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"

typedef struct moveweight {
	PlaceId *location;
	double *weight;
} *MoveWeight


PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves);
PlaceId getRandomMove();

void decideDraculaMove(DraculaView dv)
{
	int numPossibleMoves;
	getPossibleMoves(dv, numPossibleMoves);
	MoveWeight moves = malloc(numPossibleMoves * sizeof(struct moveweight));
	moves->location = getPossibleMoves(dv, numPossibleMoves);					// yeah its called twice but w/e
	// TODO: Replace this with something better!
	registerBestPlay(placeIdToAbbrev(getRandomMove(dv)), "You'll never expect this!");
}

// Returns an array of all placeids reachable by drac this turn
PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves) {
	PlaceId *possibleMoves = DvGetValidMoves(dv, numPossibleMoves);
	// if no other legal moves, drac tps to castle dracula
	if (*numPossibleMoves == 0) {
		possibleMoves[0] = TELEPORT;
		*numPossibleMoves = 1;
	}
	// write this to our array?
	return possibleMoves;
}

// Returns the placeid of a random place reachable by drac this turn
PlaceId getRandomMove(DraculaView dv) {
	//for ultimate randomness. comment out if you want repeateability
	srand ( time(0) );
	int r = rand();
	if (DvGetRound(dv) == 0){
		//drac hasnt had a turn yet
		//printf("RNGods: %d\n", r);
		PlaceId location = r % NUM_REAL_PLACES;
		//printf("attempting to spawn at %s\n", placeIdToName(location));
		while(placeIdToType(location) == SEA) {
			location = rand() % NUM_REAL_PLACES;
			//printf("attempting to spawn at %s\n", placeIdToName(location));
		}
		//printf("successfully spawned at %s\n", placeIdToName(location));
		return location;
	}
	int numPossibleMoves;
	PlaceId *possibleMoves = getPossibleMoves(dv, &numPossibleMoves);
	printf("Possible moves are:");
	for (int i = 0; i < numPossibleMoves; i ++) {
		printf("%s ", placeIdToName(possibleMoves[i]));
	}
	printf("\n");
	int random = r % numPossibleMoves;
	printf("RNG: %d\n", random);
	return possibleMoves[random];
}
