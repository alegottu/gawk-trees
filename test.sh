#!/bin/bash

#AWKLIBPATH=$PWD gawk -f read-pred.awk < pred.txt
AWKLIBPATH=$PWD gawk -f test.awk <<< "$1"
#AWKLIBPATH=$PWD gawk -f test-rand.awk < rand.txt
