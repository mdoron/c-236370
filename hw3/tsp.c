#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_PATH 10000000
#define SERIAL_VAR 7
#define PREF_SIZE 4
const int root = 0;
#define PREFIX_LENGTH 5
#define TRUE 1
#define FALSE 0
#define LISTEN while(TRUE)

typedef struct task_t {
	int prefix[PREFIX_LENGTH];
	char finish;
} Task;

void fillPrefs(int procsNum,int citiesNum, int r, int size, int count, int** prefs);
int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum);
int ABS(int n);
int find(int* prefix, int len, int initialWeight, int* bestPath,int* xCoord,int* yCoord,int citiesNum);
int findRec(int current, int curWeight, int* path, int* used, int* bestPath,int* xCoord,int* yCoord,int citiesNum);

int ABS(int n);
int getMax(int arr[], int* ind, int size);
void sort(int arr[], int size);
void swap(int* x, int* y);
int prefixWeight(int prefix[]);


int nextPermut(int* prefix);
void createTask(int prefix[], char finish);


int* globalxCoord;
int* globalyCoord;
int* minNextEdgesWeight;
int globalCitiesNum;
int** dists;
int numProcs;
int myRank;

int prevBound;
int localBound;
Task* task;

enum Tag { ASK_FOR_JOB, REPORT, NOTHING_TO_REPORT, REPORT_WEIGHT, REPORT_PATH, NEW_JOB, NEW_BOUND };

void build(Task* data, MPI_Datatype* message_type_ptr) {
	int block_lengths[2];
	MPI_Aint displacements[2];
	MPI_Datatype typelist[2];
	MPI_Aint addresses[3];  // Helper array

	// First specify the types
	typelist[0] = MPI_INT;
	typelist[1] = MPI_CHAR;

	// Specify the number of elements of each type
	block_lengths[0] = PREFIX_LENGTH;
	block_lengths[1] = 1;

	// Calculate the displacements of the members  relative to indata
	MPI_Get_address(data, &addresses[0]);
	MPI_Get_address(&(data->prefix), &addresses[1]);
	MPI_Get_address(&(data->finish), &addresses[2]);

	displacements[0] = addresses[1] - addresses[0];
	displacements[1] = addresses[2] - addresses[0];

	// Create the derived type
	MPI_Type_create_struct(2, block_lengths,displacements, typelist, message_type_ptr);

	// Commit it so that it can be used
	MPI_Type_commit(message_type_ptr);
}

