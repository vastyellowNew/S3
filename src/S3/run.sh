#!/bin/bash 

./main '../Benchmarks/Table/FSM2284_32_256.table' '../Benchmarks/Input/test.in' '../Benchmarks/Accept/AC_2284.txt' 32 256 0 1 1 4 4 >> Record_dfa2284.txt

mkdir dfa2284
cp *.txt dfa2284/
rm *.txt


