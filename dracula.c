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

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "Map.h"
#include "Queue.h"

typedef struct moveweight {
	PlaceId *location;
	double *weight;
} *MoveWeight;


PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves);
PlaceId getRandomMove();

void decideDraculaMove(DraculaView dv)
{
	int numPossibleMoves;
	getPossibleMoves(dv, &numPossibleMoves);
	MoveWeight moves = malloc(numPossibleMoves * sizeof(struct moveweight));
	moves->location = getPossibleMoves(dv, &numPossibleMoves);					// yeah its called twice but w/e
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

// add weights based on distance from drac
MoveWeight *weightMovesByLocation(DraculaView dv, MoveWeight *mw) {
	// initialise
	int *numReturnedLocs = malloc(sizeof(int));
	memoryError(*numReturnedLocs);
	PlaceId *possibleMoves = DvWhereCanIGo(dv, numReturnedLocs);
	int numPossibleMoves = *numReturnedLocs;
    
	PlaceId src = possibleMoves[0];
    PlaceId final_dest = NOWHERE;

	// pos 0 is the current pos, which doesn't need a movement score
	for (int i = 1; i < numPossibleMoves - 1; i++) {
		PlaceId dest = possibleMoves[i];
		// Make a visited array
		int visited[NUM_REAL_PLACES];
		for (int i = 0; i < NUM_REAL_PLACES; i++) {
			visited[i] = -1;
		}

		visited[0] = src;

		bool found = false;
		bool pathFound = false;
		int temp_round = 0;
		PlaceId temp_place = NOWHERE;

		// Make Queue to travel breadth-first
		Queue q = newQueue();
		QueueJoin(q, src);

		// BFS
		while (!found && !QueueIsEmpty(q)) {
			int prev_place = QueueLeave(q);
			// When we create a new_place, we must create from connections of prev.
			temp_place = prev_place;
			// Checking the round:
			int path_count = 0;
			PlaceId prev = prev_place;
			while (prev != src) {
				path_count++;
				prev = visited[prev];
			}
			temp_round += path_count;

			int numLocs;
			PlaceId *list = possibleMoves;

			if (prev_place == dest) {
				found = true;
			} else for (int i = 1; i < numLocs; i++) {
				int new_place = possibleMoves[i];
				if (visited[new_place] == -1 && prev_place != new_place) {
					visited[new_place] = prev_place;
					QueueJoin(q, new_place);
					if (new_place == dest) {
						pathFound = true;
					}
				}
			}
			free(list);
		}
		dropQueue(q);

		// Reset values:
		temp_place = NOWHERE;
		temp_round = 0;

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
	}

    // Mind blank about shortening array from front, so...
    PlaceId *path_without_src = malloc ((k) * sizeof(PlaceId));
    memoryError(path_without_src);
    for (int i = 0; i < k; i++) {
        path_without_src[i] = path[i + 1];
    }

    free(path);

	free(numReturnedLocs);
    return path_without_src;
}