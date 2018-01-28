#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include "time_util.h"

#define BILLION 1000000000L

int bad_function(){
	int x[BILLION] = {};
	x[0] = 2;
	for (int i = 1; i < BILLION; i++){
		x[i] = x[i - 1] * x[i - 1];
    }
	return 0;
}

//Simple dummy testing function for dev purposes
int dummy_function (int dataSize){
    unsigned int x[dataSize];
	int total = 0;
	for (int i = 0; i < dataSize; i++){
        x[i] = i;
		total = x[i];
	    
	}
	return total;
}

int main (int argc, char **argv){
	char *test_name = argv[1];
	char filePath[160] = "data/";	
	strcat(filePath, test_name);
	int numberSamples = atoi(argv[2]);
	int dataSize = atoi(argv[3]);//Number of bytes to test
	struct timespec start, stop;

	FILE *frec = NULL;

	//Checks number of arguments from 2 to 3
    if (argc < 4){
			printf("Usage: %s test_name, number_samples <output_file> dataSize\n", argv[0]);
			exit(1);
	}
 

	// Pin the thread to a single CPU to minimize effects of scheduling
	// Don't use CPU #0 if possible, it tends to be busier with servicing interrupts
	srandom(time(NULL));
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET((random() ?: 1) % get_nprocs(), &set);
	if (sched_setaffinity(getpid(), sizeof(set), &set) != 0){
			perror("sched_setaffinity");
			return 1;
	}
/*	
	if (argc == 2){
		char def_path[256] = "data/";
        if (strlen(test_name) > 250){
		    printf("Test_name too long > 250\n");
		    exit(1);
		}	   
        strncpy(def_path, test_name, 250);
	    filePath = &def_path;	
	} */ 
    printf("filepath: %s\n", filePath); 
	frec = fopen(filePath, "w+"); 
	if (frec == NULL){
		perror("Bad File path");
		exit(1);
	}

    //Determine PC Hash/ID
    int pcId = 0;
	enum TESTS {DUMMY, BAD} function = DUMMY; 
    printf("Beginning experiments++++++++++++++++++++++++++++++++++++++++\n");
    for (int sampleNumber = 0; sampleNumber < numberSamples; sampleNumber++){
    	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
	    	perror("clock gettime start");
			exit(EXIT_FAILURE);
		}

	    //Call Testing Function - 
		switch (function){
			case DUMMY:
		    	dummy_function(dataSize);
				printf("dummy_function\n");	
				break;
			case BAD:
				bad_function();
				break;
		}	
	
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
        	perror("clock gettime stop");
			exit(EXIT_FAILURE);
		}

    	struct timespec diff = difftimespec(stop, start);
		long int msec = timespec_to_nsec(diff);

    
    	//Writing to file section
    	enum mode {DIFFERENCE, INTERVAL} m = DIFFERENCE;
		switch (m){
			case INTERVAL:
				break;
			case DIFFERENCE:
	        	fprintf(frec, "%d %d %d %ld\n", pcId, function, dataSize, msec); 
				printf("Experiment number %d: Took %ld to write %d\n", sampleNumber, msec, dataSize);
     			break;		
		}
    }
	fclose(frec);

	return 0;
}

