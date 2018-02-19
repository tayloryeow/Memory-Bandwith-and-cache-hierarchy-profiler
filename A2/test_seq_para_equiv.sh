#!/bin/bash
#Run scipt to test output of parallel implementation against sequential impl

rm b1-*.pgm

total_err=0

#Iterate through all parameter permutations with progressively larger thread_numbers
#Test the mode = 1 type output (sequential) against the parall output (mode = 2|3|4)
for num_threads in 1 4 16 64 256 1024
do    
    echo "THREAD LOAD: "$num_threads
    #Different built in implementations
    for preset in b1 b2
    do
        #All 4 filters. 
        for filter in 1 2 3 4
        do     
            #Set/Create serialization comparison file
            base_out=${preset}-baseline-n${num_threads}-f${filter}.pgm
            ./main.out -${preset} -m1 -f${filter} -o ${base_out}
   
            #Create the parallel comparison file
            for thread_modes in 2 3 4
            do 
                #Create parallization output with the these parameters
                out_name=${preset}-m${num_threads}-f${filter}-n${num_threads}.pgm
                ./main.out -${preset} -n${num_threads} -m${thread_modes} -f${filter} -o ${out_name}

                #Compare Files
                result=$(diff ${base_out} ${out_name})
                if [ $? -eq 0 ]
                then 
                    echo "TEST: Apply "$num_threads" thread(s) on "$preset" using filter="$filter" | m:"$thread_modes" passes *"
                else
                    total_err=1
                    echo "TEST: Apply "$num_threads" thread(s) on "$preset" using filter="$filter" | m:"$thread_modes" FAILS X"
                fi

                #remove parralel test file
                rm ${out_name}
            done
            rm ${base_out}
        done
    done
done

if [ $total_err -eq 0 ]
then
    echo "***************************************************************************************"
    echo "* ALL TESTS PASSING * Parallel implementation is equivalent with sequential implemntation"
    echo "*********************"
else
    echo "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
    echo "X FAIL X Parallel implementation is not equivalent"
    echo "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
fi
exit $total_err



