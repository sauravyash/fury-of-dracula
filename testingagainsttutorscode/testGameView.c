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



printf("testing a round \n");

char *trail =
	"GPA.... SBR.... HMA.... MCD.... DDU.V.. GST.... SPR.... HSR.... MCD.... DGWT... GZU.... SVI.... HTO.... MCD.... DAO.... GST.... SZA.... HSR.... MCD.... DIR.... GZU.... SVI.... HTO.... MCD.... DSWT... GST.... SZA.... HSR.... MCD.... DLVT... GZU.... SVI.... HTO.... MCD.... DMNT.V. GST.... SZA.... HSR.... MCD.... DLOT.M. GZU.... SVI.... HTO.... MCD.... DPLT... GST.... SZA.... HSR.... MCD.... DEC.... GZU.... SVI.... HTO.... MCD.... DAO..M. GST.... SZA.... HSR.... MCD.... DGWT.M. GZU.... SVI.... HTO.... MCD.... DDUT.M. GST.... SZA.... HSR.... MCD.... DIR..M. GZU.... SVI.... HTO.... MCD.... DLVT.M. GST.... SZA.... HSR.... MCD.... DSWT... GZU.... SVI.... HTO.... MCD.... DLOT... GST.... SZA.... HSR.... MCD.... DMNT.M. GZU.... SVI.... HTO.... MCD.... DEDT.M. GST.... SZA.... HSR.... MCD.... DNS.... GZU.... SVI.... HTO.... MCD.... DEC..M. GST.... SZA.... HSR.... MCD.... DPLT.M. GZU.... SVI.... HTO.... MCD.... DLOT.M. GST.... SZA.... HSR.... MCD.... DSWT.M. GZU.... SVI.... HTO.... MCD.... DLVT.M. GST.... SZA.... HSR.... MCD.... DMNT... GZU.... SVI.... HTO.... MCD.... DED.V.. GST.... SZA.... HSR.... MCD.... DNS..M. GZU.... SVI.... HTO.... MCD.... DEC..M. GST.... SZA.... HSR.... MCD.... DLOT.M. GZU.... SVI.... HTO.... MCD.... DSWT.M. GST.... SZA.... HSR.... MCD.... DLVT.M. GZU.... SVI.... HTO.... MCD.... DMNT.V. GST.... SZA.... HSR.... MCD.... DEDT... GZU.... SVI.... HTO.... MCD.... DNS.... GST.... SZA.... HSR.... MCD.... DAO..M. GZU.... SVI.... HTO.... MCD.... DGWT.M. GST.... SZA.... HSR.... MCD.... DDUT.M. GZU.... SVI.... HTO.... MCD.... DIR..M. GST.... SZA.... HSR.... MCD.... DSW.VM. GZU.... SVI.... HTO.... MCD....";

Message messages[42*5] = {};
GameView gv = GvNew(trail, messages);


GvFree(gv);
printf("Test passed!\n");

return EXIT_SUCCESS;
}
