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
	int *numPossibleMoves;
	player whoseTurn = hv->currentPlayer;
	//check where player is at, if hunter has 0 health, is in hospital
	if (HvGetHealth(hv,whoseTurn) > 0) {
		PlaceId currentLocation = HvGetPlayerLocation(hv,whoseTurn );
	} else {
		PlaceId currentLocation = HOSPITAL_PLACE;
	}
	PlaceId * possibleMoves = HvGetValidMoves(hv, numPossibleMoves);
	registerBestPlay("TO", "Have we nothing Toulouse?");
}
