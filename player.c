////////////////////////////////////////////////////////////////////////
// Runs a player's game turn ...
//
// Can  produce  either a hunter player or a Dracula player depending on
// the setting of the I_AM_DRACULA #define
//
// This  is  a  dummy  version of the real player.c used when you submit
// your AIs. It is provided so that you can test whether  your  code  is
// likely to compile ...
//
// Note that this is used to drive both hunter and Dracula AIs. It first
// creates an appropriate view, and then invokes the relevant decideMove
// function,  which  should  use the registerBestPlay() function to send
// the move back.
//
// The real player.c applies a timeout, and will halt your  AI  after  a
// fixed  amount of time if it doesn 't finish first. The last move that
// your AI registers (using the registerBestPlay() function) will be the
// one used by the game engine. This version of player.c won't stop your
// decideMove function if it goes into an infinite loop. Sort  that  out
// before you submit.
//
// Based on the program by David Collien, written in 2012
//
// 2017-12-04	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.3	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#ifdef I_AM_DRACULA
# include "dracula.h"
# include "DraculaView.h"
//# include "DraculaTest.c"
#else
# include "hunter.h"
# include "HunterView.h"
//# include "HunterTest.c"
#endif

// Moves given by registerBestPlay are this long (including terminator)
#define MOVE_SIZE 3

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

// A pseudo-generic interface, which defines
// - a type `View',
// - functions `ViewNew', `decideMove', `ViewFree',
// - a trail `xtrail', and a message buffer `xmsgs'.
#ifdef I_AM_DRACULA

typedef DraculaView View;

# define ViewNew DvNew
# define decideMove decideDraculaMove
# define ViewFree DvFree

# define xPastPlays "GPA.... SBR.... HMA.... MKL.... DMI.V.. GPA.... SBR.... HSR.... MCD.... DGOT... GST.... SPR.... HTO.... MGA.... DVET... GST.... SPR.... HMR.... MCN.... DFLT... GST.... SPR.... HMR.... MCN.... DROT... GST.... SPR.... HMR.... MCN.... DBIT... GST.... SVI.... HMS.... MBS.... DNPT.V. GZU.... SZA.... HTS.... MVR.... DHIT.M. GMU.... SSZ.... HROT... MSO.... DD1T.M. GZA.... SJM.... HNPTTTD MBC.... DTS..M. GVI.... SZA.... HZA.... MSO.... DIO.... GZA.... SSJ.... HVI.... MVA.... DBS..M. GVI.... SZA.... HPR.... MIO.... DCNT... GPR.... SVI.... HNU.... MTS.... DGA.V.. GVI.... SMU.... HST.... MRO.... DCDT... GBD.... SZU.... HZU.... MTS.... DD2T... GSZ.... SST.... HMU.... MIO.... DHIT... GJM.... SZU.... HVE.... MBS.... DBCT... GSJ.... SST.... HAS.... MVR.... DKLT.M. GVA.... SZU.... HIO.... MCN.... DBET.V. GIO.... SST.... HVA.... MGATT.. DSZT.M. GTS.... SZU.... HSO.... MKLT... DZAT... GRO.... SMI.... HVR.... MSZT... DSJT... GTS.... SVE.... HBS.... MZAT... DSOT.M. GRO.... SAS.... HCN.... MZA.... DSAT... GTS.... SIO.... HGA.... MVI.... DHIT.M. GRO.... SVA.... HKL.... MPR.... DD1.V.. GTS.... SSOT... HSZ.... MNU.... DVAT... GNP.... SVR.... HBD.... MST.... DATT.M. GTS.... SBS.... HZA.... MNU.... DIO.... GRO.... SCN.... HVI.... MMU.... DAS..M. GTS.... SGA.... HPR.... MZU.... DBIT.M. GMS.... SKL.... HNU.... MMU...."

# define xMsgs { "", "", "", "" }

#else

typedef HunterView View;

# define ViewNew HvNew
# define decideMove decideHunterMove
# define ViewFree HvFree

