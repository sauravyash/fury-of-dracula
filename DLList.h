// DLList.h - Interface to doubly-linked list ADT
// Written by John Shepherd, March 2013
// Last modified, August 2014

#ifndef DLLIST_H
#define DLLIST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// External view of DLList
// Implementation given in DLList.c
// Implements a DLList of strings (i.e. items are strings)

typedef struct DLListRep *DLList;

/** Create a new, empty DLList. */
DLList newDLList (void);

/** Release all resources associated with a DLList. */
void freeDLList (DLList);

/** Add new node to DLList. */
DLListNode *newDLListNode (DLList, char *it);

/** Return the item at current position. */
char *DLListCurrent (DLList);

/** delete current item
 * new item becomes item following current
 * if current was last, current becomes new last
 * if current was only item, current becomes null */
void DLListDelete (DLList);

/** return number of elements in a list */
size_t DLListLength (DLList);

/** is the list empty? */
bool DLListIsEmpty (DLList);

#endif
