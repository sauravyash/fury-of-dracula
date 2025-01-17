////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testGameView.c: test the GameView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////

		printf("Basic initialisation\n");

		char *trail = "";
		Message messages[] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 0);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == NOWHERE);
		assert(GvGetVampireLocation(gv) == NOWHERE);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("After Lord Godalming's turn\n");

		char *trail =
			"GST....";

		Message messages[1] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 0);
		assert(GvGetPlayer(gv) == PLAYER_DR_SEWARD);
		assert(GvGetScore(gv) == GAME_START_SCORE);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) == NOWHERE);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("After Mina Harker's turn\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";

		Message messages[4] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 0);
		assert(GvGetPlayer(gv) == PLAYER_DRACULA);
		assert(GvGetScore(gv) == GAME_START_SCORE);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(GvGetPlayerLocation(gv, PLAYER_VAN_HELSING) == ZURICH);
		assert(GvGetPlayerLocation(gv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == NOWHERE);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("After Dracula's turn\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB.... DC?.V..";

		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahahaha"
		};

		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 1);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(GvGetPlayerLocation(gv, PLAYER_VAN_HELSING) == ZURICH);
		assert(GvGetPlayerLocation(gv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
		assert(GvGetVampireLocation(gv) == CITY_UNKNOWN);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD..";

		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahahaha",
			"Aha!"
		};

		GameView gv = GvNew(trail, messages);

		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) ==
				GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_DRACULA_ENCOUNTER);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS - LIFE_LOSS_HUNTER_ENCOUNTER);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == GENEVA);
		assert(GvGetVampireLocation(gv) == NOWHERE);

		GvFree(gv);
		printf("Test passed\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Test for Dracula doubling back at sea, "
		       "and losing blood points (Hunter View)\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DS?.... "
			"GST.... SST.... HST.... MST.... DD1....";

		Message messages[] = {
			"Party at Geneva", "Okay", "Sure", "Let's go", "Mwahahahaha",
			"", "", "", "", "Back I go"
		};

		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 2);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE - 2 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS - (2 * LIFE_LOSS_SEA));
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == SEA_UNKNOWN);

		GvFree(gv);
		printf("Test passed!\n");
	}


	{///////////////////////////////////////////////////////////////////

		printf("Test for Dracula doubling back at sea, "
			   "and losing blood points (Dracula View)\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DEC.... "
			"GST.... SST.... HST.... MST.... DD1.... "
			"GFR.... SFR.... HFR.... MFR....";

		Message messages[14] = {
			"Hello", "Rubbish", "Stuff", "", "Mwahahah",
			"Aha!", "", "", "", "Back I go"};

		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 2);
		assert(GvGetPlayer(gv) == PLAYER_DRACULA);
		assert(GvGetScore(gv) == GAME_START_SCORE - 2 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS - (2 * LIFE_LOSS_SEA));
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == ENGLISH_CHANNEL);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Checking that hunters' health points are capped\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE....";

		Message messages[6] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing a hunter 'dying'\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE.... SGE.... HGE.... MGE.... DSTT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GSTTTTD";

		Message messages[21] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetScore(gv) == GAME_START_SCORE
		                         - 4 * SCORE_LOSS_DRACULA_TURN
		                         - SCORE_LOSS_HUNTER_HOSPITAL);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == 0);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == HOSPITAL_PLACE);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == STRASBOURG);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing Dracula doubling back to Castle Dracula\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T...";

		Message messages[10] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS + (2 * LIFE_GAIN_CASTLE_DRACULA));
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CASTLE_DRACULA);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing vampire/trap locations\n");

		char *trail =
			"GVI.... SGE.... HGE.... MGE.... DCD.V.. "
			"GBD.... SGE.... HGE.... MGE.... DKLT... "
			"GSZ.... SGE.... HGE.... MGE.... DGAT... "
			"GSZ.... SGE.... HGE.... MGE....";

		Message messages[19] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == GALATZ);
		assert(GvGetVampireLocation(gv) == CASTLE_DRACULA);
		int numTraps = 0;
		PlaceId *traps = GvGetTrapLocations(gv, &numTraps);
		assert(numTraps == 2);
		sortPlaces(traps, numTraps);
		assert(traps[0] == GALATZ && traps[1] == KLAUSENBURG);
		free(traps);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing trap locations after one is destroyed\n");

		char *trail =
			"GVI.... SGE.... HGE.... MGE.... DBC.V.. "
			"GBD.... SGE.... HGE.... MGE.... DKLT... "
			"GSZ.... SGE.... HGE.... MGE.... DGAT... "
			"GBE.... SGE.... HGE.... MGE.... DCNT... "
			"GKLT... SGE.... HGE.... MGE....";

		Message messages[24] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) ==
				GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_TRAP_ENCOUNTER);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == KLAUSENBURG);
		assert(GvGetVampireLocation(gv) == BUCHAREST);
		int numTraps = 0;
		PlaceId *traps = GvGetTrapLocations(gv, &numTraps);
		assert(numTraps == 2);
		sortPlaces(traps, numTraps);
		assert(traps[0] == CONSTANTA && traps[1] == GALATZ);
		free(traps);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing a vampire maturing\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T.V.";

		Message messages[35] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetScore(gv) == GAME_START_SCORE
		                         - 7 * SCORE_LOSS_DRACULA_TURN
		                         - SCORE_LOSS_VAMPIRE_MATURES);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
		assert(GvGetVampireLocation(gv) == NOWHERE);

		GvFree(gv);
		printf("Test passed!\n");
	}


	{///////////////////////////////////////////////////////////////////

		printf("Testing move/location history\n");

		char *trail =
			"GLS.... SGE.... HGE.... MGE.... DST.V.. "
			"GCA.... SGE.... HGE.... MGE.... DC?T... "
			"GGR.... SGE.... HGE.... MGE.... DC?T... "
			"GAL.... SGE.... HGE.... MGE.... DD3T... "
			"GSR.... SGE.... HGE.... MGE.... DHIT... "
			"GSN.... SGE.... HGE.... MGE.... DC?T... "
			"GMA.... SSTTTV.";

		Message messages[32] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) ==
				GAME_START_HUNTER_LIFE_POINTS - 2 * LIFE_LOSS_TRAP_ENCOUNTER);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
		assert(GvGetVampireLocation(gv) == NOWHERE);

		// Lord Godalming's move/location history
		{
			int numMoves = 0; bool canFree = false;
			PlaceId *moves = GvGetMoveHistory(gv, PLAYER_LORD_GODALMING,
			                                  &numMoves, &canFree);
			assert(numMoves == 7);
			assert(moves[0] == LISBON);
			assert(moves[1] == CADIZ);
			assert(moves[2] == GRANADA);
			assert(moves[3] == ALICANTE);
			assert(moves[4] == SARAGOSSA);
			assert(moves[5] == SANTANDER);
			assert(moves[6] == MADRID);
			if (canFree) free(moves);
		}

		// Dracula's move/location history
		{
			int numMoves = 0; bool canFree = false;
			PlaceId *moves = GvGetMoveHistory(gv, PLAYER_DRACULA,
			                                  &numMoves, &canFree);
			assert(numMoves == 6);
			assert(moves[0] == STRASBOURG);
			assert(moves[1] == CITY_UNKNOWN);
			assert(moves[2] == CITY_UNKNOWN);
			assert(moves[3] == DOUBLE_BACK_3);
			assert(moves[4] == HIDE);
			assert(moves[5] == CITY_UNKNOWN);
			if (canFree) free(moves);
		}

		{
			int numLocs = 0; bool canFree = false;
			PlaceId *locs = GvGetLocationHistory(gv, PLAYER_DRACULA,
			                                     &numLocs, &canFree);
			assert(numLocs == 6);
			assert(locs[0] == STRASBOURG);
			assert(locs[1] == CITY_UNKNOWN);
			assert(locs[2] == CITY_UNKNOWN);
			assert(locs[3] == STRASBOURG);
			assert(locs[4] == STRASBOURG);
			assert(locs[5] == CITY_UNKNOWN);
			if (canFree) free(locs);
		}

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing connections\n");

		char *trail = "";
		Message messages[] = {};
		GameView gv = GvNew(trail, messages);

		{
			printf("\tChecking Galatz road connections "
			       "(Lord Godalming, Round 1)\n");
			int numLocs = -1;
			PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
			                                     1, GALATZ, true, false,
			                                     false, &numLocs);

			assert(numLocs == 5);
			sortPlaces(locs, numLocs);
			assert(locs[0] == BUCHAREST);
			assert(locs[1] == CASTLE_DRACULA);
			assert(locs[2] == CONSTANTA);
			assert(locs[3] == GALATZ);
			assert(locs[4] == KLAUSENBURG);
			free(locs);
		}

		{
			printf("\tChecking Ionian Sea boat connections "
			       "(Lord Godalming, Round 1)\n");

			int numLocs = -1;
			PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
			                                     1, IONIAN_SEA, false, false,
			                                     true, &numLocs);

			assert(numLocs == 7);
			sortPlaces(locs, numLocs);
			assert(locs[0] == ADRIATIC_SEA);
			assert(locs[1] == ATHENS);
			assert(locs[2] == BLACK_SEA);
			assert(locs[3] == IONIAN_SEA);
			assert(locs[4] == SALONICA);
			assert(locs[5] == TYRRHENIAN_SEA);
			assert(locs[6] == VALONA);
			free(locs);
		}

		{
			printf("\tChecking Paris rail connections "
			       "(Lord Godalming, Round 2)\n");
			int numLocs = -1;
			PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
			                                     2, PARIS, false, true,
			                                     false, &numLocs);

			assert(numLocs == 7);
			sortPlaces(locs, numLocs);
			assert(locs[0] == BORDEAUX);
			assert(locs[1] == BRUSSELS);
			assert(locs[2] == COLOGNE);
			assert(locs[3] == LE_HAVRE);
			assert(locs[4] == MARSEILLES);
			assert(locs[5] == PARIS);
			assert(locs[6] == SARAGOSSA);
			free(locs);
		}

		{
			printf("\tChecking Athens rail connections (none)\n");
			int numLocs = -1;
			PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
			                                     1, ATHENS, false, true,
			                                     false, &numLocs);

			assert(numLocs == 1);
			assert(locs[0] == ATHENS);
			free(locs);
		}
        // CustomTests
        {
            printf("\tChecking Bari rail connections "
			       "(Lord Godalming, Round 3)\n");
			int numLocs = -1;
			PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
			                                     3, BARI, false, true,
			                                     false, &numLocs);

			assert(numLocs == 4);
			sortPlaces(locs, numLocs);
			assert(locs[0] == BARI);
			assert(locs[1] == FLORENCE);
			assert(locs[2] == NAPLES);
			assert(locs[3] == ROME);
			free(locs);
        }

        {
            printf("\tChecking Bordeaux rail connections "
			       "(Lord Godalming, Round 3)\n");
			int numLocs = -1;
			PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
			                                     3, BORDEAUX, false, true,
			                                     false, &numLocs);

			assert(numLocs == 12);
			sortPlaces(locs, numLocs);
			assert(locs[0] == ALICANTE);
			assert(locs[1] == BARCELONA);
			assert(locs[2] == BORDEAUX);
			assert(locs[3] == BRUSSELS);
			assert(locs[4] == COLOGNE);
			assert(locs[5] == LE_HAVRE);
			assert(locs[6] == LISBON);
			assert(locs[7] == MADRID);
			assert(locs[8] == MARSEILLES);
			assert(locs[9] == PARIS);
			assert(locs[10] == SANTANDER);
			assert(locs[11] == SARAGOSSA);

			free(locs);
        }

		GvFree(gv);
		printf("Test passed!\n");
	}

    printf("|| CUSTOM TESTS ||\n");
	{///////////////////////////////////////////////////////////////////

		printf("Testing a hunter resting/staying in same city"
                " with no life point lost.\n");

		char *trail =
			"GGE.... SGE.... HGE.... MST.... DC?.V.. "
			"GLO.... SST.... HBE.... MGE.... DC?T... "
			"GGE.... SST.... HGE.... MSZ.... DC?T... ";


		Message messages[35] = {};
		GameView gv = GvNew(trail, messages);
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) ==
				GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_VAN_HELSING) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_MINA_HARKER) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetScore(gv) == GAME_START_SCORE
								 - 3 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing a hunter resting/staying in same"
                " city after trap encounter\n");

		char *trail =
			"GGE.... SGE.... HGE.... MST.... DC?.V.. "
			"GLO.... SST.... HBEV... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MSZ.... DC?T... "
			"GGE.... SST.... HGE.... MSZ.... DC?T... ";

		Message messages[35] = {};
		GameView gv = GvNew(trail, messages);
		// loses 2 health, gains 3
		assert(GvGetHealth(gv, PLAYER_VAN_HELSING) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_MINA_HARKER) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetScore(gv) == GAME_START_SCORE
								 - 4 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing a hunter resting/staying in same city after trap encounter\n");

		char *trail =
			"GGE.... SGE.... HGE.... MST.... DC?.V.. "
			"GLO.... SST.... HBEV... MGE.... DC?T... "
			"GGED... SGE.... HGE.... MSZ.... DC?T... "
			"GGE.... SST.... HGE... MSZ.... DC?T... ";

		Message messages[35] = {};
		GameView gv = GvNew(trail, messages);
		// loses 2 health, gains 3
		assert(GvGetHealth(gv, PLAYER_VAN_HELSING) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) ==
                GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_DRACULA_ENCOUNTER + LIFE_GAIN_REST);
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_MINA_HARKER) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetScore(gv) == GAME_START_SCORE
								 - 4 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);

		GvFree(gv);
		printf("Test passed!\n");
	}

	{///////////////////////////////////////////////////////////////////

		printf("Testing a hunter resting/staying in same "
                "city after dracula encounter\n");

		char *trail =
			"GGE.... SGE.... HGE.... MST.... DC?.V.. "
			"GLO.... SST.... HBEV... MGE.... DC?T... "
			"GGED... SGE.... HGE.... MSZ.... DC?T... "
			"GGE.... SSTD...";

		Message messages[35] = {};
		GameView gv = GvNew(trail, messages);
		//loses 2 health, gaines 3
		assert(GvGetHealth(gv, PLAYER_VAN_HELSING) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_DRACULA_ENCOUNTER + LIFE_GAIN_REST);
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) <= GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_DRACULA_ENCOUNTER);
		assert(GvGetHealth(gv, PLAYER_MINA_HARKER) <= GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetScore(gv) == GAME_START_SCORE
								 - 3 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);

		GvFree(gv);
		printf("Test passed!\n");
	}
    {///////////////////////////////////////////////////////////////////////
        printf("Check decremented game score each time Dracula finishes his turn.\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB.... DC?.V..";

		Message messages[5] = {};
		GameView gv = GvNew(trail, messages);
        assert(GvGetScore(gv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
        GvFree(gv);
        printf("Test Passed!\n");
    }
    { //////////////////////////////////////////////////////////////////////
        printf("Check Dracula is permitted to exceed 40 blood points.\n");
        char *trail =
			"GST.... SLV.... HLS.... MGO.... DKL.V.."
            "GBU.... SLV.... HLS.... MGO.... DCD.V..";

		Message messages[5] = {};
		GameView gv = GvNew(trail, messages);
        assert(GvGetHealth(gv, PLAYER_DRACULA)
                == GAME_START_BLOOD_POINTS + LIFE_GAIN_CASTLE_DRACULA);
        printf("Test Passed\n");
		GvFree(gv);
    }
    { //////////////////////////////////////////////////////////////////////
        printf("A hunter gains 3 life points each time they rest\n");
        char *trail =
			"GLS.... SGE.... HGE.... MGE.... DST.V.. "
			"GCAD... SGE.... HGE.... MGE.... DC?T... "
			"GCA.... SGE.... HGE.... MGE.... DC?T... ";

        Message messages[5] = {};
		GameView gv = GvNew(trail, messages);

        assert(GvGetHealth(gv, PLAYER_LORD_GODALMING)
                    == GAME_START_HUNTER_LIFE_POINTS
                        - LIFE_LOSS_DRACULA_ENCOUNTER + LIFE_GAIN_REST);
        GvFree(gv);
        printf("Test Passed\n");
    }
    { //////////////////////////////////////////////////////////////////////
        printf("Check that immature vampires can be killed within "
                "6 rounds of being spawned.\n");
        char *trail =
			"GZU.... SGE.... HGE.... MGE.... DST.V.. "
			"GZU.... SGE.... HGE.... MGE.... DC?T... "
			"GZU.... SGE.... HGE.... MGE.... DC?T... "
			"GZU.... SGE.... HGE.... MGE.... DC?T... "
			"GZU.... SGE.... HGE.... MGE.... DC?T... "
			"GSTV... SGE.... HGE.... MGE.... DC?T... ";

        Message messages[5] = {};
		GameView gv = GvNew(trail, messages);

        assert(GvGetScore(gv)
                    == GAME_START_SCORE
                        - 6 * SCORE_LOSS_DRACULA_TURN);
        GvFree(gv);
        printf("Test Passed\n");
    }
	{ //////////////////////////////////////////////////////////////////////
        printf("Test Road, Rail and Sea moves. \n");
        char *trail =
			"GZU.... SFR.... HGE.... MLE.... ";

        Message messages[5] = {};
		GameView gv = GvNew(trail, messages);

        int *numPlaces = malloc(sizeof(int));
        PlaceId *places =
            GvGetReachable(gv, PLAYER_MINA_HARKER, 0, LE_HAVRE, numPlaces);
        bool foundRoad = false,
             foundSea = false,
             foundRail = false;

        for (int i = 0; i < *numPlaces; i++) {
            if (places[i] == ENGLISH_CHANNEL) foundSea = true;
            if (places[i] == NANTES) foundRoad = true;
            if (places[i] == COLOGNE) foundRail = true;
        }
		free(places);
		free(numPlaces);
        assert(foundSea && foundRoad && foundRail);
        GvFree(gv);
        printf("Test Passed\n");
    }
    {
        printf("Test max of 3 encounters @ a city \n");
        char *trail =
			"GZU.... SGE.... HGE.... MGE.... DST.V.. "//st
			"GZU.... SGE.... HGE.... MGE.... DFRT... "
			"GZU.... SGE.... HGE.... MGE.... DLIT... "
			"GZU.... SGE.... HGE.... MGE.... DCOT... "
			"GZU.... SGE.... HGE.... MGE.... DBUT... "
			"GSTV... SGE.... HGE.... MGE.... DPAT... "
			"GZU.... SGE.... HGE.... MGE.... DSTT... " //st
			"GZU.... SGE.... HGE.... MGE.... DFRT... "
			"GZU.... SGE.... HGE.... MGE.... DLIT... "
			"GZU.... SGE.... HGE.... MGE.... DCOT... "
			"GZU.... SGE.... HGE.... MGE.... DBUT... "
			"GST.... SGE.... HGE.... MGE.... DPAT... "
			"GZU.... SGE.... HGE.... MGE.... DSTT... " //st
			"GZU.... SGE.... HGE.... MGE.... DFR.V.. "
			"GZU.... SGE.... HGE.... MGE.... DLIT... "
			"GSTV... SGE.... HGE.... MGE.... DCOT... "
			"GZU.... SGE.... HGE.... MGE.... DBUT... "
			"GZU.... SGE.... HGE.... MGE.... DPAT... "
            "GZU.... SGE.... HGE.... MGE.... DSTT... "
            "GST.... "; // st but no encounter should be here

        Message messages[19*5] = {};
		GameView gv = GvNew(trail, messages);
        int score = GAME_START_SCORE - 19 * SCORE_LOSS_DRACULA_TURN;

        assert(GvGetScore(gv) == score);
        assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);
		GvFree(gv);
    }

    printf("ALL PROVIDED TESTS PASSED!!!\n");





{///////////////////////////////////////////////////////////////////

	printf("testing a round \n");

	char *trail =
		"GPA.... SBR.... HMA.... MCD.... DZU.V.. GST.... SPR.... HSR.... MCD.... DMUT... GZUV... SBD.... HTO.... MKL.... DC?T... GMUT... SKL.... HSR.... MSZ.... DC?T... GZA.... SSZ.... HTO.... MZA.... DD2T... GZA.... SZA.... HSR.... MMU.... DC?T... GMU.... SMU.... HTO.... MMU.... DC?T... GMU.... SMU.... HSR.... MMU.... DMRT... GMU.... SMU.... HTO.... MMU.... DTOT.M. GMU.... SMU.... HSR.... MZU.... DHIT.M. GZU.... SZU.... HTOTTD. MMRT... DC?T.M. GMR.... SMR.... HSR.... MMR.... DSRT.M. GMR.... SMR.... HTO.... MSRTD.. DD1T.M. GTO.... STO.... HSRT... MSRD...";

	Message messages[20] = {};
	GameView gv = GvNew(trail, messages);

	printf("M health is %d\n", GvGetHealth(gv, PLAYER_MINA_HARKER) );
	printf("M  is  at %s\n", placeIdToName(GvGetPlayerLocation(gv, PLAYER_MINA_HARKER) ));

	GvFree(gv);
	printf("Test passed!\n");
	return EXIT_SUCCESS;
}
}
