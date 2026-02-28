if [[ $# -eq 0 ]]; then
	echo 'usage: ./debug.sh <name of file to convert and test>'
	exit 0
fi

CONVERTED=$(mktemp)
python3 convert.py "$1" > $CONVERTED

while read line
do
	diff <(gawk -lghtrees -f $CONVERTED <<< $line) <(gawk -f "$1" <<< $line)
done

rm $CONVERTED
