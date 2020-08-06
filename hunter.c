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
#include <time.h>
PlaceId getRandomMove(HunterView hv);
void decideHunterMove(HunterView hv)
{
	// TODO: Replace this with something better!
	srand(time(NULL));
	PlaceId bestMove;
	int pathLength = -1;
	if (HvGetRound(hv) == 0) {
		bestMove = getRandomMove(hv);
		registerBestPlay(placeIdToAbbrev(bestMove), "marco polo?");
		return;
	}
	Player hunter = HvGetPlayer(hv);
	int health = HvGetHealth(hv, hunter);
	PlaceId * bloodyBrickRoad;
	if(health <= 0) {
		//hunter is gonna get tp'ed to hospital
		bloodyBrickRoad =  HvWhereCanIGo(hv, &pathLength);
		bestMove = bloodyBrickRoad[rand() % pathLength];
		registerBestPlay(placeIdToAbbrev(bestMove), "+1HP");
		return;
	}

	PlaceId vamp = HvGetVampireLocation(hv);
	if (vamp != NOWHERE) {
		//we know where the vamp is
		bloodyBrickRoad = HvGetShortestPathTo(hv, hunter, vamp,
		                             &pathLength);
		if (pathLength != -1) {
			bestMove = bloodyBrickRoad[0];
			registerBestPlay(placeIdToAbbrev(bestMove), "its a stakeout");
			return;
		}

	}

	Round sawDracRound = -1;
	PlaceId lastSawDrac = HvGetLastKnownDraculaLocation(hv, &sawDracRound);
	if(sawDracRound != -1) {
		//have seen drac before
		bloodyBrickRoad = HvGetShortestPathTo(hv, hunter, lastSawDrac,
		                             &pathLength);
		if (pathLength != -1) {
			bestMove = bloodyBrickRoad[0];
			registerBestPlay(placeIdToAbbrev(bestMove), "the game is on!");
			return;
		}

	} else  {
		//hunter is in hospital!?? but should have healed up now
		bloodyBrickRoad =  HvWhereCanIGo(hv, &pathLength);
		bestMove = bloodyBrickRoad[rand() % pathLength];
		registerBestPlay(placeIdToAbbrev(bestMove), "~~~");
		return;
	}


}


// Returns the placeid of a random place reachable by drac this turn
//discountinued
PlaceId getRandomMove(HunterView hv) {
	//for ultimate randomness. comment out if you want repeateability
	int r = rand();
		//drac hasnt had a turn yet
		//printf("RNGods: %d\n", r);
		PlaceId location = r % NUM_REAL_PLACES;

		//printf("attempting to spawn at %s\n", placeIdToName(location));
		//PlaceId *possibleMovesHunter;
		//int numHunterLocations = -1;
//hunter cant be at castle drac
		while( location == CASTLE_DRACULA) {
			srand ( time(0) );
					location = rand() % NUM_REAL_PLACES;
			//printf("attempting to spawn at %s\n", placeIdToName(location));
		}
		//printf("successfully spawned at %s\n", placeIdToName(location));
		return location;
}
