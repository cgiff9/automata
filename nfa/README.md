
NFA
===========
This program reads a nondeterministic finite automaton
from a supplied "machine file" and runs the automaton 
on a supplied input string. Automaton only works
over the alphabet {0, 1, e}, where 'e' is the empty string.

Every character other than zero is treated as a 1. 
Because the input string is treated as an array of 
characters, an input string containing the 
sequence "\0" will be treated the same as the 
sequence "10". Please be aware of your shell's
quirks when supplying input strings that contain
certain special characters other than 0 and 1.

BUILDING
--------------
```
cd automata/nfa
make nfa
```

PARAMETERS
----------
```
-q           quiet; turn off printout and delays
                *program returns 1 on accept, 0 on reject
-s           convert input to ascii binary representation
-a <num>     delay printout on string accept by 'num' ms
-r <num>     delay printout on string reject by 'num' ms
-f <file>    input string file
                *may contain multiple strings, one per line
                *option overridden by command-line input string
-o <file>    output accept file
                *file is created if it doesn't exist
```

The machine file and input string are supplied as such:
```
./nfa machineFile.txt 10001
```
Other parameters can be mixed throughout:
```
./nfa -q machineFile.txt -a600 -r100 10001 -o /tmp/acc.txt
```
Just know that the first "non-option" parameter is always
the machine file and the 2nd "non-option" parameter is 
always the input string. The input string supplied directly
via the command line always overrides the '-f' option. 

RUNNING THE MACHINE
-------------------------
The input string can be supplied directly as a
command line argument as shown below:
```
$./nfa <machine file> <input string>
$./nfa auto_endsTwoOnes.txt 10001011
```
The input string can also be supplied in a file containing
one input string per line.
```
$./nfa <machine file> -f <input string file>
$./nfa auto_endsTwoOnes.txt -f input_test_string.txt
```
Not that if an input string is supplied via the command-line 
while also invoking the '-f' option, then the '-f'option 
will be ignored.

MACHINE FILE FORMAT
-------------------------
The machine file contains a list of states with one
state per line. It may contain commented lines starting
with the hash '#' character, but lines containing states
may not contain any comments. Blank lines are ignored.

The supplied machine file must follow a specific
format. For instance, the file "auto_endsTwoOnes.txt"
contains:
```
NAME q0 START 1 FINAL 0 ZERO q0 ONE q0,q1 EMPTY x
NAME q1 START 0 FINAL 0 ZERO x ONE q2 EMPTY x
NAME q2 START 0 FINAL 1 ZERO x ONE x EMPTY x 
```
Each line corresponds to a distinct state which
contains the following information:
```
NAME   : Provide non-spaced name for state
START* : 1 if this is the start state, else 0
FINAL  : 1 if this is a final state, else 0
ZERO   : Upon reading '0', go to state names
ONE    : Upon reading '1', go to state names
EMPTY  : Upon reading the empty string, go to state names
*One and only one START state must exist.
```

For example, in the "auto_endsTwoOnes.txt" file, the
first line tells us that there is a state named "q0",
which is the START state, but is not the FINAL state:

If "q0" reads a '0', it goes to "q0". <br />
If "q0" reads a '1', it goes to either "q0" or "q1".

The second line tells us that there is a state named
"q1":

If "q1" reads a '0', it does nothing. <br />
If "q1" reads a '1', it goes to "q2".

The third state contains no transitions. An 'x' or an
'X' must be used to indicate that no transition
exists for the particular character. The EMPTY string
indicates a transition when no character is read.

Multiple state names can be indicated by separating
them with commas with no spaces between.

Take care not to use the same name for two
states, or the machine will complain about
the duplicates.

INPUT STRING FILE FORMAT
------------------------------
If an input string file is used, then it may contain 
an uninterrupted string of '0's and '1's. For example, 
the input file  called  "input_test_string.txt" looks 
like this:

```
111010000101010100010101111100000001010111
```

This file may contain multiple strings, with
one per line. Linefeed and newline characters
will not be included in the string (keep in mind
this does not refer to the separate characters
'\' and '\n' together, but to simply pressing
the 'ENTER/RETURN' button. 

Characters other than linefeeds and newlines will
be interpreted as '1's, including spaces and tabs.

Other examples that are tested to work on Linux:

```
./nfa auto_endsTwoOnes.txt -f \
     input_test_string_multi.txt
```
```
./nfa auto_endsTwoOnes.txt -f \
     <(cat /dev/urandom | tr -dc '[:alnum:]' | \
     fold -w 4096 | \
     sed 's/[a-zA-X1-9]/1/g' | \
     tr 01 10)
```
```
./nfa auto_endsTwoOnes.txt -f <(cat <<EOF
110001
1010101
10001011
EOF
)
```
Note that at least on Linux, you may provide files
that produce an infinite number of lines, like the
random device /dev/urandom. Keep in mind you will 
need to halt the program manually with CTRL+C or
by killing the ./nfa process.

READING OUTPUT
--------------------
Here is a sample run with typical program output. 
This information is displayed unless the '-q'
option is invoked:
```
$ ./nfa auto_endsTwoOnes.txt 10011
q0, START? 1, FINAL? 0, ZERO-> q0, ONE-> q0,q1, EMPTY->
q1, START? 0, FINAL? 0, ZERO->  ONE-> q2, EMPTY->
q2, START? 0, FINAL? 1, ZERO->  ONE->  EMPTY->
[1]0011: q0 -> q0
[0]011: q0 -> q0
[0]11: q0 -> q0
[1]1: q0 -> q0
[1]: q0 -> q0
[1]: q0 -> q1
[1]1: q0 -> q1
[1]: q1 -> q2 (F)
[1]0011: q0 -> q1
(ACCEPTED):
        ==>10011
```

First, the machine file itself is printed.

Second, the transitions made during the machine run
are output line-by-line. The "(F)" indicates that the 
latest transition has landed on a FINAL state. The output
reflects the current character being read in brackets
followed by the remaining characters to be read. So long
as the final character being read leads to a FINAL state, 
the string will be accepted.

Optional delay parameters can be used to pause the
output on an accepted state and/or a rejected state.
For example:

```
./nfa auto_endsTwoOnes.txt 110101 -a500 -r40
```

These options will delay the output for 500 
milliseconds on an accepted string or delay 40 
milliseconds on a rejected string. Both options allow 
for delays when reading multi-line input string files.

