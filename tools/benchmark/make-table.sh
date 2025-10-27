#!/bin/bash

if [[ $# -eq 0 ]]; then
	echo 'Usage: make-table [--sort-strict] [--bintree] directory search-term'
	exit
fi

if [[ "$1" == *"-s"* ]]; then
	sort_key=3
	shift
else
	sort_key=5
fi

if [[ "$1" == *"-b"* ]]; then
	pattern='.*/[ab].*data'
	shift
else
	pattern='.*/[^ab].*data'
fi

if [[ $# -lt 2 ]]; then
	header='| Dimensions | gawk_trees Peak Memory Usage | gawk Peak Memory Usage | Memory Decrease | gawk_trees Runtime | gawk Runtime | Slowdown |
|------------|------------------------------|------------------------|-----------------|--------------------|--------------|----------|'
	search='Max|User'
	both=1
elif [[ "$2" == *"m"* ]]; then
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

# replace scheme in file names with C-style array syntax
for line in $points; do
	line=$(sed -E 's/([0-9]+)-/\[\1\]/g' <<< "$line")
	line=$(sed -E 's/([0-9]+)([x^])/\[\1\]\2/' <<< "$line")
	line=$(sed -E 's/\]([0-9]+)$/\]\[\1\]/' <<< "$line")
	line=$(sed -E 's/^[0-9]+$/\[\0\]/' <<< "$line")
	
	# seperate case for long version of translating '^'
	# if grep -q '\^' <<< "$line"; then
	# 	num=$(grep -oE '[0-9]+$' <<< "$line")
	# 	figure=$(sed -E 's/\^[0-9]+//' <<< "$line")
	# 	line=""

	# 	for ((i=0;i<num;i++)); do
	# 		line+="[$figure]"
	# 	done
	# fi

	rows+="| $line 
"
done

echo "$header"
data=$(find "$1" -type f -regex "$pattern" -exec grep -E "$search" {} \; | grep -oE '[0-9].*[0-9]')

i=1
stats=""

if [[ -z "$both" ]]; then
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
else
	while read t_data1; read m_data1; read t_data2; read m_data2
	do
		if [[ $i -ne 1 ]]; then
			stats+=' |
'
		fi

		stats+=$(sed -n ${i}p <<< "$rows" | tr -d '\n')
		stats+="| $m_data1 | $m_data2 | "
		stats+=$(awk '{print ($2 - $1) / $2 * 100}' <<< "$m_data1 $m_data2")
		stats+='%'

		stats+=" | $t_data1 | $t_data2 | "
		stats+=$(awk '{print ($1 - $2) / $2 * 100}' <<< "$t_data1 $t_data2")
		stats+='%'

		i=$(expr $i + 1)
	done <<< $data
fi

stats+=' |'
sort_reverse=""

if [[ $sort_key -ne 3 ]] && [[ "$search" == *"Max"* ]]; then
	sort_reverse=" -r"
fi

stats=$(echo "$stats" | sort -n$sort_reverse -t '|' -k $sort_key)
echo "$stats"

