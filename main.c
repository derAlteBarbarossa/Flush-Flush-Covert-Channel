#include "util.h"

int main(int argc, char** argv)
{
	int threshold;
	char string[100] = "Hassan!";
	volatile char *channel = (char*) malloc(sizeof(char));

	int inFile = open(SHARED_FILE, O_RDWR);
	void *mapaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, inFile, 0);
	if(mapaddr == MAP_FAILED)
	{
		printf("Could not map the file\n");
		exit(-1);
	}

	channel = (char*) mapaddr;

	threshold = find_threshold(channel);
	printf("Threshold: %d\n", threshold);

	

	// demo(string, channel, threshold);
	demo_child_parent(string, channel, threshold);
}