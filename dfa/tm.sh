#!/bin/bash

PROG_DIR=/home/chuck/automata/dfa
DFA=${PROG_DIR}/dfa
#MACHINE_FILE="${PROG_DIR}/auto_divisibleByThree.txt"
MACHINE_FILE="${PROG_DIR}/auto_Liouville_contains_4th.txt"
#MACHINE_FILE="${PROG_DIR}/auto_acceptAll.txt"

#INPUT_STR_CMD='$(echo "obase=2; $(od -vAn -N8 -tu8 < /dev/urandom)"|bc)'
#INPUT_STR_CMD="'$(cat /dev/urandom | tr -dc '[:digit:]' | fold -w ${1:-30} | head -n 5 | sed 's/[1-9]/1/g' | tr 01 10)'"
#INPUT_STR_CMD='$($PROG/DIR/genbits.sh 30 

INPUT_STR_LENGTH=${1:-1024}
SLEEP_INTERVAL=${2:-0}

if [ "$SLEEP_INTERVAL" -eq 0 ]; then
	while true; do 
		$DFA \
			$MACHINE_FILE \
			-f <(cat /dev/urandom | tr -dc '[:digit:]' | fold -w $((RANDOM%INPUT_STR_LENGTH+1)) | head -n5 | sed 's/[1-9]/1/g' | tr 01 10 )
			#-f <(cat /dev/urandom | tr -dc '[:digit:]' | fold -w $((RANDOM%INPUT_STR_LENGTH+1)) | head -n5 | sed 's/[1-7]/1/g' | tr 01 10 | sed 's/8/00/g' | sed 's/9/000/g')
			#-f <(cat /dev/urandom | tr -dc '[:digit:]' | fold -w $(( RANDOM % INPUT_STR_LENGTH + 1)) | head -n 5 | sed 's/[1-9]/1/g' | tr 01 10)
			#-f <(cat /dev/urandom | tr -dc '[:digit:]' | fold -w ${INPUT_STR_LENGTH} | head -n 5 | sed 's/[1-9]/1/g' | tr 01 10)
			#-f <(echo "obase=2; $(openssl rand 128 | od -DAn | tr '\n' ' ' | sed -r 's/\s+//g')"|bc)
			#-f <(echo "obase=2; $(od -vAn -N8 -tu8 < /dev/urandom)"|bc)
			#-f <($PROG_DIR/genbits.sh $INPUT_STR_LENGTH)
	done
else
	while true; do 
		$DFA \
			$MACHINE_FILE \
			-f <(cat /dev/urandom | tr -dc '[:digit:]' | fold -w ${INPUT_STR_LENGTH} | head -n 5 | sed 's/[1-8]/1/g' | tr 01 10)
			#-f <($PROG_DIR/genbits.sh $INPUT_STR_LENGTH)
			#-f <(echo "obase=2; $(od -vAn -N8 -tu8 < /dev/urandom)"|bc)
		sleep $SLEEP_INTERVAL
	done
fi

#
# generating strings
#
# ### BASH BUILT-IN RANDOM VARIABLE ###
# $(echo "obase=2;$((RANDOM % RANDOM_MODULO))"|bc)
# ### LINUX RANDOM DEV USING OD
# $(echo "obase=2; $(od -vAn -N8 -tu8 < /dev/random)"|bc)
