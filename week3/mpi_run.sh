#!/usr/bin/env bash

echo "test 1, on file seq" >> test.txt
mpirun test2.out >> test.txt

for FILE in test1.out test3.out test4.out
do
    for i in 2 4 8 12 16 24 32 40 48 56 64
    do
        echo "test $i, on file $FILE" >> test.txt
        if [[ $i -gt 4 ]]
        then
            mpirun -c 4 $FILE >> test.txt
        else
            mpirun -c $i $FILE >> test.txt
        fi
    done
done