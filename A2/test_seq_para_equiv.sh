#!/bin/bash
#Run scipt to test output of parallel implementation against sequential impl

./main.out -b1 -m1 -f4 -o b1-baseline.pgm
./main.out -b1 -n10 -m2 -f4 -o b1-m2-f4.pgm
./main.out -b1 -n10 -m3 -f4 -o b1-m3-f4.pgm
./main.out -b1 -n10 -m4 -f4 -o b1-m4-f4.pgm
for i in 'b1-m2-f4.pgm' 'b1-m3-f4.pgm' 'b1-m4-f4.pgm' 
do
    result=$(diff b1-baseline.pgm $i)
    if [ $? -eq 0 ]
    then
        echo $i" Parallelization passes test on B1, No filter"
    else
        echo $i" Parallelization FAILS test on B1, No filter"
    fi
done



