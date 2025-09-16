#!/bin/bash

if [[ $# -eq 0 ]]; then
	echo 'Usage: make-table [--bintree] directory search-term'
	exit
fi

if [[ "$1" == *"-b"* ]]; then
	shift
	pattern='.*/[ab].*data'
	header='| dimensions | avl | binary | ratio |
|------------|-----|--------|-------|'
else
	pattern='.*/[^ab].*data'
	header='| dimensions | extension | normal | ratio |
|------------|-----------|--------|-------|'
fi;
files=$(find "$1" -type f -regex "$pattern" -printf '%f\n')

points=$(echo "$files" | sed 's/.data//')
rows=""

for line in $points; do
	rows+="| $line 
"
done

echo "$header"

data=$(find "$1" -type f -regex "$pattern" -exec grep "$2" {} \; | grep -oE '[0-9].*[0-9]')
ends=""

for line in $data; do
	ends+=$(echo "| $line | ")
done

i=1
while read data1; read data2
do
	sed -n ${i}p <<< "$rows" | tr -d '\n'
	echo -n "| $data1 | $data2 | "
	echo $(awk '{print $1 / $2}' <<< "$data1 $data2") \|
	i=$(expr $i + 1)
done <<< $data

