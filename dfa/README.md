
DFA
===========
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

BUILDING
--------------
```
cd automata/dfa
make dfa
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
./dfa machineFile.txt 10001
```
Other parameters can be mixed throughout:
```
./dfa -q machineFile.txt -a600 -r100 10001 -o /tmp/acc.txt
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
$./dfa <machine file> <input string>
$./dfa oddNumberZeroes.txt 10001010
```
The input string can also be supplied in a file containing
one input string per line.
```
$./dfa <machine file> -f <input string file>
$./dfa oddNumberZeroes.txt -f input_test_string.txt
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
format. For instance, the file "auto_oddNumberZeroes.txt"
contains:
```
NAME q0 START 1 FINAL 0 ZERO q1 ONE q0
NAME q1 START 0 FINAL 1 ZERO q0 ONE q1
```
Each line corresponds to a distinct state which
contains the following information:
```
NAME   : Provide non-spaced name for state
START* : 1 if this is the start state, else 0
FINAL  : 1 if this is a final state, else 0
ZERO   : Upon reading '0', go to state name
ONE    : Upon reading '1', go to state name
*One and only one START state must exist.
```

For example, in the "oddNumberZeroes.txt" file, the
first line tells us that there is a state named "q0",
which is the START state, but is not the FINAL state:

If "q0" reads a '0', it goes to "q1". <br />
If "q0" reads a '1', it goes to "q0" (back to itself).

The second line tells us that there is a state named
"q1", which is a FINAL state:

If "q1" reads a '0', it goes to "q0". <br />
If "q1" reads a '1', it goes to "q1" (back to itself).

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
./dfa auto_Liouville_contains_3rd.txt -f \
     input_test_string_multi.txt
```
```
./dfa auto_Liouville_contains_4th.txt -f \
     <(cat /dev/urandom | tr -dc '[:alnum:]' | \
     fold -w 4096 | \
     sed 's/[a-zA-X1-9]/1/g' | \
     tr 01 10)
```
```
./dfa auto_Liouville_contains_3rd.txt -f <(cat <<EOF
110001
1010101
1000101
EOF
)
```
Note that at least on Linux, you may provide files
that produce an infinite number of lines, like the
random device /dev/urandom. Keep in mind you will 
need to halt the program manually with CTRL+C or
by killing the ./dfa process.

READING OUTPUT
--------------------
Here is a sample run with typical program output. 
This information is displayed unless the '-q'
option is invoked:
```
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
```

First, the machine file itself is printed.

Second, the transitions made during the machine run
are output line-by-line. The "(F)" indicates that the 
latest transition has landed on a FINAL state. Thus, 
the last line output before a state is ACCEPTED should 
end with "(F)".

Optional delay parameters can be used to pause the
output on an accepted state and/or a rejected state.
For example:

```
./dfa auto_divisibleByThree.txt 110101 -a500 -r40
```

These options will delay the output for 500 
milliseconds on an accepted string or delay 40 
milliseconds on a rejected string. Both options allow 
for delays when reading multi-line input string files.

