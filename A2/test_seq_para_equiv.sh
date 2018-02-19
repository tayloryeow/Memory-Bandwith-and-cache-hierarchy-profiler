#!/bin/bash
#Run scipt to test output of parallel implementation against sequential impl

rm b1-*.pgm

total_err=0

for num_threads in 1 4 16 64 256 1024
do    

    ./main.out -b1 -m1 -f4 -o b1-baseline-n${num_threads}.pgm
    ./main.out -b1 -n${num_threads} -m2 -f4 -o b1-m2-f4-n${num_threads}.pgm
    ./main.out -b1 -n${num_threads} -m3 -f4 -o b1-m3-f4-n${num_threads}.pgm
    ./main.out -b1 -n${num_threads} -m4 -f4 -o b1-m4-f4-n${num_threads}.pgm
    ./main.out -b2 -m1 -f4 -o b2-baseline-n${num_threads}.pgm
    ./main.out -b2 -n${num_threads} -m2 -f4 -o b2-m2-f4-n${num_threads}.pgm
    ./main.out -b2 -n${num_threads} -m3 -f4 -o b2-m3-f4-n${num_threads}.pgm
    ./main.out -b2 -n${num_threads} -m4 -f4 -o b2-m4-f4-n${num_threads}.pgm
    list="m2-f4-n${num_threads}.pgm m3-f4-n${num_threads}.pgm m4-f4-n${num_threads}.pgm"
    echo "list:" $list
    for preset in b1 b2
    do
        for i in $list
        do
            result=$(diff ${preset}-baseline-n${num_threads}.pgm ${preset}-$i) 
            if [ $? -eq 0 ]
            then 
                echo $i" Parallelization on "${preset}" (No filter) passes *"
            else
                total_err=$((total_err+1))
                echo $i" Parallelization on "${preset}" (No filter) FAILS  X"
            fi
        done
    done
done

if [ $total_err -eq 0 ]
then
    echo "FAIL - Parallel implementation is not equivalent"
fi
exit $total_err



