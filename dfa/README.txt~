README FOR DFA

This program reads a deterministic finite automaton
from a supplied file and runs the automaton on 
a supplied input string. Automaton only works
over the alphabet {0, 1}.

--------------
   RUNNING:
--------------
The input string can be supplied directly as a
command line argument as shown below:

$./dfa <states file> <input string>
$./dfa oddNumberZeroes.txt 10001010

The input string can also be supplied in a single-
line file containing the input string:

$./dfa <states file> -f <input string file>
$./dfa oddNumberZeroes.txt -f input_string.txt

------------------------
   STATES FILE FORMAT
------------------------
The supplied states file must follow a specific
format. For instance, our file "oddNumberZeroes.txt"
may contain:

[0] NAME q0 START 1 FINAL 0 ZERO-> 1 ONE-> 0
[1] NAME q1 START 0 FINAL 1 ZERO-> 0 ONE-> 1

Each line corresponds to a distinct state:
-------------------------------------------------
[i]    : Array location (index) of this state
NAME   : Provide non-spaced name for state
START* : 1 if this is the start state, else 0
FINAL  : 1 if this is a final state, else 0
ZERO-> : Upon reading '0', go to the given array index
ONE->  : Upon reading '1', go to the given array index
-------------------------------------------------
* One and only one START state must exist.

For example, in our "oddNumberZeroes.txt" file, the
first line tells us that there is a state named "q0",
which is the START state, but is not the FINAL state.
If "q0" reads a '0', it goes to "q1".
If "q0" reads a '1', it goes to "q0" (back to itself).

The second line tells us that there is a state named
"q1", which is a FINAL state. 
If "q1" reads a '0', it goes to "q0".
If "q1" reads a '1', it goes to "q1" (back to itself).

Note that there is only one START bit set to 1. One 
and only one state must have a START bit of 1.

------------------------------
   INPUT STRING FILE FORMAT
------------------------------
An input string file, if used, must contain a
single-line, uninterrupted string of '0's and
'1's. For example, an input file "input_string.txt"
may look like this:

1000000001010101010101010101

Only the first line is read.

--------------------
   READING OUTPUT
--------------------
Each state transition is output to the console:

$ ./dfa auto_oddNumZeroes.txt 10100
q0, START? 1, FINAL? 0, ZERO-> q1, ONE-> q0
q1, START? 0, FINAL? 1, ZERO-> q0, ONE-> q1
INPUT: 10100
1: q0 -> q0
0: q0 -> q1 (F)
1: q1 -> q1 (F)
0: q1 -> q0
0: q0 -> q1 (F)
ACCEPTED.

First, the input states are regurgitated for the 
user's covenience. In the future, a '-v' argument
will specifically invoke this action.

Secondly, the INPUT string is regurgiated.

Thirdly, the transitions made during the machine run
are output line-by-line. The "(F)" indicates that the 
latest transition has landed on a FINAL state. Thus, 
the last line output before a state is ACCEPTED must 
end with "(F)".


DATE: 15-02-09
