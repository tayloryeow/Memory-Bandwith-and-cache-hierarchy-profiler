#!/bin/bash

pgm_dir="pgm/"
out_pfx="out/"
array=$(ls $pgm_dir)

mkdir $out_pfx

for i in $(ls $pgm_dir)
do
	if ./pgm_validator.sh $pgm_dir$i; then
		for number in 1 2 3 
		do
			echo "Input: " $pgm_dir${i}
			echo "Output: " $out_pfx$number$i
		
			./main.out -i $pgm_dir$i -f${number} -m1 -o $out_pfx${number}$i
		
		done
	else
		echo "Invalid .pgm -> "$pgm_dir$i
	fi		
done

