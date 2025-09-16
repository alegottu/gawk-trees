#!/bin/bash

if [[ $# -eq 0 ]]; then
	echo 'Usage: make-table [--bintree] directory search-term'
	exit
fi

if [[ "$1" == *"-b"* ]]; then
	shift
	pattern='.*/[ab].*data'
	header='| dimensions | avl | binary |
|------------|-----|--------|'
else
	pattern='.*/[^ab].*data'
	header='| dimensions | extension | normal |
|------------|-----------|--------|'
fi;
files=$(find "$1" -type f -regex "$pattern" -printf '%f\n')

points=$(echo "$files" | sed 's/.data//')
rows=""

for line in $points; do
	rows+="| $line 
"
done

echo "$header"

data=$(find -type f -regex "$pattern" -exec grep "$2" {} \; | grep -oE '[0-9].*[0-9]')
ends=""

for line in $data; do
	ends+=$(echo "| $line | ")
done

i=0
for line in $data; do
	n=$(expr $i / 2 + 1)
	if [[ $(expr $i % 2) -eq 0 ]]; then
		sed -n ${n}p <<< "$rows" | tr -d '\n'
		echo -n "| $line "
	else
		echo "| $line |"
	fi
	i=$(expr $i + 1)
done
