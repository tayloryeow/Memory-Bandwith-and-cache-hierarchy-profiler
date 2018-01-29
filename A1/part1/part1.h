
enum mode{
	INTERVAL,
	DIFFERENCE
};

enum tests{
	DUMMY, 
	BAD
};

int dummy_function (int dataSize);
int bad_function();
int poor_locality(int dataSize);

int algorithmn_enum(char *arg);

int run_experiment(FILE *frec, int numberSamples, int dataSize, int algorithmn, int depth);
void warm_cache(int dataSize);