// The dynamic parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	if(numProcs == 1)
		exit(0);
	int i;
	int bestWeight = INT_MAX;
	prevBound = INT_MAX;
	localBound = INT_MAX;
	int bestPath[citiesNum];
	MPI_Datatype MPI_Task;
	Task t;
	build(&t,&MPI_Task);
	MPI_Status status;
	char info;
	int path[citiesNum];
	int weight;
	
	globalCitiesNum = citiesNum;
	globalxCoord = xCoord;
	globalyCoord = yCoord;
	localBound = INT_MAX;
	int minEdges[citiesNum];
	minNextEdgesWeight = minEdges;
	task = (Task*) malloc(sizeof(Task));

	MPI_Request request;
	
	printf("@@@");
	fflush(stdout);
	
	if(myRank == 0) {
		int prefix[PREFIX_LENGTH];
		int i;
		for(i = 0; i < PREFIX_LENGTH; i++) { 
			prefix[i] = i;
		}
		printf("!!!");
		do {
			createTask(prefix, FALSE);
			LISTEN {
				printf("###");
				fflush(stdout);
				MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				int source = status.MPI_SOURCE;
				if(status.MPI_TAG == ASK_FOR_JOB) {
					// check if need irecv
					MPI_Recv(&info,1,MPI_CHAR,source,ASK_FOR_JOB, MPI_COMM_WORLD, &status);
					MPI_Issend(task, 1, MPI_Task, source, NEW_JOB, MPI_COMM_WORLD,&request);
					break;
				}
				else if(status.MPI_TAG == REPORT) {
					printf("^^^");
					fflush(stdout);
					MPI_Recv(&info,1,MPI_CHAR,source,REPORT, MPI_COMM_WORLD, &status);
					MPI_Recv(&weight,1,MPI_INT,source,REPORT_WEIGHT, MPI_COMM_WORLD, &status);
					MPI_Recv(path,citiesNum,MPI_INT,source,REPORT_PATH, MPI_COMM_WORLD, &status);
					if(weight < bestWeight) {
						bestWeight = weight;
						memcpy(bestPath, path, citiesNum * sizeof(int));
						for(i = 1; i < numProcs; ++i)
							MPI_Issend(&bestWeight, 1, MPI_INT, i, NEW_BOUND, MPI_COMM_WORLD, &request);
					}
				}
			}
			printf("()()()");
		} while(nextPermut(prefix));
		printf("===");
		fflush(stdout);
		createTask(prefix, TRUE);
		for(i = 1; i < numProcs; ++i)
			MPI_Issend(task, 1, MPI_Task, i, NEW_JOB, MPI_COMM_WORLD, &request);
		int count = 0;
		while(count < numProcs - 1) {
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int source = status.MPI_SOURCE;
			if(status.MPI_TAG == ASK_FOR_JOB) {
				MPI_Recv(&info,1,MPI_CHAR,source,ASK_FOR_JOB, MPI_COMM_WORLD, &status);
				MPI_Issend(task, 1, MPI_Task, source, NEW_JOB, MPI_COMM_WORLD, &request);
				continue;
			}
			else if(status.MPI_TAG == REPORT) {
				MPI_Recv(&info,1,MPI_CHAR,source,REPORT, MPI_COMM_WORLD, &status);
				MPI_Recv(&weight,1,MPI_INT,source,REPORT_WEIGHT, MPI_COMM_WORLD, &status);
				MPI_Recv(path,citiesNum,MPI_INT,source,REPORT_PATH, MPI_COMM_WORLD, &status);
				if(weight < bestWeight) {
					bestWeight = weight;
					memcpy(bestPath, path, citiesNum * sizeof(int));
				}
			}
			else if(status.MPI_TAG == NOTHING_TO_REPORT) {
				MPI_Recv(&info,1,MPI_CHAR,source,NOTHING_TO_REPORT, MPI_COMM_WORLD, &status);
				++count;
			}
		}
		memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
	}
	else {
		LISTEN {
			info = ASK_FOR_JOB;
			MPI_Issend(&info,1,MPI_CHAR,0,ASK_FOR_JOB,MPI_COMM_WORLD, &request);
			do {
				MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				if(status.MPI_TAG == NEW_JOB)
					MPI_Recv(task,1,MPI_Task,0,NEW_JOB, MPI_COMM_WORLD, &status);
				else if(status.MPI_TAG == NEW_BOUND) {
					int prevBOund = localBound;
					MPI_Recv(&localBound,1,MPI_INT,0,NEW_BOUND, MPI_COMM_WORLD, &status);
					if(prevBOund < localBound)
						localBound = prevBound;
				}
			} while(status.MPI_TAG != NEW_JOB);

			if(task->finish) {
				MPI_Issend(&info,1,MPI_CHAR,0,NOTHING_TO_REPORT,MPI_COMM_WORLD, &request);
				break;
			}
			weight = find(task->prefix, PREFIX_LENGTH, prefixWeight(task->prefix), path,xCoord,yCoord,citiesNum);
			if(weight < localBound) {
				info = REPORT;
				MPI_Ssend(&info,1,MPI_CHAR,0,REPORT,MPI_COMM_WORLD);
				MPI_Ssend(&weight,1,MPI_INT,0,REPORT_WEIGHT,MPI_COMM_WORLD);
				MPI_Ssend(path,citiesNum,MPI_INT,0,REPORT_PATH,MPI_COMM_WORLD);
			}
		}
		
	}

	for(i = 0; i < globalCitiesNum; i++) {
		free(dists[i]);
	}
	free(dists);
	free(task);

	return bestWeight;
}

void swap(int* x, int* y) {
	int tmp = *x;
	*x = *y;
	*y = tmp;
}

