#!/bin/bash

if [[ $# -eq 0 ]]; then
	echo 'Usage: run-tests [-i] [tests if no file] [file]
	Reads a series of tests described by a file or given via
	stdin in order to run each of those using the mem_benchmark.py
	script; each argument or line will be supplied as the arguments
	for one run of mem_benchmark.py, save for -i as the first argument
	which will automatically apply -i to all tests'
	exit
fi

args=""
if [[ "$1" == *"-i"* ]]; then
	args=" -i"
	shift
fi

if [[ -f "$1" ]]; then
	while read line; do
		echo "python3 mem_benchmark.py$args $line"
		python3 mem_benchmark.py$args $line
	done < "$1"
else
	for test in "$@"; do
		echo "python3 mem_benchmark.py$args $test"
		python3 mem_benchmark.py$args $test
	done
fi
