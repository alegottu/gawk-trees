#!/bin/bash

if [[ "$1" == *"-b"* ]]; then
	shift
	pattern='.*/[ab].*time'
else
	pattern='.*/[en].*time'
fi;
files=$(find "$1" -type f -regex "$pattern")

points=$(echo "$files" | cut -d '/' -f 2 | tail -n +2 | uniq)
rows=""

for line in $points; do
	rows+="| $line 
"
done

types=$(echo "$files" | head -n 2 | grep -oE '[a-zA-Z]+.time' | sed 's/.time//')

echo -n '| dimensions '
for line in $types; do
	echo -n "| $line "
done
echo -n '|
|'
for (( i=-1; i<11; i++ )); do
	echo -n '-'
done
for line in $types; do
	echo -n '|'
	for (( i=-2; i<${#line}; i++ )); do
		echo -n '-'
	done
done
echo '|'

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
