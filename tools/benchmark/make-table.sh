#!/bin/bash

if [[ $# -eq 0 ]]; then
	echo 'Usage: make-table [--bintree] directory search-term'
	exit
fi

if [[ "$1" == *"-b"* ]]; then
	pattern='.*/[ab].*data'
	shift
else
	pattern='.*/[^ab].*data'
fi

if [[ "$2" == *"m"* ]]; then
	header='| Dimensions | gawk_trees Peak Memory Usage | gawk Peak Memory Usage | Memory Decrease |
|------------|------------------------------|------------------------|-----------------|'
	search='Max'
elif [[ "$2" == *"t"* ]]; then
	header='| Dimensions | gawk_trees Runtime | gawk Runtime | Slowdown |
|------------|--------------------|--------------|----------|'
	search='User'
fi

# if --bintree
if ! echo "$pattern" | grep '^' > /dev/null; then
	header=$(echo "$header" | sed 's/gawk_trees/AVL/')
	header=$(echo "$header" | sed 's/gawk/Bintree/')
fi

files=$(find "$1" -type f -regex "$pattern" -printf '%f\n')
points=$(echo "$files" | sed 's/.data//')
rows=""

for line in $points; do
	rows+="| $line 
"
done

echo "$header"

data=$(find "$1" -type f -regex "$pattern" -exec grep "$search" {} \; | grep -oE '[0-9].*[0-9]')
ends=""

for line in $data; do
	ends+=$(echo "| $line | ")
done

stats=""
i=1

while read data1; read data2
do
	if [[ $i -ne 1 ]]; then
		stats+=' |
'
	fi

	stats+=$(sed -n ${i}p <<< "$rows" | tr -d '\n')
	stats+="| $data1 | $data2 | "

	if [[ "$search" == "Max" ]]; then
		stats+=$(awk '{print ($2 - $1) / $2 * 100}' <<< "$data1 $data2")
	else
		stats+=$(awk '{print ($1 - $2) / $2 * 100}' <<< "$data1 $data2")
	fi

	i=$(expr $i + 1)
	stats+='%'
done <<< $data

stats+=' |'
sort_reverse=""
if [[ "$search" == "Max" ]]; then
	sort_reverse=" -r"
fi
stats=$(echo "$stats" | sort -n$sort_reverse -t '|' -k 5)

while read stat
do
	echo $stat
done <<< $stats
