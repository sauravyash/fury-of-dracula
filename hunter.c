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

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"

void decideHunterMove(HunterView hv)
{
	// TODO: Replace this with something better!
	registerBestPlay("TO", "Have we nothing Toulouse?");
}


// Find places with weights
//this is all for hunter.c?
/*
if (possibleMoves == NULL) {
	// first move
	// TODO
} else if (numPossibleMoves < 1) {
	// when no possible moves, only option: TP
	// TODO
} else {
	// find distance to all positions
	for (int i = 0; i < numPossibleMoves; i++) {
		PlaceId dest = possibleMoves[i];
		// Make a visited array
		int visited[NUM_REAL_PLACES];
		for (int i = 0; i < NUM_REAL_PLACES; i++) {
			visited[i] = -1;
		}

		visited[0] = src;
		bool found = false;

		// Make Queue to travel breadth-first
		Queue q = newQueue();
		QueueJoin(q, src);

		// bool pathFound = false;
		// int temp_round = 0;
		// PlaceId temp_place = NOWHERE;

		// BFS
		while (!found && !QueueIsEmpty(q)) {
			int prev_place = QueueLeave(q);
			// Checking the round:
			// int path_count = 0;
			// PlaceId prev = prev_place;
			// while (prev != src) {
			// 	path_count++;
			// 	prev = visited[prev];
			// }
			// temp_round += path_count;

			int numLocs = numPossibleMoves;
			PlaceId *list = possibleMoves;

			if (prev_place == dest) {
				found = true;
			} else for (int i = 0; i < numLocs; i++) {
				int new_place = possibleMoves[i];
				if (visited[new_place] == -1 && prev_place != new_place) {
					visited[new_place] = prev_place;
					QueueJoin(q, new_place);
					// if (new_place == dest) {
					// 	pathFound = true;
					// }
				}
			}
			free(list);
		}
		dropQueue(q);

		// Reset values:
		// temp_place = NOWHERE;
		// temp_round = 0;

		// if (!pathFound) return 0;
		bool pathFound = false;
		for (int i = 0; i < NUM_REAL_PLACES; i++) {
			if (visited[i] == src) {
				pathFound = true;
				break;
			}
		}

		if (!pathFound) {
			// This should never occur (indicates problem in DvWhereCanIGo)

			// TODO: create error handling in case this happens
		} else {
			int reversePath[NUM_REAL_PLACES];
			reversePath[0] = dest;

			int prev = visited[dest];
			int len = 1;
			while (visited[prev] != -1) {
				reversePath[len] = prev;
				len++;
				prev = visited[prev];
				if (prev == src) {
					reversePath[len] = src;
					break;
				}
			}

			for (int i = 0; i < numPossibleMoves; i++) {
				if (placeWeights[i]->location == reversePath[i]) {
					// do stuff to weights here
					placeWeights[i]->weight = 0.5;
				}
			}

			// Code for finding path from src->dest
			// PlaceId *path = malloc ((len + 1) * sizeof(PlaceId));
			// memoryError(path);
			// for (int i = 0, j = len; i <= len; i++, j--) {
			// 	path[i] = reversePath[j];
			// }
			//free(path);
		}
	}
	// Mind blank about shortening array from front, so...
	PlaceId *path_without_src = malloc ((k) * sizeof(PlaceId));
	memoryError(path_without_src);
	for (int i = 0; i < k; i++) {
		path_without_src[i] = path[i + 1];
	}

}
*/
