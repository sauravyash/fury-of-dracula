########################################################################
# COMP2521 20T2 // the Fury of Dracula // the Hunt
# hunt/Makefile: build your Dracula and Hunter AIs
#
# 2018-12-31	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
# 2020-07-10	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>

CC = gcc
CFLAGS = -Wall -Werror

# do not change the following line
BINS = hunter testGameView

# add any other *.o files that your solution requires
# (and add their dependencies below after DraculaView.o)
# if you're not using Map.o or Places.o, you can remove them
OBJS = GameView.o Map.o Places.o Queue.o utils.o

# add whatever system libraries you need here (e.g. -lm)
LIBS =

all: $(BINS)


hunter: playerHunter.o hunter.o HunterView.o $(OBJS) $(LIBS)
testGameView: testGameView.o testUtils.o GameView.o Map.o Places.o utils.o
testGameView.o: testGameView.c GameView.h Map.h Places.h Game.h utils.h

playerHunter.o: player.c hunter.h Game.h HunterView.h GameView.h Places.h
	$(CC) $(CFLAGS) -c $< -o $@


hunter.o: hunter.c hunter.h HunterView.h GameView.h Places.h Game.h
GameView.o:	GameView.c GameView.h Places.h Game.h utils.h
HunterView.o: HunterView.c GameView.h Places.h HunterView.h Game.h Queue.h utils.h
Map.o: Map.c Map.h Places.h
Places.o: Places.c Places.h

# if you use other ADTs, add dependencies for them here
Queue.o: Queue.c Queue.h
utils.o: utils.c Places.h

.PHONY: clean
clean:
	-rm -f $(BINS) *.o core
