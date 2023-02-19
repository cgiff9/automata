
----------------------------
----------------------------
   README FOR DFA PROGRAM
----------------------------
----------------------------
This program reads a deterministic finite automaton
from a supplied "machine file" and runs the automaton 
on a supplied input string. Automaton only works
over the alphabet {0, 1}.

Every character other than zero is treated as a 1. 
Because the input string is treated as an array of 
characters, an input string containing the 
sequence "\0" will be treated the same as the 
sequence "10". Please be aware of your shell's
quirks when supplying input strings that contain
certain special characters other than 0 and 1.

--------------
   BUILDING
--------------
cd automata/dfa
make dfa

----------------
   PARAMETERS
----------------
-q           quiet; turn off printout and delays
             *program returns 1 on accept, 0 on reject
-s           convert input to ascii binary representation
-a <num>     delay printout on string accept by 'num' ms
-r <num>     delay printout on string reject by 'num' ms
-f <file>    input string file
             *may contain multiple strings, one per line
             *command-line input string overrides this
-o <file>    output accept file
             *date string accepted also output 
             *file is created if it doesn't exist

The machine file and input string are supplied as such:
./dfa machineFile.txt 10001

Other parameters can be mixed throughout:
./dfa -q machineFile.txt -a600 -r100 10001 -o /tmp/acc.txt

Just know that the first "non-option" parameter is always
the machine file and the 2nd "non-option" paramter is 
always the input string. The input string supplied directly
via the command line always overrides the '-f' option. 

-------------------------
   RUNNING THE MACHINE
-------------------------
The input string can be supplied directly as a
command line argument as shown below:

$./dfa <machine file> <input string>
$./dfa oddNumberZeroes.txt 10001010

The input string can also be supplied in a single-
line file containing the input string:

$./dfa <machine file> -f <input string file>
$./dfa oddNumberZeroes.txt -f input_test_string.txt

Not that if an input string is supplied via the command-
line while also invoking the '-f' option, then the '-f'
option will be ignored.

-------------------------
   MACHINE FILE FORMAT
-------------------------
The machine file contains a list of states with one
state per line. It may contain commented lines starting
with the hash '#' character, but lines containing states
may not contain any comments. Blank lines are ignored.

The supplied machine file must follow a specific
format. For instance, the file "auto_oddNumberZeroes.txt"
contains:

[0] NAME q0 START 1 FINAL 0 ZERO 1 ONE 0
[1] NAME q1 START 0 FINAL 1 ZERO 0 ONE 1

Each line corresponds to a distinct state which
contains the following information:
-------------------------------------------------
[n]    : Machine index of this state
NAME   : Provide non-spaced name for state
START* : 1 if this is the start state, else 0
FINAL  : 1 if this is a final state, else 0
ZERO   : Upon reading '0', go to the machine index n
ONE    : Upon reading '1', go to the machine index n
-------------------------------------------------
*One and only one START state must exist.

For example, in the "oddNumberZeroes.txt" file, the
first line tells us that there is a state named "q0",
which is the START state, but is not the FINAL state:

If "q0" reads a '0', it goes to "q1".
If "q0" reads a '1', it goes to "q0" (back to itself).

The second line tells us that there is a state named
"q1", which is a FINAL state:

If "q1" reads a '0', it goes to "q0".
If "q1" reads a '1', it goes to "q1" (back to itself).

Note that there is only one START bit set to 1. One 
and only one state must have a START bit of 1.

The numbers inside the brackets [num] are indexes of
the state described by the particular line. These
indexes are used by ZERO and ONE to describe the
transition function of the state.

The indexes do not need to be in numerical order. This
may help for particularly complex machines where it may
be logical/visually necessarily to insert a new 
state between two others. For instance, the four 
machine files:

[0] NAME q0 START 1 FINAL 0 ZERO 1 ONE 0
[1] NAME q1 START 0 FINAL 1 ZERO 0 ONE 1

and

[1] NAME q1 START 0 FINAL 1 ZERO 0 ONE 1
[0] NAME q0 START 1 FINAL 0 ZERO 1 ONE 0

and

[1] NAME q0 START 1 FINAL 0 ZERO 1 ONE 0
[0] NAME q1 START 0 FINAL 1 ZERO 0 ONE 1

and 

[100] NAME q0 START 1 FINAL 0 ZERO 1 ONE 0
[1] NAME q1 START 0 FINAL 1 ZERO 0 ONE 1

...are all logically equivalent.

Take care not to use the same index for two
states, or the machine will complain about
the duplicate indexes.

