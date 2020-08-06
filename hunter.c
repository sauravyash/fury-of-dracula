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


#define PLAYER1_START_POS 22
#define PLAYER2_START_POS 11
#define PLAYER3_START_POS 17
#define PLAYER4_START_POS 44

void decideHunterMove(HunterView hv)
{
	// First things first:
	Round round = HvGetRound(hv);
	//int current_score = HvGetScore(hv);
	Player current_player = HvGetPlayer(hv);
	
	// ARE YOU DEAD!!!
    
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

	PlaceId move = NOWHERE;
	
	if (round == 0) {
		if(current_player == PLAYER_LORD_GODALMING) move = PLAYER1_START_POS;
		else if(current_player == PLAYER_DR_SEWARD) move = PLAYER2_START_POS;
		else if(current_player == PLAYER_VAN_HELSING) move = PLAYER3_START_POS;
		else if(current_player == PLAYER_MINA_HARKER) move = PLAYER4_START_POS;
		else printf("I am noone..\n");
	} else if (current_player == Leader) {
		printf("I am the leader\n");
		// Basically try and move closer to dracula or a vamp.
		int len = 0;
		PlaceId *chaseDrac = HvGetShortestPathTo(hv, current_player, Drac_Loc, &len);
		if (len > 0) move = chaseDrac[0];
		else move = Leader_Loc;
	} else {
		printf("I am a follower");
		// Basically move closer to the leader...
		int len = 0;
		PlaceId *followLead = HvGetShortestPathTo(hv, current_player, Leader_Loc, &len);
		if (len > 0) move = followLead[0];
		else move = Leader_Loc;
	}
	
	printf("Next move is to <%s>.\n",placeIdToName(move));
	char *moveTo = malloc(2 * sizeof(char));
	strcpy(moveTo, placeIdToAbbrev(move));
	registerBestPlay(moveTo, "Lol we moving");
	
}
