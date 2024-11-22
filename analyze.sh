#!/bin/bash
#command time --verbose ./test.sh 100000000

export AWKLIBPATH=$PWD
rm *.massif

echo "peak mem usage for processes using extension"
valgrind --tool=massif --pages-as-heap=yes --massif-out-file='%ptest.massif' gawk -f test.awk <<< "$1" > /dev/null
files=$(find . -name '*test.massif')
for file in "$files"
do
	ms_print "$file" | grep -E -A 1 "^\s+.B\s*$"
done

# could maybe break down pattern into outer loop later
echo "peak mem usage with for processes without extension"
valgrind --tool=massif --pages-as-heap=yes --massif-out-file='%ptest-no-ext.massif' gawk -f test-no-ext.awk <<< "$1" > /dev/null
files=$(find . -name '*test-no-ext.massif')
for file in "$files"
do
	ms_print "$file" | grep -E -A 1 "^\s+.B\s*$"
done
