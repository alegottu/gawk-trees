#!/bin/bash

# usage: $1 = num tests, $2 = num fields, $3 = length of fields in chars

rm test.txt

for (( i=0; i<2; i+=1 ))
do 
	for (( j=0; j<3; j+=1 ))
	do
		tr -dc 0-9 < /dev/urandom | head -c 1 >> test.txt
		echo -n " " >> test.txt
	done

	echo >> test.txt
done
