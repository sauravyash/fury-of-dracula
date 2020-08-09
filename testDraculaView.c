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
			"GPA.... SBR.... HMA.... MKL.... DGO.V.. GPA.... SPR.... HMA.... MKL.... DFLT... GPA.... SPR.... HMA.... MCD.... DROT... GST.... SVI.... HMA.... MGA.... DBIT... GZU.... SZA.... HSR.... MGA.... DNPT... GZU.... SSZ.... HTO.... MGA.... DHIT... GZU.... SJM.... HMR.... MGA.... DD1T.V. GST.... SSJ.... HMS.... MGA.... DTS..M. GST.... SVA.... HMS.... MGA.... DGOT.M. GST.... SVA.... HTS.... MKL.... DFLT.M. GNU.... SIO.... HRO.... MSZ.... DVET.M. GST.... STS.... HNPTT.. MZA.... DMIT... GZU.... SRO.... HBI.... MVI.... DHIT... GST.... SNP.... HAS.... MPR.... DMR.V.. GZU.... SBI.... HVET... MNU.... DCFT.M. GST.... SAS.... HMU.... MST.... DBOT.M. GZU.... SVE.... HZU.... MMU.... DNAT... GST.... SMU.... HST.... MZU.... DLET.M. GZU.... SZU.... HPA.... MMU.... DHIT.M. GMU.... SST.... HPA.... MVE.... DD1T.V. GZA.... SPA.... HNAT... MAS.... DEC..M. GVI.... SNA.... HBB.... MVE.... DLOT.M. GZA.... SBB.... HSN.... MMI.... DMNT... GVI.... SSN.... HSR.... MMR.... DEDT.M. GZA.... SSR.... HTO.... MZU.... DHIT.M. GVI.... STO.... HMR.... MMU.... DD1T.M. GZA.... SMR.... HMI.... MZA.... DNS.... GVI.... SMS.... HGO.... MJM.... DAMT.M. GPR.... SCG.... HTS.... MZA.... DBUT.M. GVI.... STS.... HRO.... MSJ.... DCOT.M. GBD.... SIO.... HNP.... MVA.... DFRT.M. GSZ.... SVA.... HBI.... MIO.... DLIT.M. GJM.... SIO.... HAS.... MTS.... DBRT... GZA.... SVA.... HVE.... MRO.... DHAT.M. GSJ.... SIO.... HMU.... MFL.... DHIT.M. GZA.... SVA.... HZU.... MGO.... DCOT.M. GMU.... SIO.... HST.... MTS.... DAMT.M. GZU.... SIO.... HNU.... MIO.... DBUT.M. GST.... SIO.... HPR.... MSA.... DLET.M. GPA.... SSA.... HVI.... MSO.... DD1.VM. GNA.... SSA.... HZA.... MVR.... DHIT.M. GBB.... SSO.... HSZ.... MVR.... DPAT.M. GSN.... SVR.... HJM.... MBS.... DCFT.M. GSR.... SVR.... HSJ.... MCN.... DGET.M. GTO.... SVR.... HVA.... MGA.... DSTT.M. GMR.... SBS.... HIO.... MGA.... DBUT.V. GMS.... SCN.... HSA.... MKL.... DAMT.M. GTS.... SCN.... HSO.... MSZ.... DCOT.M. GRO.... SCN.... HVR.... MZA.... DFRT.M. GNP.... SCN.... HBS.... MVI.... DLIT.M. GBI.... SCN.... HCN.... MPR.... DHIT.M. GAS.... SCN.... HGA.... MNU.... DD1T.M. GVE.... SCN.... HKL.... MST.... DBR.VM. GMI.... SCN.... HCD.... MZU.... DHAT.M. GGO.... SGA.... HCD.... MMU.... DCOT.M. GFL.... SGA.... HKL.... MVE.... DAMT.M. GRO.... SGA.... HBD.... MAS.... DBUT.M. GTS.... SGA.... HZA.... MVE.... DLET.M. GIO.... SGA.... HVI.... MMI.... DNAT.V. GVA.... SGA.... HPR.... MMR.... DHIT.M. GSO.... SGA.... HNU.... MZU.... DBOT.M. GSO.... SKL.... HST.... MMU.... DCFT.M. GSO.... SSZ.... HZU.... MZA.... DPAT.M. GSO.... SZA.... HMU.... MJM.... DBUT.M. GSO.... SVI.... HVE.... MSJ.... DAMT.M. GSO.... SPR.... HAS.... MVA.... DCO.VM. GSO.... SNU.... HIO.... MIO.... DHAT.M. GSO.... SST.... HVA.... MTS.... DBRT.M. GSO.... SZU.... HSO.... MRO.... DLIT.M. GSO.... SMU.... HVR.... MFL.... DFRT.M. GVR.... SVE.... HVR.... MGO.... DNUT.M. GBS.... SAS.... HVR.... MVE.... DPRT.V. GBS.... SIO.... HBS.... MAS.... DVIT.M. GBS.... SVA.... HCN.... MBI.... DBDT.M. GCN.... SSO.... HCN.... MNP.... DKLT.M. GGA.... SVR.... HGA.... MTS.... DBET.M. GGA.... SBS.... HKLT... MIO.... DSJT.M. GKL.... SCN.... HSZ.... MVA.... DHIT.M. GSZ.... SBC.... HZA.... MIO.... DD1.VM. GZA.... SKL.... HVI.... MVA.... DSOT.M. GVI.... SGA.... HPR.... MIO.... DVRT... GPR.... SCD.... HNU.... MSA.... DCNT.M. GNU.... SKL.... HST.... MSOT... DBS..M. GST.... SSZ.... HZU.... MVRT... DIO..M. GZU.... SZA.... HMU.... MBS.... DD2..V. GST.... SVI.... HVE.... MCNT... DTPT... GZU.... SPR.... HAS.... MGA.... DKLT... GST.... SNU.... HIO.... MKLTD.. DBCT... GZU.... SST.... HVA.... MSZ.... DCNT... GST.... SZU.... HSO.... MKL.... DGAT... GZU.... SMU.... HVR.... MCDT... DCDT... GST.... SVE.... HBS.... MGAT... DD5.V.. GZU.... SAS.... HCNT... MCDT... DBET... GST.... SIO.... HBCT... MKLV... DSJT... GZU.... SVA.... HKL.... MBD.... DHIT... GST.... SSO.... HGA.... MZA.... DVAT... GZU.... SVR.... HCD.... MVI.... DATT... GST.... SBS.... HCD.... MPR.... DD1T... GZU.... SCN.... HKL.... MNU.... DIO..M. GST.... SGA.... HSZ.... MST.... DSAT.M. GST.... SKL.... HZA.... MMU.... DSOT.M. GST.... SCD.... HVI.... MMU.... DBET.M. GZU.... SCD.... HPR.... MZU.... DBCT.M. GST.... SKL.... HNU.... MMI.... DCNT.M. GZU.... SSZ.... HST.... MVE.... DGA.V.. GST.... SZA.... HZU.... MAS.... DCDT.M. GZU.... SVI.... HMU.... MVE.... DD2T.M. GST.... SPR.... HVE.... MGO.... DKLT.M. GPA.... SNU.... HAS.... MTS.... DBET.M. GST.... SST.... HIO.... MIO.... DBCT.M. GZU.... SZU.... HVA.... MVA.... DCNT.V. GST.... SMU.... HSO.... MSO.... DGAT.M. GZU.... SVE.... HVR.... MVR.... DCDT.M. GST.... SAS.... HBS.... MCNT... DD2T.M. GZU.... SIO.... HCN.... MGATT.. DKLT.M. GST.... SVA.... HGA.... MSZ.... DHIT.M. GZU.... SSO.... HKLTTD. MKLD... DBDT... GST.... SVR.... HSJ.... MCDT... DVI.V.. GZU.... SBS.... HBE.... MGA.... DPRT... GST.... SCN.... HSZ.... MBC.... DBRT... GZU.... SGA.... HZA.... MBE.... DHAT... GST.... SKL.... HVIV... MBE.... DLIT... GZU.... SCD.... HPRT... MSJ.... DHIT.M. GST.... SKL.... HNU.... MSJ.... DD1T... GNU.... SSZ.... HST.... MZA.... DCOT... GPR.... SZA.... HZU.... MVI.... DAMT.M. GVI.... SVI.... HST.... MPR.... DNS..M. GZA.... SPR.... HPA.... MNU.... DEDT.M. GSZ.... SNU.... HST.... MST.... DMNT.M. GZA.... SST.... HMU.... MZU.... DLVT.M. GJM.... SZU.... HVE.... MMU.... DSW.VM. GZA.... SST.... HAS.... MVE.... DLOT.M. GVI.... SZU.... HIO.... MAS.... DPLT... GMU.... SMU.... HVA.... MVE.... DHIT.M. GVE.... SVE.... HIO.... MMI.... DD1T.M. GAS.... SAS.... HTS.... MMR.... DEC..M. GIO.... SIO.... HRO.... MZU.... DLET.V. GVA.... SVA.... HNP.... MMU.... DBUT.M. GIO.... SSO.... HBI.... MZA.... DAMT.M. GIO.... SSO.... HAS.... MSZ.... DCOT.M. GIO.... SVR.... HVE.... MJM.... DFRT.M. GIO.... SBS.... HMI.... MBE.... DLIT... GIO.... SCN.... HMR.... MSO.... DBRT.M. GIO.... SCN.... HMS.... MVR.... DHA.VM. GTS.... SGA.... HMS.... MBS.... DHIT.M. GRO.... SKL.... HCG.... MIO.... DCOT.M. GRO.... SSZ.... HTS.... MVA.... DAMT.M. GRO.... SZA.... HGO.... MVA.... DBUT.M. GNP.... SVI.... HFL.... MSJ.... DLET.M. GNP.... SPR.... HRO.... MZA.... DNAT.V. GNP.... SNU.... HNP.... MVI.... DBOT.M. GBI.... SST.... HNP.... MPR.... DCFT.M. GAS.... SZU.... HNP.... MNU.... DPAT.M. GVE.... SMU.... HNP.... MST.... DHIT.M. GMU.... SVE.... HNP.... MPATTD. DSTT.M. GZU.... SAS.... HBI.... MSTT... DMUT.M. GST.... SVE.... HAS.... MSZ.... DZA.VM. GPA.... SMI.... HVE.... MZAVD.. DD1T.M. GST.... SMR.... HMUT... MSZ.... DSJT... GNU.... SMS.... HZU.... MKL.... DSOT... GPR.... STS.... HST.... MCD.... DBET... GVI.... SRO.... HPA.... MGA.... DHIT... GZAT... STS.... HNA.... MCN.... DKLT... GSZ.... SRO.... HBB.... MBS.... DBCT... GJM.... SNP.... HSN.... MVR.... DGAT.M. GSJ.... SBI.... HBB.... MSOT... DCDT... GVA.... SAS.... HSN.... MBCT...";

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
