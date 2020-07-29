# Fury of Dracula Game
This is a game client written in C for the COMP2521 assignment 'Fury of Dracula'.

#### Group: Tom + Cindy + Tara + Yash

- Member 1: tdavie, Tom Davie, z5263970, H13A
- Member 2: sauravyash, Saurav Yash Agasti, z5309578, H13A
- Member 3: TaraAndresson, Tara Andresson, z5255646, H13A
- Member 4: cindyjbli, Cindy Li, z5059048, H13A


# Tests
## Gameview Tests
### Default
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

### Custom
- [ ] Check the game score decreases by 1 each time Dracula finishes his turn.
- [ ] Check the game score decreases by 6 each time a hunter loses all their life points and is teleported to the Hospital of St Joseph and St Mary.
- [ ] Check the game score decreases by 13 each time a vampire matures.
- [ ] Check that immature vampires can be killed within 6 moves of being spawned
- [ ] Check Dracula starts with 40 blood points 
- [ ] Check Dracula is permitted to exceed 40 blood points.
- [ ] Check Dracula loses 10 blood points each time he encounters a hunter.
- [ ] Check Dracula loses 2 blood points if he is at sea at the end of his turn.
- [ ] Check Dracula regains 10 blood points if he is in Castle Dracula at the end of his turn, even if he has been automagically teleported there.
- [ ] Test for end of game
-- [ ] if dracula runs out of life points
-- [ ] if Game points reaches 0
- [ ] Test Hunters cannot exceed 9 life points.
- [ ] A hunter loses 2 life points if they encounter a trap.
- [ ] A hunter loses 4 life points if they encounter Dracula.
- [ ] A hunter gains 3 life points each time they rest
- [ ] Check that the sequence of Character moves is Lord Godalming, Dr. Seward, Van Helsing, Mina Harker, Dracula
- [ ] Test Road, Rail and Sea moves
- [ ] Test Rail move follows the sum rules (sum = round number + hunter number):
-- [ ] sum mod 4 is 0: The hunter may not move by rail this turn.
-- [ ] sum mod 4 is 1: The maximum allowed distance via rail is 1.
-- [ ] sum mod 4 is 2: The maximum allowed distance via rail is 2.
-- [ ] sum mod 4 is 3: The maximum allowed distance via rail is 3.
- [ ] Check Hunter teleportation when reaching health <= 0 and helth is regained on
  next turn
- [ ] Check auto teleportation when Dracula has no legal moves (because of previous moves)
- [ ] Check that Dracula cannot make a LOCATION move to a location if he has already made a LOCATION move to that same location in the last 5 rounds.
- [ ] Check that Dracula can never move to the Hospital of St Joseph and St Mary.
- [ ] Test Dracula can't make a HIDE move if he has made a HIDE move in the last 5 rounds.
- [ ] Test Dracula cannot make a HIDE move while he is at sea.
- [ ] Test Location move that isnt adjacent to the current pos.
- [ ] Test Double Back moves 1 to 5
- [ ] Test a double back move then 4 moves then another double back move is illegal
- [ ] test drcula losing 2 health @ sea
- [ ] check for encounter when dracula move to a city
-- [ ] Test when round number is divisible by 13, an immature vampire is placed, else a trap
- [ ] check for max of 3 encounters at a city
- [ ] Check that there are no encounters at sea
- [ ] Test collaborative research move (when all hunters rest as their last move, they reveal dracula's last moves up to his last Location move)
- [ ] Check that HIDE moves are revealed when they become the last move in Dracula's trail.
