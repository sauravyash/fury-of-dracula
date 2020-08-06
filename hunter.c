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

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "Places.h"


#define PLAYER1_START_POS 22
#define PLAYER2_START_POS 11
#define PLAYER3_START_POS 17
#define PLAYER4_START_POS 44

// FUNCTION DEC:
PlaceId MapSleuth (HunterView hv);
PlaceId DraculaHunt (HunterView hv, PlaceId bestMove, Player current_player);


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
        bestMove = MapSleuth (hv);
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
    
    if (current_player == Leader) {
		printf("I am the leader\n");
		// Basically try and move closer to dracula.
		int len = 0;
		PlaceId *chaseDrac = HvGetShortestPathTo(hv, current_player, Drac_Loc, &len);
		if (len > 0) bestMove = chaseDrac[0];
		else bestMove = Leader_Loc;
	
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
}

PlaceId MapSleuth (HunterView hv) {
    
    int poss_places;
    PlaceId *possible_places = HvWhereCanIGoByType(hv, true, false,
                             true, &poss_places);
    
    // Select one for now... randomly:
    if (poss_places > 1) return possible_places[1];
    else return possible_places[0];
}