//keep first node always 0
int nextPermut(int* prefix) {
	int i, j;
	for(i = PREFIX_LENGTH-2; i >= 1; --i)
		if(prefix[i] < prefix[i+1])
			break;
	if(i < 1) // last permutation
		return 0;
	for(j = PREFIX_LENGTH-1; j > i; --j)
		if(prefix[j] > prefix[i])
			break;
	swap(prefix+i, prefix+j);
	++i;
	for(j = PREFIX_LENGTH-1; j > i; --j, ++i)
		swap(prefix+i, prefix+j);
	return 1;
}

int prefixWeight(int prefix[]) {
	int w = 0;
	int i;
	for(i = 0; i < PREFIX_LENGTH - 1;++i)
		w += dists[prefix[i]][prefix[i+1]];
	return w;
}


int getMax(int arr[], int* ind, int size) {
	int i;
	*ind = 0;
	for(i = 0; i < size; ++i)
		if(arr[i] > arr[*ind])
			*ind = i;
	return arr[*ind];
}

void sort(int arr[], int size) {
	int i, j;
	int min = 0;
	for(i  = 0; i < size; ++i) {
		for(j = i + 1; j < size; ++j)
			if(arr[j] < arr[min])
				min = j;
		int tmp = arr[i];
		arr[i] = arr[min];
		arr[min] = tmp;
	}
}

void workerInitialize() {

}

void createTask(int prefix[], char finish) {
	memcpy(task->prefix, prefix, PREFIX_LENGTH * sizeof(int));
	task->finish = finish;
}



//===================================================================
//===================================================================

int ABS(int a) {
  return a>0 ? a : a*(-1);
}

//gets 2 cities and coordinates and citiesNum
//returns the manhatten distance
int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum) {
  return city1==city2? 0 : (ABS(xCoord[city1]-xCoord[city2])+ABS(yCoord[city1]-yCoord[city2]));
}


/*
@param current - the current index we are working on - last one will be the stop
@param path - current path
@param curWeight - curr path weight
@param bestPath - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int findRec(int current, int curWeight, int* path, int* used, int* bestPath,int* xCoord,int* yCoord,int citiesNum) {
	if(current == citiesNum - 1) {
		memcpy(bestPath, path, citiesNum * sizeof(int));
		return curWeight + getDist(path[0],path[citiesNum - 1],xCoord,yCoord,citiesNum);
	}
	int bestWeight = MAX_PATH;
	int receivedPath[citiesNum];
	for(int i = 0; i < citiesNum; ++i) {
		if(used[i] == 1)
			continue;
		int check_now = curWeight + getDist(path[current],i,xCoord,yCoord,citiesNum);
		path[current + 1] = i;
		used[i] = 1;
		int weight = findRec(current + 1, check_now, path, used, receivedPath,xCoord,yCoord,citiesNum);
		used[i] = 0;
		if(weight < bestWeight) {
			bestWeight = weight;
			memcpy(bestPath, receivedPath, citiesNum * sizeof(int));
		}
	}
	return bestWeight;
}



/*
@param prefix - gets prefix of a path
@param len - the prefix length
@param initialWeight - prefix path weight
@param bestPath - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int find(int* prefix, int len, int initialWeight, int* bestPath,int* xCoord,int* yCoord,int citiesNum) {
	int used[citiesNum];
  int path[citiesNum];
	memset(used, 0, citiesNum * sizeof(int));
	memcpy(path, prefix, len * sizeof(int));
	for(int i = 0; i < len; ++i)
		used[prefix[i]] = 1;
	return findRec(len - 1, initialWeight, path, used, bestPath,xCoord, yCoord,citiesNum);
}


// returns all path prefixes the process have to solve, according to it's rank and number of processes
void fillPrefs(int procsNum,int citiesNum, int r, int size, int firstIndex, int** prefs) {
	int i,j;
	for(i = firstIndex, j = 0; i < firstIndex + size; ++i, ++j) {
		prefs[j] = malloc(PREF_SIZE * sizeof(int));
		prefs[j][0] = 0;	// all prefixes start at city 0
		prefs[j][1] = 1 + i / (citiesNum - 2);	// according to the number of branch in the tree of prefixes
		prefs[j][2] = 1 + i % (citiesNum - 3);	// according to the number of branch in the tree of prefixes
		if(prefs[j][1] <= prefs[j][2])
			++prefs[j][2];
	}
}