------------------------------
   INPUT STRING FILE FORMAT
------------------------------
If an input string file is used, ie.:
    '-f input_test_string.txt' 
then it may contain an uninterrupted string 
of '0's and '1's. For example, the input file 
called  "input_test_string.txt" looks like this:

111010000101010100010101111100000001010111

This file may contain multiple strings, with
one per line. Linefeed and newline characters
will not be included in the string (keep in mind
this does not refer to the separate characters
'\' and '\n' together, but to simply pressing
the 'ENTER/RETURN' button. 

Characters other than linefeeds and newlines will
be interpreted as '1's, including spaces and tabs.
Be careful with how you structure these files.

Other examples that are tested to work on Linux:

./dfa auto_Liouville_contains_3rd.txt -f \
     input_test_string_multi.txt

./dfa auto_Liouville_contains_5th.txt -f \
     <(cat /dev/urandom | tr -dc '[:alnum:]' | \
     fold -w 4096 | \
     sed 's/[a-zA-X1-9]/1/g' | \
     tr 01 10)

./dfa auto_Liouville_contains_3rd.txt -f <(cat <<EOF
110001
1010101
1000101
EOF
)

Note that at least on Linux, you may provide files
that produce an infinite number of lines, like the
random device /dev/urandom. Keep in mind you will 
need to halt the program manually with CTRL+C or
by killing the ./dfa process. Be careful with the
random device, though, or you may end up looking at
very unfamiliar characters in your terminal (the
'reset' command is helpful if this happens).

--------------------
   READING OUTPUT
--------------------
Here is a sample run with typical program output. 
This information is displayed unless the '-q'
option is invoked:

$ ./dfa auto_oddNumZeroes.txt 10100
q0, START? 1, FINAL? 0, ZERO-> q1, ONE-> q0
q1, START? 0, FINAL? 1, ZERO-> q0, ONE-> q1
1: q0 -> q0
0: q0 -> q1 (F)
1: q1 -> q1 (F)
0: q1 -> q0
0: q0 -> q1 (F)
(ACCEPTED)
     ==>10100

First, the machine file itself is printed.

Second, the transitions made during the machine run
are output line-by-line. The "(F)" indicates that the 
latest transition has landed on a FINAL state. Thus, 
the last line output before a state is ACCEPTED should 
end with "(F)", provided there are no bugs!

Optional delay paramters can be used to pause the
output on an accepted state and/or a rejected state.
For example:

./dfa auto_mysetery.txt 110101 -a500 -r40

These options will delay the output for 500 
milliseconds on an accepted string or delay 40 
milliseconds on a rejected string. Both options allow 
for scripts/batch operations using this program to slow 
down sufficiently to be observed more slowly.

-----------------------
   MACHINE CONSTANTS
-----------------------
If you are getting undefined behavior when running
your machine file, and you're certain of its correctness,
please observe the top of the file "dfa.c" and adjust
the three values defined under the "MACHINE CONSTANTS"
comment. 

There are certain bounds that may need to be manually 
set in the source code. The current restriction (and
its default value) is:

STATE NAME CHARACTER LIMIT: 50

This constant (C Macro definition) governs the maximum
number of characters in a state's name and is called:
	STATE_NAME_MAX

There are other less critical definitions which define
the default sleep value for printed output upon accepted
and rejected strings. These defaults are triggered when 
the argument to the '-a' or '-r' paramter is 0:

	SLEEP_ACCEPT_MSEC: 1000
	SLEEP_REJECT_MSEC: 100

----------------------
   OPTIONAL SCRIPTS
----------------------
There are two scripts:
tm.sh
ttm.sh

They help automate the running of the dfa program
over random input strings. You may find them useful
or cumbersome. These scripts were created in the 
bash shell on linux, and the "ttm.sh" script uses
the "screen" command to create multiple instances
of the "tm.sh" script. But hey, who am I to remind
the intelligent user of the need to read scripts
before running them? ;P

There is one more script which could be used by
"tm.sh" called "genbits.sh" which creates strings
with a potentially limited number of ones and a
potentially massive number of zeroes. I'll let you
figure out which machine files such script may be
useful for. ;)

You are always free to write your own
scripts to automate the creation of input strings
and/or run the machine multiple times. 

Keep in mind that with new functionalities being
introduced to the dfa program, these scripts may
become essentially obsoleted to a degree.

Again, enjoy the program:)


LAST MODIFIED: 23-02-18
CREATION DATE: 15-02-09