# define xPastPlays "GPA.... SBR.... HMA.... MKL.... DC?.V.. GST.... SPR.... HMA.... MKL.... DC?T... GZU.... SVI.... HMA.... MKL.... DBUT... GZU.... SVI.... HSR.... MCD.... DC?T... GZU.... SZA.... HSR.... MGA.... DC?T... GZU.... SSZ.... HTO.... MCN.... DC?T... GZU.... SJM.... HMR.... MBS.... DHIT.V. GST.... SSJ.... HMS.... MBS.... DC?T.M. GST.... SSJ.... HMS.... MVR.... DC?T.M. GNU.... SVA.... HTS.... MBS.... DC?T.M. GPR.... SIO.... HRO.... MVR.... DC?T.M. GVI.... STS.... HNP.... MBS.... DC?T.M. GZA.... SRO.... HBI.... MVR.... DC?T.M. GSZ.... SNP.... HAS.... MBS.... DC?.VM. GJM.... SBI.... HVE.... MVR.... DC?T.M. GSJ.... SAS.... HMU.... MBS.... DSNT.M. GVA.... SVE.... HZU.... MIO.... DC?T.M. GIO.... SMU.... HST.... MVA.... DC?T.M. GVA.... SZU.... HPA.... MSO.... DC?T.M. GIO.... SST.... HNA.... MVR.... DC?T.V. GTS.... SPA.... HBB.... MBS.... DC?T.M. GRO.... SNA.... HSNT... MVR.... DHIT... GNP.... SBB.... HMA.... MBS.... DD1T.M. GBI.... SSN.... HSR.... MCN.... DS?..M. GAS.... SSR.... HTO.... MBS.... DC?T.M. GVE.... STO.... HMR.... MVR.... DC?T.M. GMU.... SMR.... HMI.... MBS.... DS?..M. GZU.... SMS.... HGO.... MVR.... DC?T.M. GST.... SCG.... HTS.... MBS.... DC?T.M. GPA.... STS.... HRO.... MVR.... DC?T... GNA.... SIO.... HNP.... MBS.... DHIT.M. GBB.... SVA.... HBI.... MIO.... DD1T.M. GSN.... SIO.... HAS.... MTS.... DS?.... GSR.... SVA.... HVE.... MRO.... DC?T.M. GTO.... SIO.... HMU.... MFL.... DC?T.M. GMR.... SSA.... HZU.... MVE.... DC?T.M. GMS.... SSA.... HST.... MAS.... DC?T.M. GCG.... SSA.... HPA.... MVE.... DC?T.M. GTS.... SSA.... HNA.... MMU.... DPRT... GIO.... SSA.... HBB.... MZU.... DNU.VM. GVA.... SSO.... HSN.... MST.... DC?T.M. GVA.... SSO.... HSR.... MNUV... DC?T.M. GIO.... SVR.... HMA.... MPRT... DC?T.M. GSA.... SBS.... HLS.... MVI.... DC?T.M. GIO.... SCN.... HAO.... MZA.... DC?T... GVA.... SGA.... HNS.... MSZ.... DC?T... GIO.... SKL.... HHA.... MJM.... DC?T.M. GVA.... SSZ.... HLI.... MSJ.... DC?T.M. GIO.... SZA.... HNU.... MVA.... DC?T.M. GVA.... SVI.... HPR.... MSO.... DC?T.M. GIO.... SPR.... HVI.... MVR.... DC?T.M. GVA.... SNU.... HZA.... MBS.... DFLT.M. GSO.... SST.... HSZ.... MVR.... DRO.VM. GVR.... SZU.... HJM.... MBS.... DC?T.M. GBS.... SMU.... HSJ.... MIO.... DNPT.M. GVR.... SVE.... HVA.... MTS.... DHIT.M. GBS.... SAS.... HIO.... MROV... DD1T.M. GVR.... SIO.... HTS.... MFLT... DS?.... GBS.... SVA.... HRO.... MVE.... DC?T... GVR.... SSJ.... HNPTTT. MAS.... DS?..M. GBS.... SZA.... HBI.... MIO.... DC?T... GCN.... SVI.... HAS.... MSA.... DC?T... GBS.... SZA.... HVE.... MVA.... DC?T... GIO.... SVI.... HMU.... MAT.... DC?T... GTS.... SZA.... HZU.... MIO.... DC?T.M. GRO.... SZA.... HZU.... MTS.... DSN.V.. GNP.... SVI.... HST.... MNP.... DSRT.M. GBI.... SZA.... HPA.... MBI.... DC?T.M. GAS.... SVI.... HNA.... MAS.... DC?T.M. GVE.... SPR.... HNA.... MVE.... DC?T.M. GMU.... SNU.... HBB.... MMU.... DC?T.M. GZU.... SST.... HSNV... MZU.... DHIT... GST.... SZU.... HSRT... MMU.... DD1T... GPA.... SMU.... HTO.... MZA.... DC?T.M. GNA.... SVE.... HMR.... MVI.... DC?T.M. GBB.... SAS.... HMS.... MPR.... DC?T.M. GSN.... SVE.... HTS.... MNU.... DC?T.M. GSR.... SAS.... HRO.... MST.... DC?T.M. GTO.... SVE.... HNP.... MPA.... DC?.VM. GMR.... SAS.... HBI.... MST.... DC?T.M. GMS.... SVE.... HAS.... MMU.... DC?T.M. GCG.... SMI.... HVE.... MMI.... DC?T.M. GTS.... SMR.... HAS.... MMR.... DC?T.M. GIO.... SMS.... HVE.... MZU.... DC?T.M. GVA.... STS.... HMU.... MGE.... DC?T.V. GIO.... SRO.... HZU.... MCF.... DC?T.M. GVA.... SNP.... HST.... MPA.... DSNT.M. GVA.... SBI.... HPA.... MST.... DHIT.M. GVA.... SBI.... HNA.... MMU.... DSRT.M. GVA.... SAS.... HBB.... MVE.... DC?T.M. GVA.... SIO.... HSNTT.. MAS.... DC?T.M. GIO.... SVA.... HSRT... MVE.... DC?.VM. GSA.... SSO...."

