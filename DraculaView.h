////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.h: the DraculaView ADT
//
// You may augment this with as many of your own functions as you like,
// but do NOT remove or modify any of the existing function signatures,
// otherwise, your code will not be able to compile with our tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-11-30	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#ifndef FOD__DRACULA_VIEW_H_
#define FOD__DRACULA_VIEW_H_

#include <stdbool.h>

#include "Game.h"
#include "Places.h"

typedef struct draculaView *DraculaView;

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

/**
 * Creates a new view to summarise the current state of the game.
 *
 * @param pastPlays - A string of all the plays made in the game so far.
 *                    See the spec for details of the format. The string
 *                    can be assumed to be valid.
 *
 * @param messages - An  array containing a message for each play in the
 *                   game so far. It will have exactly the  same  number
 *                   of  elements as there are plays in `pastPlays`. The
 *                   message from the first play will be at index 0, and
 *                   so on. The contents of each message will be exactly
 *                   as provided by the player.
 *
 * The "Message" type is defined in Game.h.
 * You are free to ignore messages if you wish.
 */
DraculaView DvNew(char *pastPlays, Message messages[]);

/**
 * Frees all memory allocated for `dv`.
 * After this has been called, `dv` should not be accessed.
 */
void DvFree(DraculaView dv);

////////////////////////////////////////////////////////////////////////
// Game State Information

/**
 * Get the current round number
 */
Round DvGetRound(DraculaView dv);

/**
 * Gets the current game score; a positive integer between 0 and 366.
 */
int DvGetScore(DraculaView dv);

/**
 * Gets the current health points for the given player; an value between
 * 0 and 9 for a hunter, or >= 0 for Dracula.
 */
int DvGetHealth(DraculaView dv, Player player);

/**
 * Gets the current location of a given player.
 *
 * This function should return:
 * - NOWHERE if the given player has not had a turn yet.
 * - Otherwise,  a  PlaceId corresponding to a real location, as Dracula
 *   has full knowledge of all moves.
 */
PlaceId DvGetPlayerLocation(DraculaView dv, Player player);

/**
 * Gets the location of the sleeping immature vampire.
 *
 * This function should return:
 * - NOWHERE  if the immature vampire does not exist (i.e., if it hasn't
 *   been spawned, or if it has already matured or been vanquished).
 * - Otherwise,  a  PlaceId corresponding to a real location, as Dracula
 *   has full knowledge of all moves.
 */
PlaceId DvGetVampireLocation(DraculaView dv);

/**
 * Gets the locations of all active traps.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numTraps to the number of  traps.  The  array  should
 * contain  multiple  copies  of the same location if there are multiple
 * traps in that location. The locations can be in any order.
 */
PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps);

////////////////////////////////////////////////////////////////////////
// Making a Move

/**
 * Gets all the moves that Dracula can validly make this turn.
 *
 * This  function  should  return  the  moves in a dynamically allocated
 * array, and set *numReturnedMoves to the number of moves returned. The
 * array can be in any order but must contain unique entries.
 *
 * If  Dracula  has  no valid moves (other than TELEPORT), this function
 * should set *numReturnedMoves to 0 and return  an  empty  array  (NULL
 * will suffice).
 *
 * If  Dracula  hasn't  made  a move yet, set *numReturnedMoves to 0 and
 * return NULL.
 */
PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves);

/**
 * Gets all the locations that Dracula can move to this turn.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numReturnedLocs to the number of locations  returned.
 * The array can be in any order but must contain unique entries.
 *
 * The  returned  locations should be consistent with Dracula's movement
 * rules (see the spec for details).  If  Dracula  has  no  valid  moves
 * (other than TELEPORT), this function should set *numReturnedLocs to 0
 * and return an empty array (NULL will suffice).
 *
 * If  Dracula  hasn't  made  a move yet, set *numReturnedMoves to 0 and
 * return NULL.
 */
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs);

/**
 * Similar  to  DvWhereCanIGo, but the caller can restrict the transport
 * types to be considered. For example, if road is  true,  but  boat  is
 * false, boat connections will be ignored.
 */
PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs);

