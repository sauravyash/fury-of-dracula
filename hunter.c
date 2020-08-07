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

// FUNCTION DEC:
PlaceId MapSleuth (HunterView hv);
PlaceId DraculaHunt (HunterView hv, PlaceId bestMove, Player current_player);
PlaceId VampHunt (HunterView hv, bool drac_found, Player current_player, PlaceId bestMove);


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

    // CHASING DRAC & VAMPS
    // So if any player is closest to drac make them hunt him and leave earlier
    // job, but don't make them leave their job just to follow the leader.
    int path = 0;
    PlaceId Drac_Loc = HvGetLastKnownDraculaLocation(hv, &path);
    PlaceId Vamp_Loc = HvGetVampireLocation(hv);
    bool drac_found = false;
    bool vamp_found = false;
    if (placeIsReal(Drac_Loc)) {
        bestMove = DraculaHunt (hv, bestMove, current_player);
        drac_found = true;
    }
    if (current_player == PLAYER_LORD_GODALMING || current_player == PLAYER_DR_SEWARD) {
        if (placeIsReal(Vamp_Loc)) {
            bestMove = VampHunt (hv, drac_found, current_player, bestMove);
            vamp_found = true;
        }
        if (drac_found == false && vamp_found == false) {
            // Select one for now... randomly:
            int len = 0;
		    PlaceId *possible_moves = HvWhereCanIGo(hv, &len);
		    if (len > 0) bestMove = possible_moves[1];
			free(possible_moves);
        }
    }

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
    // Find player closest to Drac.
    int temp_player = 0;
    int length = NUM_REAL_PLACES;
    int new_length = NUM_REAL_PLACES;
    while (temp_player < NUM_PLAYERS - 1) {
        PlaceId * path = HvGetShortestPathTo(hv, temp_player, Drac_Loc, &new_length);
        if (new_length < length) {
            new_length = length;
            Leader = temp_player;
        }
        temp_player++;
		free(path);
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
		free(chaseDrac);

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
		free(followLead);
	}

	return bestMove;
}

PlaceId VampHunt (HunterView hv, bool drac_found, Player current_player, PlaceId bestMove) {

    // NOW FIND OUT WHERE VAMP IS FOR CHASING:
    // Basically, whoever is closest to Vamp becomes Leader...
    // (or if vamp is unknown, then Lord G)
    //Round vamp_round = -1;
    PlaceId Vamp_Loc = HvGetVampireLocation(hv);
    Player Leader;
    // Find player closest to Vamp.
    int temp_player = 0;
    int length = NUM_REAL_PLACES;
    int new_length = NUM_REAL_PLACES;
    int hunters_chasing = 0;
    if (drac_found == true) hunters_chasing = 1;
    else hunters_chasing = 2;
    temp_player = hunters_chasing - 1;
    while (temp_player >= 0) {
        PlaceId * path = HvGetShortestPathTo(hv, temp_player, Vamp_Loc, &new_length);
        if (new_length < length && new_length > 0) {
            new_length = length;
            Leader = temp_player;
        }
        temp_player--;
		free(path);
    }

    printf("Current leader is %d.\n", Leader);
	PlaceId Leader_Loc = HvGetPlayerLocation(hv, Leader);
	printf("All patrolling players move towards <%s>.\n", placeIdToName(Leader_Loc));

    if (current_player == Leader) {
		printf("I am the leader\n");
		// Basically try and move closer to dracula.
		int len = 0;
		PlaceId *chaseDrac = HvGetShortestPathTo(hv, current_player, Vamp_Loc, &len);
		if (len > 0) bestMove = chaseDrac[0];
		else bestMove = Leader_Loc;
		free(chaseDrac);

	} else {
		if (current_player == PLAYER_LORD_GODALMING && drac_found == true) {
		    // Don't follow after vamp..
		    return bestMove;
		}
		printf("I am a follower");
		// Basically move closer to the leader...
		int len = 0;
		PlaceId *followLead = HvGetShortestPathTo(hv, current_player, Leader_Loc, &len);
		if (len > 0) bestMove = followLead[0];
		else bestMove = Leader_Loc;
		free(followLead);
	}

	return bestMove;
}

PlaceId MapSleuth (HunterView hv) {

    int poss_places = 0;
	PlaceId returnValue;
    PlaceId *possible_places = HvWhereCanIGoByType(hv, true, false,
                             true, &poss_places);

    // Select one for now... randomly:
    if (poss_places > 1) {
		returnValue= possible_places[1];
    } else {
		returnValue = possible_places[0];
	}
	free(possible_places);
	return returnValue;
}
