#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <math.h>

void readToArray(FILE *fn, int numLines, int *numArray);
void god(int signal);

// List of PIDS if we need to clear them
int* listOfPIDS;
int numOfPIDS = 0;

int main(int argc, char* argv[]){
	// Set up 100 second timer
	struct itimerval time;
	time.it_value.tv_sec = 100;
	time.it_value.tv_usec = 0;
	time.it_interval = time.it_value;
	signal(SIGALRM, god);
	setitimer(ITIMER_REAL, &time, NULL);
	
	// Set up catching ctrl c
	signal(SIGINT, god);
	signal(SIGPROF, god);

	// Get contents from file
	// Get Shared memory key
	key_t key1 = ftok("./master.c", 1);
	if(key1 == -1){
		perror("Error: Failed to get key in master.c for shared memory array.");
		return EXIT_FAILURE;
	}

	// Get shared memory array key
	int arrID = shmget(key1, sizeof(int), 0666 | IPC_CREAT);
	if(arrID == -1){
		perror("ERROR: Failed to get shared memory id for arrID in master.c");
		return EXIT_FAILURE;
	}

	// Attach memory key
	int *numbers = (int*)shmat(arrID,(void*)0,0);
	if(numbers == (void*)-1){
		perror("ERROR: Failed to attach memory in master.c for numbers array");
		return EXIT_FAILURE;
	}
	FILE *fn = fopen("numFile", "r");
	if(!fn){
		perror("Error opening file in master.c\n");
		return EXIT_FAILURE;
	}

	// Check how many numbers are in the file
	int numLines = 0;
	char c;
	while(!feof(fn)){
		c = fgetc(fn);
		if(c == '\n'){
			numLines++;
		}
	}

	readToArray(fn, numLines, numbers);
	printf("count: %d\n", numLines);
	// Read numbers into array
	/*
	rewind(fn);	// Rewind file
	int i;
	for(i = 0; i < numLines; i++){
		fscanf(fn, "%d", &numbers[i]);
	}

	*/
	//Testing elements in the array
	int i;
	for(i = 0; i < numLines;i++){
		printf("Element %d is %d\n", i, numbers[i]);
	}

	fclose(fn);
	// Create children until done
	int exitCount = 0;
	int childDone = 0;
	int arrIndex = 0;
	int activeChildren = 0;
	int numAdd = 2;
	pid_t pid;
	int status;
	//listOfPIDS = calloc(numLines, (sizeof(int)));
	
	/*
	// Computation 1
	while(numAdd < numLines){
		if(exitCount < numLines && activeChildren < 20 && childDone < numLines){
			pid = fork();
			//printf("pid: %d\n", pid);
			// Fork error
			if(pid < 0){
				perror("Forking error");
				return EXIT_FAILURE;
			}else if(pid == 0){
				char convertIndex[15];
				char convertAdd[15];
				sprintf(convertIndex, "%d", arrIndex);
				sprintf(convertAdd, "%d", numAdd);
				char *args[] = {"./bin_adder", convertIndex, convertAdd, NULL};
				execvp(args[0], args);
			}
			listOfPIDS[numOfPIDS] = pid;
			numOfPIDS++;
			childDone++;
			activeChildren++;
			// Increment array index
			arrIndex = arrIndex + numAdd;
		}
		// If the index of the array is greater then the number of elements in the array
		if(arrIndex >= numLines){
			// Increment the number of elements we will be adding together
			numAdd = numAdd * 2;
			// Wait for all child processes to stop
			wait(NULL);
			// Increment exit count
			exitCount = exitCount + activeChildren;
			// Since we waited for all children to exit, we want to set the amount of active children to 0
			activeChildren = 0;
			// If the number of elements we are adding together is greater then the number of lines that exist, we are finished
			if(numAdd > numLines){
				break;
			}
			// Reset the index
			arrIndex = 0;
		}
		// Check if child has ended
		if((pid = waitpid((pid_t)-1, &status, WNOHANG)) > 0){
			if(WIFEXITED(status)){
				//printf("Exit the child\n");
				activeChildren--;
				exitCount++;
			}
		}
		//printf("activeChildren: %d\n", activeChildren);
		// Absolute fail safe for child processes
		if(activeChildren > 20){
			god(1);
		}
	}
	*/
	
	readToArray(fn, numLines, numbers);
	// Computation 2
	numAdd = log2(numLines + 1);
	printf("num add = %d\n", numAdd);

	// Clear up shared memory
	free(listOfPIDS);
	shmdt(numbers);
	shmctl(arrID, IPC_RMID, NULL);
	return 0;
}

void readToArray(FILE *fn, int numLines, int *numArray){
	rewind(fn);
	int i;
	for(i = 0; i < numLines; i++){
		fscanf(fn, "%d", &numArray[i]);
	}
}

void god(int signal){
	int i;
	for(i = 0; i < numOfPIDS; i++){
		kill(listOfPIDS[1], SIGTERM);
	}

	printf("GOD HAS BEEN CALLED AND THE RAPTURE HAS BEGUN. SOON THERE WILL BE NOTHING\n");
	free(listOfPIDS);
	kill(getpid(), SIGTERM);
}
