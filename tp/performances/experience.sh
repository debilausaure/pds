#!/bin/sh

INPUT_FILE=$1
ITERATION=10

for i in $(seq 0 18)
do
	BUF_SIZE=$(echo "2^$i" | bc -l)
	TOTAL_AVG=0.00
	USER_AVG=0.00
	SYSTEM_AVG=0.00
	for _ in $(seq 1 $ITERATION)
	do
		TIMES=$(/usr/bin/time -f "%e %U %S" ./cat "$BUF_SIZE" "$INPUT_FILE" 2>&1 > /dev/null)
		TOTAL=$( echo "$TIMES" | cut -d ' ' -f 1)
		USER=$(  echo "$TIMES" | cut -d ' ' -f 2)
		SYSTEM=$(echo "$TIMES" | cut -d ' ' -f 3)
		TOTAL_AVG=$( echo "scale=2; $TOTAL  + $TOTAL_AVG"  | bc -l)
		USER_AVG=$(  echo "scale=2; $USER   + $USER_AVG"   | bc -l)
		SYSTEM_AVG=$(echo "scale=2; $SYSTEM + $SYSTEM_AVG" | bc -l)
	done
	TOTAL_AVG=$( echo "scale=2; $TOTAL_AVG  / $ITERATION" | bc -l)
	USER_AVG=$(  echo "scale=2; $USER_AVG   / $ITERATION" | bc -l)
	SYSTEM_AVG=$(echo "scale=2; $SYSTEM_AVG / $ITERATION" | bc -l)
	echo "$BUF_SIZE $TOTAL_AVG $USER_AVG $SYSTEM_AVG"
done
