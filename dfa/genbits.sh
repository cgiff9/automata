#!/bin/bash

STRING_LENGTH=${1:-30}
MODULUS=${2:-10}

while [ "$STRING_LENGTH" -gt 0 ]; do
	if [ "$((RANDOM % MODULUS))" -eq 0 ]; then
		echo -ne 1
	else
		echo -ne 0
	fi
	STRING_LENGTH=$((STRING_LENGTH-1))
done
echo
