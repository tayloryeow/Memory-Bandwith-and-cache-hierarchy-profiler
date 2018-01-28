
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#define BILLION 1000000000L

static  


//Simple dummy testing function for dev purposes
int dummy_function (){
    for (int i = 0; i < BILLION; i++);
}

int main (int argc, char **argv){
	char *test_name = argv[1];
	char *filePath = argv[3];
	int numberSamples = atoi(argv[2]);
	struct timespec start, stop;

	FILE *frec = NULL;

	//Checks number of arguments from 2 to 3
    if (argc < 3){
			printf("Usage: %s test_name, number_samples <output_file>\n", argv[0]);
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
	frec = fopen(filePath, "a+"); 
	if (frec == NULL){
		perror("Bad File path");
		exit(1);
	}

    //Determine PC Hash/ID
    int pcId = 0;
    printf("Beginning experiments++++++++++++++++++++++++++++++++++++++++\n");
    for (int sampleNumber = 0; sampleNumber < numberSamples; sampleNumber++){
    	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
	    	perror("clock gettime start");
			exit(EXIT_FAILURE);
		}

	    //Call Testing Function - 
		dummy_function();	
	
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
        	perror("clock gettime stop");
			exit(EXIT_FAILURE);
		}

    	uint64_t nsecs = difftimespec();

    
    	//Writing to file section
    	enum mode {DIFFERENCE, INTERVAL} m = DIFFERENCE;
		switch (m){
			case INTERVAL:
				break;
			case DIFFERENCE:
	        	fprintf(frec, "%d %ld\n", pcId, nsecs); 
				printf("Experiment number %d: Took %ld\n", sampleNumber, nsecs);
     			break;		
		}
    }
	fclose(frec);

	return 0;
}
