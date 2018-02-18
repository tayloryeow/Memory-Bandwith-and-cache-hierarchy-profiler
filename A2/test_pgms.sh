#!/bin/bash

pgm_dir="pgm/"
out_pfx="out/"
array=$(ls $pgm_dir)

mkdir $out_pfx

for i in $(ls $pgm_dir)
do
	if ./pgm_validator.sh $pgm_dir$i; then
		for number in 1 2 3 4 
		do
            output="$out_pfx"filter"${number}"mode"$2"-"$i"
            input=$pgm_dir${i}
			echo "Input: " $input
            echo "Output: " ${output} 
	

            if [[ $2 == 1 ]]; then
			    ./main.out -i ${input} -f${number} -m1 -o ${output}	
            fi
            if [[ $2 == 2 ]]; then
                ./main.out -i ${input} -f${number} -n1 -m2 -o ${output}
		    fi
            done

	else
		echo "Invalid .pgm -> "$pgm_dir$i
	fi		
done