/*
    "GPA.... SBR.... HMA.... MKL.... DC?.V.. GST.... SPR.... HSR.... MSZ.... DC?T... " \
    "GZU.... SVI.... HTO.... MZA.... DC?T... GST.... SZA.... HSR.... MVI.... DC?T... " \
    "GZU.... SVI.... HTO.... MZA.... DC?T... GST.... SZA.... HSR.... MVI.... DEC.... " \
    "GZU.... SVI.... HTO.... MZA.... DC?T.V. GST.... SZA.... HSR.... MVI.... DC?T.M. " \
    "GZU.... SZA.... HSR.... MVI.... DC?T.M. GST.... SZA.... HSR.... MVI.... DC?T.M. " \
    "GZU.... SZA.... HSR.... MVI.... DC?T.M. GZU.... SVI.... HTO.... MZA.... DBUT... " \
    "GST.... SZA.... HSR.... MVI.... DC?T.M. GZU.... SVI.... HTO.... MZA.... DC?.VM. " \
    "GST.... SVI.... HTO.... MZA.... DC?T.M. GZU.... SVI.... HSR.... MZA.... DC?T.M. " \
    "GZU.... SVI.... HTO.... MZA.... DC?T.M. GZU.... SVI.... HTO.... MVI.... DHAT.M. " \
    "GST.... SZA.... HSR.... MZA.... DC?T.M. GZU.... SVI.... HTO.... MVI.... DD1T.V. " \
    "GST.... SVI.... HTO.... MVI.... DHIT.M. GZU.... SVI.... HTO.... MVI.... DC?T.M. " \
    "GST.... SVI.... HTO.... MVI.... DC?T.M. GST.... SZA.... HSR.... MZA.... DFRT.M. "\
    "GZU.... SVI.... HTO.... MVI.... DC?T.M. GST.... SZA.... HSR.... MZA.... DC?T.M. " \
    "GZU.... SVI.... HTO.... MVI.... DC?.VM. GST.... SZA.... HSR.... MZA.... DC?T.M. " \
    "GZU.... SZA.... HSR.... MZA.... DC?T.M. GZU.... SVI.... HTO.... MVI.... DFRT.M. " \
    "GST.... SZA.... HSR.... MZA.... DC?T.M. GZU.... SVI.... HTO.... MVI.... DC?T.M. " \
    "GST.... SVI.... HTO.... MVI.... DC?T.V. GZU.... SVI.... HTO.... MVI.... DS?..M. "\
    "GST.... SVI.... HTO.... MVI.... DC?T.M. GST.... SZA.... HSR.... MZA.... DMNT.M. " \
    "GZU.... SVI.... HTO.... MVI.... DC?T.M. GST.... SZA.... HSR.... MZA.... DC?T.M. " \
    "GZU.... SZA.... HSR.... MZA.... DC?T.M. GST.... SZA.... HSR.... MZA.... DC?.V.. " \
    "GZU.... SZA.... HSR.... MZA.... DEC..M. GZU.... SVI.... HTO.... MVI.... DC?T.M. "\
    "GST.... SZA.... HSR.... MZA.... DC?T.M. GZU.... SVI.... HTO.... MVI.... DC?T.M. " \
    "GZU.... SVI.... HTO.... MZA.... DC?T.M. GST.... SVI.... HTO.... MZA.... DAMT.V. "\
    "GST.... SZA.... HSR.... MVI.... DS?.... GZU.... SVI.... HTO.... MZA.... DC?T.M. "\
    "GST.... SZA.... HSR.... MVI.... DC?T.M. GST.... SZA.... HTO.... MVI.... DC?T.M. "\
    "GZU.... SZA.... HTO.... MVI.... DC?T.M. GZU.... SVI.... HSR.... MZA.... DPRT.M. "\
    "GST.... SZA.... HTO.... MVI.... DC?.V.. GZU.... SVI.... HSR.... MZA.... DC?T.M. "\
    "GST.... SVI.... HSR.... MZA.... DC?T.M. GZU.... SVI.... HSR.... MZA.... DC?T.M. "\
    "GST.... SVI.... HSR.... MZA.... DC?T.M. GST.... SZA.... HTO.... MVI.... DC?T.M. "\
    "GZU.... SVI.... HSR.... MZA.... DC?T.V. GST.... SZA.... HTO.... MVI.... DC?T.M. "\
    "GST.... SZA.... HTO.... MZA.... DZUT.M. GZUTD.. SVI.... HSR.... MVI.... DSTT.M. "\
    "GZU.... SZA.... HTO.... MZA.... DC?T.M. GZU.... SVI.... HSR.... MVI.... DC?T.M. "\
    "GSTT... SZA.... HTO.... MZA.... DC?T.M. GZU.... SVI.... HSR.... MVI.... DC?.VM. "\
    "GMU.... SZA.... HTO.... MZA.... DC?T... GZU.... SVI.... HSR.... MVI.... DC?T... "\
    "GMU.... SZA.... HTO.... MZA.... DC?T.M. GZU.... SVI.... HSR.... MVI.... DSTT.M. "\
    "GMU.... SVI.... HSR.... MVI.... DC?T.M. GZU.... SVI.... HSR.... MVI.... DC?T.V. "\
    "GSTT... SZA.... HTO.... MZA.... DC?T.M. GST.... SVI.... HSR.... MVI.... DC?T.M. "\
    "GZU.... SZA.... HTO.... MZA.... DC?T.M. GMU.... SVI.... HSR.... MVI.... DC?T... "\
    "GZU.... SZA.... HTO.... MZA.... DSTT.M. GMU.... SVI.... HSR.... MVI.... DC?T.M. "\
    "GZU.... SVI.... HSR.... MVI.... DC?.VM. GSTT... SZA.... HTO.... MZA.... DC?T.M. "\
    "GST.... SVI.... HSR.... MVI.... DC?T.M. GZU.... SZA.... HTO.... MZA.... DC?T.M. "\
    "GMU.... SVI.... HSR.... MVI.... DCOT... GZU.... SZA.... HTO.... MZA.... DC?T.M. "\
    "GMU.... SVI.... HSR.... MVI.... DC?T.V. GZU.... SVI.... HSR.... MVI.... DS?..M. "\
    "GST.... SVI.... HSR.... MVI.... DC?T.M. GZU.... SVI.... HSR.... MVI.... DC?T.M. "\
    "GZU.... SZA.... HTO.... MZA.... DLVT.M. GST.... SVI.... HSR.... MVI.... DC?T.M. "\
    "GZU.... SZA.... HTO.... MZA.... DC?T.M. GST.... SZA.... HTO.... MZA.... DC?.V.. "\
    "GZU.... SZA.... HTO.... MZA.... DS?..M. GST.... SZA.... HTO.... MZA.... DC?T.M. "\
    "GST.... SVI.... HSR.... MVI.... DC?T.M. GZU.... SZA.... HTO.... MZA.... DC?T.M. "\
    "GST.... SVI.... HSR.... MVI.... DC?T.M. GST.... SVI.... HTO.... MVI.... DC?T.V. "\
    "GST.... SZA.... HTO.... MVI.... DZUT... GZUTD.. SVI.... HSR.... MZA.... DC?T.M. "\
    "GZU.... SZA.... HTO.... MVI.... DC?T.M. GZU.... SVI.... HSR.... MZA.... DC?T.M. "\
    "GST.... SZA.... HTO.... MVI.... DC?T.M. GPA.... SVI.... HSR.... MZA.... DC?T.M. "\
    "GST.... SZA.... HTO.... MVI.... DC?.V.. GPA.... SVI.... HSR.... MZA.... DS?..M. "\
    "GST.... SZA.... HTO.... MVI.... DC?T.M. GPA.... SZA.... HTO.... MVI.... DS?..M. "\
    "GST.... SZA.... HTO.... MVI.... DC?T.M."
*/
# define xMsgs {\
    "", "", "", "", ""}/*,"", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    \
    "", "", "", "", "","", "", "","", "", "","", "", "","", "", "","", "", "", \
    "", "", "", "", "","", "", "","", "", "","","","","" \
}*/

#endif

int main(void)
{
	char *pastPlays = xPastPlays;
    
    int c = 0;
    for (int i = 0; i < strlen(pastPlays); i++) {
        if (pastPlays[i] == ' ') c++;
    }

	Message msgs[c+1];
    for (int i = 0; i <= c; i++) {
        strcpy(msgs[c], "");
    }

	View state = ViewNew(pastPlays, msgs);
	decideMove(state);
	ViewFree(state);

	printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
	return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay(const char *play, Message message)
{
	strncpy(latestPlay, play, MOVE_SIZE - 1);
	latestPlay[MOVE_SIZE - 1] = '\0';

	strncpy(latestMessage, message, MESSAGE_SIZE - 1);
	latestMessage[MESSAGE_SIZE - 1] = '\0';
}
