# Fury of Dracula Game
This is a game client written in C for the COMP2521 assignment 'Fury of Dracula'.

#### Group: Tom + Cindy + Tara + Yash

- Member 1: tdavie, Tom Davie, z5263970, H13A
- Member 2: sauravyash, Saurav Yash Agasti, z5309578, H13A
- Member 3: TaraAndresson, Tara Andresson, z5255646, H13A
- Member 4: cindyjbli, Cindy Li, z5059048, H13A


# Tests
## Default
- Basic initialisation
- After Lord Godalming's turn
- After Mina Harker's turn
- After Dracula's turn
- Encountering Dracula
- Test for Dracula doubling back at sea, and losing blood points (Hunter View)
- Test for Dracula doubling back at sea, and losing blood points (Dracula View)
- Checking that hunters' health points are capped
- Testing a hunter 'dying'
- Testing Dracula doubling back to Castle Dracula
- Testing vampire/trap locations
- Testing trap locations after one is destroyed
- Testing a vampire maturing
- Testing move/location history
- Testing connections
- Checking Galatz road connections (Lord Godalming, Round 1)
- Checking Ionian Sea boat connections (Lord Godalming, Round 1)
- Checking Paris rail connections (Lord Godalming, Round 2)
- Checking Athens rail connections (none)

## Game Tests
- [x] Check the game score decreases by 1 each time Dracula finishes his turn.
- [x] Check the game score decreases by 6 each time a hunter loses all their life points and is teleported to the Hospital of St Joseph and St Mary.
- [x] Check the game score decreases by 13 each time a vampire matures.
- [x] A hunter gains 3 life points each time they rest
- [x] test drcula losing 2 health @ sea

## Hunter Tests
- [x] Test Hunters cannot exceed 9 life points.
- [x] Check that immature vampires can be killed within 6 rounds of being spawned
- [ ] Test Road, Rail and Sea moves
- [ ] Test Rail move follows the sum rules (sum = round number + hunter number):
  - [ ] sum mod 4 is 0: The hunter may not move by rail this turn.
  - [ ] sum mod 4 is 1: The maximum allowed distance via rail is 1.
  - [ ] sum mod 4 is 2: The maximum allowed distance via rail is 2.
  - [ ] sum mod 4 is 3: The maximum allowed distance via rail is 3.
- [ ] Test Location move that isnt adjacent to the current pos.
- [ ] Test collaborative research move (when all hunters rest as their last move, they reveal dracula's last moves up to his last Location move)
- [ ] Check that the hunters loses 2 life points if they encounter a trap.
- [ ] Check that the hunters loses 4 life points if they encounter Dracula.
- [ ] Check Hunter teleportation when reaching health <= 0 and health is regained on next turn
- [ ] Check that HIDE moves are revealed when they become the last move in Dracula's trail.

## Dracula Tests
- [ ] Test Dracula cannot make a HIDE move while he is at sea.
- [ ] Test Location move that isnt adjacent to the current pos.
- [ ] check for encounter when dracula move to a city
- [ ] Check auto teleportation when Dracula has no legal moves (because of previous moves)
- [ ] Test when round number is divisible by 13, an immature vampire is placed, else a trap
- [ ] check for max of 3 encounters at a city
- [ ] Check that there are no encounters at sea
- [ ] Test Double Back moves 1 to 5
- [ ] Test that a second double back move within the first five moves after a double back is illegal
- [ ] Check Dracula starts with 40 blood points 
- [ ] Check Dracula is permitted to exceed 40 blood points.
- [ ] Check Dracula loses 10 blood points each time he encounters a hunter.
- [ ] Check Dracula loses 2 blood points if he is at sea at the end of his turn.
- [ ] Check Dracula regains 10 blood points if he is in Castle Dracula at the end of his turn, even if he has been automagically teleported there.
- [ ] Check that Dracula cannot make a LOCATION move to a location if he has already made a LOCATION move to that same location in the last 5 rounds.
- [ ] Check that Dracula can never move to the Hospital of St Joseph and St Mary. -> MAKE AN ABORT
- [ ] Test Dracula can't make a HIDE move if he has made a HIDE move in the last 5 rounds.

## IDK where to test (plz fix) 
- [ ] Test for end of game
  - [ ] if dracula runs out of life points    -> the HUNT
  - [ ] if Game points reaches 0              -> the HUNT

- [ ] Check that the sequence of Character moves is Lord Godalming, Dr. Seward, Van Helsing, Mina Harker, Dracula

