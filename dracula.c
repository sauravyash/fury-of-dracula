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

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"

PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves);
PlaceId getRandomMove();

void decideDraculaMove(DraculaView dv)
{
	// TODO: Replace this with something better!
	registerBestPlay(placeIdToAbbrev(getRandomMove(dv)), "You'll never expect this!");
}

// Returns an array of all placeids reachable by drac this turn
PlaceId *getPossibleMoves(DraculaView dv, int *numPossibleMoves) {
	PlaceId *possibleMoves = DvGetValidMoves(dv, numPossibleMoves);
	// write this to our array?
	return possibleMoves;
}

// Returns the placeid of a random place reachable by drac this turn
PlaceId getRandomMove(DraculaView dv) {
	int numPossibleMoves;
	PlaceId *possibleMoves = getPossibleMoves(dv, &numPossibleMoves);
	return possibleMoves[rand() % numPossibleMoves];
}