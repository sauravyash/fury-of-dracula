########################################################################
# COMP2521 20T2 // the Fury of Dracula // the Hunt
# hunt/Makefile: build your Dracula and Hunter AIs
#
# 2018-12-31	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
# 2020-07-10	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>

CC = gcc
CFLAGS = -Wall -Werror -g

# do not change the following line
BINS = dracula hunter testDraculaView

# add any other *.o files that your solution requires
# (and add their dependencies below after DraculaView.o)
# if you're not using Map.o or Places.o, you can remove them
OBJS = GameView.o Map.o Places.o Queue.o utils.o

# add whatever system libraries you need here (e.g. -lm)
LIBS =

all: $(BINS)

dracula: playerDracula.o dracula.o DraculaView.o $(OBJS) $(LIBS)
hunter: playerHunter.o hunter.o HunterView.o $(OBJS) $(LIBS)


testDraculaView: testDraculaView.o testUtils.o DraculaView.o GameView.o Map.o Places.o Queue.o utils.o
testDraculaView.o: testDraculaView.c DraculaView.h GameView.h Map.h Places.h Game.h Queue.h utils.h
playerDracula.o: player.c dracula.h Game.h DraculaView.h GameView.h Places.h Queue.h utils.h
	$(CC) $(CFLAGS) -DI_AM_DRACULA -c $< -o $@
playerHunter.o: player.c hunter.h Game.h HunterView.h GameView.h Places.h Queue.h utils.h
	$(CC) $(CFLAGS) -c $< -o $@

dracula.o: dracula.c dracula.h DraculaView.h GameView.h Places.h Game.h Queue.h utils.h
hunter.o: hunter.c hunter.h HunterView.h GameView.h Places.h Game.h Queue.h utils.h
GameView.o:	GameView.c GameView.h Places.h Game.h Queue.h utils.h
DraculaView.o: DraculaView.c DraculaView.h GameView.h Places.h Game.h Queue.h utils.h
HunterView.o: HunterView.c GameView.h Places.h HunterView.h Game.h Queue.h utils.h
Map.o: Map.c Map.h Places.h
Places.o: Places.c Places.h

# if you use other ADTs, add dependencies for them here
Queue.o: Queue.c Queue.h
utils.o: utils.c Places.h utils.h

.PHONY: clean
clean:
	-rm -f $(BINS) *.o core
