#!/bin/bash

export AWKLIBPATH=$PWD
valgrind --tool=massif --stacks=yes --massif-out-file='test.massif' gawk -f test.awk > /dev/null
valgrind --tool=massif --stacks=yes --massif-out-file='test-no-ext.massif' gawk -f test-no-ext.awk > /dev/null
echo "peak mem usage for extension"
ms_print test.massif | grep -E -A 1 "^\s+.B\s*$"
echo "peak mem usage with no extension"
ms_print test-no-ext.massif | grep -E -A 1 "^\s+.B\s*$"
rm *.massif
