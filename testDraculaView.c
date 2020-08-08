////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testDraculaView.c: test the DraculaView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-02	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////

		printf("Testing\n");

		char *trail =
			"GED.... SBD.... HPA.... MCD.... DGO.V.. GMN.... SVE.... HMA.... MCD.... DMRT... GLV.... SFL.... HAL.... MGA.... DCFT... GSW.... SVE.... HMA.... MKL.... DBOT... GLO.... SGOV... HSR.... MBE.... DNAT... GEC.... SMRT... HTO.... MSO.... DHIT... GNS.... SBU.... HSR.... MVA.... DD1T... GHA.... SST.... HAL.... MSO.... DLET... GBR.... SZU.... HLS.... MBE.... DBUT.M. GPR.... SMU.... HBA.... MSJ.... DAMT.M. GBD.... SVE.... HSR.... MBE.... DCOT.M. GKL.... SFL.... HMA.... MVR.... DFRT.M. GKL.... SFL.... HMA.... MVR.... DLIT.M. GSZ.... SGE.... HSN.... MCN.... DBR.VM. GVI.... SCF.... HLS.... MGA.... DHIT.M. GBE.... SMR.... HCA.... MBC.... DHAT.M. GSZ.... STO.... HGR.... MKL.... DCOT.M. GSZ.... STO.... HGR.... MKL.... DAMT.M. GBD.... SMR.... HMA.... MBE.... DBUT.M. GPR.... SMI.... HAL.... MSZ.... DLET.V. GVI.... SMU.... HBA.... MSA.... DNAT.M. GZA.... SZU.... HTO.... MSO.... DPAT.M. GZA.... SZU.... HTO.... MSO.... DHIT.M. GSJ.... SMI.... HSR.... MBE.... DCFT.M. GZA.... SFL.... HPATT.. MVR.... DTOT.M. GMU.... SZU.... HNAT... MVR.... DBAT.M. GMI.... SMR.... HBO.... MSO.... DD1.V.. GVE.... SCFT... HSR.... MBC.... DMS.... GMI.... SNA.... HLS.... MGA.... DCGT... GMR.... SBB.... HAO.... MCD.... DHIT... GMS.... SAO.... HIR.... MKL.... DTS..M. GAO.... SNS.... HAO.... MBE.... DGOT.M. GIR.... SAO.... HCA.... MBC.... DFLT.V. GLV.... SMS.... HLS.... MGA.... DROT... GLV.... SMS.... HLS.... MGA.... DBIT.M. GLO.... STS.... HSN.... MCD.... DHIT.M. GSW.... SNP.... HBB.... MKL.... DD1T... GLV.... SFLT... HSN.... MBD.... DNPT.M. GLO.... SBITTT. HBB.... MVI.... DTS.... GEC.... SAS.... HNA.... MVI.... DCG.VM. GNS.... SAS.... HPA.... MBD.... DMS.... GHA.... SBI.... HMA.... MKL.... DBAT... GPR.... SFL.... HSN.... MCD.... DTOT... GHA.... SVE.... HMA.... MGA.... DBOT.M. GNS.... SMI.... HBOTD.. MSZ.... DCFT... GEC.... SGE.... HZA.... MJM.... DNAT.V. GLE.... SPA.... HMU.... MZA.... DHIT... GBU.... SNATT.. HST.... MMU.... DD1T.M. GPA.... SZA.... HPA.... MST.... DBB..M. GBO.... SVI.... HBO.... MPA.... DSNT... GBB.... SHA.... HBB.... MLE.... DLST.M. GAO.... SCO.... HAO.... MPA.... DMAT... GIR.... SST.... HCA.... MMR.... DAL.V.. GLV.... SMI.... HLST... MZU.... DGRT.M. GIR.... SST.... HMAT... MMI.... DHIT... GAO.... SZU.... HSNT... MMU.... DD1T... GBB.... SGE.... HALV... MLI.... DCAT... GNA.... SGO.... HBO.... MNU.... DLST... GPA.... SST.... HSR.... MMU.... DAO.... GLE.... SZU.... HMA.... MZA.... DGWT.M. GEC.... SMI.... HBA.... MSZ.... DDUT.M. GLO.... SFL.... HMA.... MKL.... DHIT.M. GSW.... SNP.... HSN.... MCD.... DD1T.M. GSW.... SNP.... HSN.... MCD.... DIR..M. GLV.... SBI.... HMA.... MGA.... DAO.... GMN.... SRO.... HLS.... MBC.... DGW.VM. GED.... SFL.... HCA.... MKL....";

		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		printf("drac location is %s\n", placeIdToName(DvGetPlayerLocation(dv,PLAYER_DRACULA)));
		int numLocs = -1;
		printf("=====================================================\n");
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		printf("%d\n", numLocs);
		printf("==============================legal moves moves\n");
		for (int i = 0; i < numLocs; i++) {
			printf("%s\n", placeIdToName(locs[i]));
		}
		printf("===============\n");
		bool canFree = false;
		int number = -1;
		printf("past locs:\n");
		PlaceId *pastLocs = DvGetLastLocations(dv, PLAYER_DRACULA,6,&number, &canFree);
		for (int i = 0; i < number; i ++) {
			printf("%s\n",placeIdToName(pastLocs[i]));
		}
		number = 6;
		printf("past moves:\n");
		PlaceId *pastMoves = DvGetLastMoves(dv, PLAYER_DRACULA,6,&number, &canFree);
		for (int i = 0; i < number; i ++) {
			printf("%s\n",placeIdToName(pastMoves[i]));
		}
		//assert(numLocs == 4);
		//sortPlaces(locs, numLocs);
		//assert(locs[0] == BELGRADE);
		//assert(locs[1] == CONSTANTA);
		//assert(locs[2] == GALATZ);
		//assert(locs[3] == SOFIA);

		free(locs);

		printf("Test passed!\n");
		DvFree(dv);
	}



	{///////////////////////////////////////////////////////////////////

		printf("Test for basic functions, "
			   "just before Dracula's first move\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";

		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "..."
		};

		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 0);
		assert(DvGetScore(dv) == GAME_START_SCORE);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == ZURICH);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////

		printf("Test for encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD.. SAO.... HCD.... MAO....";

		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahah",
			"Aha!", "", "", ""
		};

		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 1);
		assert(DvGetScore(dv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////

		printf("Test for Dracula leaving minions 1\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST....";

		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 4);
		assert(DvGetVampireLocation(dv) == EDINBURGH);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == LONDON);
		assert(traps[1] == LONDON);
		assert(traps[2] == MANCHESTER);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////

		printf("Test for Dracula's valid moves 1\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE....";

		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);

		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		assert(numMoves == 4);
		sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
		assert(moves[1] == KLAUSENBURG);
		assert(moves[2] == HIDE);
		assert(moves[3] == DOUBLE_BACK_1);
		free(moves);

		printf("Test passed!\n");
		DvFree(dv);
	}




	return EXIT_SUCCESS;
}