/**
 * Gets  all  the  locations  that the given player can move to in their
 * next move (for Dracula that is this turn).
 *
 * This  function  should return the locations in a dynamically alloated
 * array, and set *numReturnedLocs to the number of locations  returned.
 * The  array can be in any order but must contain unique entries.
 *
 * If  the given player is a hunter, the function must take into account
 * how far the player can travel by rail in their next move.
 *
 * If  the  given  player  is  Dracula, the returned locations should be
 * consistent with the rules on Dracula's movement  (see  the  spec  for
 * details).
 *
 * If the given player hasn't made a move yet, set *numReturnedLocs to 0
 * and return NULL.
 */
PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs);

/**
 * Similar to DvWhereCanTheyGo but the caller can restrict the transport
 * types  to  be considered. For example, if road and rail are true, but
 * boat is false, boat connections will be ignored.
 */
PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Game History

/**
 * Gets  the complete move history, in chronological order, of the given
 * player as shown in the play string.
 *
 * This  function  should  return  the  moves in a dynamically allocated
 * array, and set *numReturnedMoves to the number of moves returned.
 *
 * This  function can decide whether the caller is allowed to  modify or
 * free the returned array. (If the array is part of the  DraculaView  data
 * structure,  you may not want the caller to modify or free it.) If the
 * returned array can be modified/freed, set *canFree to true  to  avoid
 * memory leaks. Otherwise, set it to false.
 */
PlaceId *DvGetMoveHistory(DraculaView dv, Player player,
                          int *numReturnedMoves, bool *canFree);

/**
 * Gets  the given player's last `numMoves` moves in chronological order
 * as shown in the play string.
 *
 * This  function  should  return  the  moves in a dynamically allocated
 * array, and set *numReturnedMoves to the number of moves returned  (in
 * case  `numMoves`  is  greater than the number of moves the player has
 * actually made).
 *
 * This  function can decide whether the caller is allowed to  modify or
 * free the returned array. (If the array is part of the  DraculaView  data
 * structure,  you may not want the caller to modify or free it.) If the
 * returned array can be modified/freed, set *canFree to true  to  avoid
 * memory leaks. Otherwise, set it to false.
 *
 * NOTE: This  function is very similar to DvGetMoveHistory, except that
 *       it gets only the last `numMoves` moves rather than the complete
 *       move history.
 */
PlaceId *DvGetLastMoves(DraculaView dv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree);

/**
 * Gets  the  complete  location history, in chronological order, of the
 * given player.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numReturnedLocs to the number of locations returned.
 *
 * If  the given player is a hunter, this function should behave exactly
 * the same as DvGetMoveHistory. If the given  player  is  Dracula,  the
 * PlaceIds  returned  should  either  correspond  to real places, or be
 * CITY_UNKNOWN or SEA_UNKNOWN. See the comment for  DvGetPlayerLocation
 * for more details.
 *
 * This  function can decide whether the caller is allowed to  modify or
 * free the returned array. (If the array is part of the  DraculaView  data
 * structure,  you may not want the caller to modify or free it.) If the
 * returned array can be modified/freed, set *canFree to true  to  avoid
 * memory leaks. Otherwise, set it to false.
 */
PlaceId *DvGetLocationHistory(DraculaView dv, Player player,
                              int *numReturnedLocs, bool *canFree);

/**
 * Gets  the  given   player's last `numLocs` locations in chronological
 * order.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numReturnedLocs to the number of  locations  returned
 * (in  case  `numLocs` is greater than the number of  moves  the player
 * has actually made).
 *
 * This  function can decide whether the caller is allowed to  modify or
 * free the returned array. (If the array is part of the  DraculaView  data
 * structure,  you may not want the caller to modify or free it.) If the
 * returned array can be modified/freed, set *canFree to true  to  avoid
 * memory leaks. Otherwise, set it to false.
 *
 * NOTE: This function is very similar to  DvGetLocationHistory,  except
 *       that  it gets only the last `numLocs` locations rather than the
 *       complete location history.
 */
PlaceId *DvGetLastLocations(DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree);

bool canHide(DraculaView dv);

bool canDoubleBack(DraculaView dv);


#endif // !defined(FOD__DRACULA_VIEW_H_)
