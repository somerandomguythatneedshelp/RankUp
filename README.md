### RankUp

WiP not going to be on bakkesplugins for a while

I am using Rider to code and build because i can and i like being *unique*

MERRY CHRISTMAS HO HO HO 

### algoritm

Game starts -> Check if Ranked -> if playlist id != 10 = 2v2 -> BeforeMatchMMR (n1) -> Game ends -> AfterMatchMMR (n2) -> Calculate difference (subtract current mmr with new mmr) -> Is new int negative or not (n3) -> if positive player win -> save mmr to a file

Game starts -> Get MMR values from file -> Calculate average (n4) -> add current mmr before the match to the array with MMR rankings and find the next one -> add current MMR with average mmr gain -> see if the next mmr ranking is still the next one -> if not, player will rank up, if not, the player will not rank up

# n1 
this int is initalized before the match starts to compare later

# n2
This int is initalized after the match has ended

# n3
Subtract n2 with n1, This is the mmr gain, if it is positive the player has won the game, if its negative the player lost the game. 

# n4 
To calculate the average, add all the numbers from the list, then divide by how many items you divided by

### WiP Comments

Timezone: GMT

Written on 15/12 00:43

- Changed plan from using an image to showing text (Other plugins will be effected by this)
- Text shows up at the start of a game and does get removed at the end. 

- Will remove unneccesary code for optimization 

Written on 15/12 01:42

- Changed max rank prediction from 9 - 13 
- basic rank prediction, more testing later today

- might add option to customize offset

Written on 15/12 16:26

- Fixed a bug where current mmr and the next mmr int would be the same

- Will add another feature to get the average MMR gain and save that to a file, Will use that to determine if the user will rank up.

Written on 16/12 21:32

- Written base for CalculateMMRGain()
- Adds non negatives numbers (numbers that are not 0 or below) to a file to find the average and use that for the max mmr prediction (add all values and divide by how many values there are to find the average)

- Will test tommorow

