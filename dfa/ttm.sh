#!/bin/bash

PROG_DIR=~/automata/dfa
TM=${PROG_DIR}/tm.sh
NUM_MACHINES=${1:-$(nproc)}

function tm_sigint() {
	$TM
}

killall tm.sh 2>/dev/null
for i in $(seq 0 $((NUM_MACHINES-2))); do 
	screen -dmS ttm_$i $TM
	#$TM >/dev/null &
done
#($TM) #> /dev/null &
#$TM
#trap tm_sigint SIGINT

# $TM > /dev/null &
screen -dmS ttm_$((NUM_MACHINES-1)) $TM
screen -r ttm_$((NUM_MACHINES-1))
