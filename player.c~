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
# include "DraculaTest.c"
#else
# include "hunter.h"
# include "HunterView.h"
# include "HunterTest.c"
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

# define xPastPlays "GZA.... SED.... HZU.... MZU...."
# define xMsgs { "", "", "", "" }

#else

typedef HunterView View;

# define ViewNew HvNew
# define decideMove decideHunterMove
# define ViewFree HvFree

# define xPastPlays "GPA.... SBR.... HMA.... MKL.... DC?.V.."
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
	Message msgs[] = xMsgs;

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
