// DLList.c - Implementation of doubly-linked list ADT
// Written by John Shepherd, March 2013
// Modified by John Shepherd, August 2014, August 2015
// Modified by Cindy Li, July 2020

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "DLList.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"

// data structures representing DLList

typedef struct DLListNode {
	PlaceId value;        /**< value of this list item (string) */
	struct DLListNode *prev;
	                    /**< pointer to previous node in list */
	struct DLListNode *next;
	                    /**< pointer to next node in list */
} DLListNode;

typedef struct DLListRep {
	size_t nitems;      /**< count of items in list */
	DLListNode *first;  /**< first node in list */
	DLListNode *curr;   /**< current node in list */
	DLListNode *last;   /**< last node in list */
} DLListRep;

static DLListNode *newDLListNode(char *it);
static void freeDLListNode(DLListNode *node);
		
/** Create a new, empty DLList. */
DLList newDLList (void)
{
	DLListRep *new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate DLList");
	new->curr = new->first = new->last = NULL;
	new->nitems = 0;
	return new;
}

/** Release all resources associated with a DLList. */
void freeDLList (DLList L)
{
	if (L == NULL) return;

	DLListNode *curr = L->first;
	while (curr != NULL) {
		DLListNode *next = curr->next;
		freeDLListNode (curr);
		curr = next;
	}
	free (L);
}

/** Create a new DLListNode (private function) */
static DLListNode *newDLListNode (char *it)
{
	DLListNode *new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate DLList node");
    PlaceId * newLocation = malloc (sizeof *newLocation);
    if (newLocation == NULL) err (EX_OSERR, "couldn't allocate DLList node");
	new->value = newLocation;
	new->prev = new->next = NULL;
	return new;
}

/** Release a DLListNode (private function) */
static void freeDLListNode (DLListNode *node)
{
	if (node == NULL) return;
	free (node->value);
	free (node);
}


/** Return the item at current position. */
char *DLListCurrent (DLList L)
{
	assert (L != NULL);
	return L->curr != NULL ? L->curr->value : NULL;
}


/** return number of elements in a list */
size_t DLListLength (DLList L)
{
	return L->nitems;
}

/** is the list empty? */
bool DLListIsEmpty (DLList L)
{
	return L->nitems == 0;
}
