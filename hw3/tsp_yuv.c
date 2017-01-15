#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#define PREFIX_LENGTH 5
#define TRUE 1
#define FALSE 0

typedef struct task_t {
	int prefix[PREFIX_LENGTH];
	char finish;
} Task;

int weightOf(int i, int j);
int abs(int n);
int solve(int prefix[], int len, int initialWeight, int* bestPath);
int recurseSolve(int curInd, int curWeight, int* path, int* used, int* bestPath);

void calcMinEdges();
int getMax(int arr[], int* ind, int size);
void sort(int arr[], int size);
void swap(int* x, int* y);
int nextPermut(int* prefix);
void allInitialize(int citiesNum, int* xCoord, int* yCoord);
void allDestruct();
void masterInitialize(int prefix[]);
void workerInitialize();
void createTask(int prefix[], char finish);
int prefixWeight(int prefix[]);
void calcDists();

const int root = 0;
int* globalxCoord;
int* globalyCoord;
int* minNextEdgesWeight;
int globalCitiesNum;
int** dists;
int localBound;
int numProcs;
int myRank;
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
	int bestPath[citiesNum];
	MPI_Datatype MPI_Task;
	Task t;
	build(&t,&MPI_Task);
	MPI_Status status;
	char info;
	int path[citiesNum];
	int weight;
	allInitialize(citiesNum, xCoord, yCoord);

	if(myRank == root) {
		int prefix[PREFIX_LENGTH];
		masterInitialize(prefix);

		do {
			createTask(prefix, FALSE);
			while(TRUE) {
				MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				int source = status.MPI_SOURCE;
				if(status.MPI_TAG == ASK_FOR_JOB) {
					// check if need irecv
					MPI_Recv(&info,1,MPI_CHAR,source,ASK_FOR_JOB, MPI_COMM_WORLD, &status);
					MPI_Ssend(task, 1, MPI_Task, source, NEW_JOB, MPI_COMM_WORLD);
					break;
				}
				else if(status.MPI_TAG == REPORT) {
					MPI_Recv(&weight,1,MPI_INT,source,REPORT_WEIGHT, MPI_COMM_WORLD, &status);
					MPI_Recv(path,citiesNum,MPI_INT,source,REPORT_PATH, MPI_COMM_WORLD, &status);
					if(weight < bestWeight) {
						bestWeight = weight;
						memcpy(bestPath, path, citiesNum * sizeof(int));
						for(i = 0; i < numProcs; ++i)
							if(i != root)
								MPI_Ssend(&bestWeight, 1, MPI_INT, i, NEW_BOUND, MPI_COMM_WORLD);
					}
				}
			}
		} while(nextPermut(prefix));
		createTask(prefix, TRUE);
		for(i = 0; i < numProcs; ++i)
			if(i != root)
				MPI_Ssend(task, 1, MPI_Task, i, NEW_JOB, MPI_COMM_WORLD);
		int count = 0;
		while(count < numProcs - 1) {
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int source = status.MPI_SOURCE;
			if(status.MPI_TAG == ASK_FOR_JOB) {
				MPI_Recv(&info,1,MPI_CHAR,source,ASK_FOR_JOB, MPI_COMM_WORLD, &status);
				continue;
			}
			else if(status.MPI_TAG == REPORT) {
				MPI_Recv(&weight,1,MPI_INT,source,REPORT_WEIGHT, MPI_COMM_WORLD, &status);
				MPI_Recv(path,citiesNum,MPI_INT,source,REPORT_PATH, MPI_COMM_WORLD, &status);
				if(weight < bestWeight) {
					bestWeight = weight;
					memcpy(bestPath, path, citiesNum * sizeof(int));
					for(i = 0; i < numProcs; ++i)
						if(i != root)
							MPI_Ssend(&bestWeight, 1, MPI_INT, i, NEW_BOUND, MPI_COMM_WORLD);
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
		workerInitialize();

		while(TRUE) {
			info = ASK_FOR_JOB;
			MPI_Ssend(&info,1,MPI_CHAR,root,ASK_FOR_JOB,MPI_COMM_WORLD);
			do {
				MPI_Probe(root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				if(status.MPI_TAG == NEW_JOB)
					MPI_Recv(task,1,MPI_Task,root,NEW_JOB, MPI_COMM_WORLD, &status);
				else if(status.MPI_TAG == NEW_BOUND)
					MPI_Recv(&localBound,1,MPI_INT,root,NEW_BOUND, MPI_COMM_WORLD, &status);
			} while(status.MPI_TAG != NEW_JOB);

			if(task->finish) {
				MPI_Ssend(&info,1,MPI_CHAR,root,NOTHING_TO_REPORT,MPI_COMM_WORLD);
				break;
			}
			weight = solve(task->prefix, PREFIX_LENGTH, prefixWeight(task->prefix), path);
			if(weight < localBound) {
				info = REPORT;
				MPI_Ssend(&info,1,MPI_CHAR,root,REPORT,MPI_COMM_WORLD);
				MPI_Ssend(&weight,1,MPI_INT,root,REPORT_WEIGHT,MPI_COMM_WORLD);
				MPI_Ssend(path,citiesNum,MPI_INT,root,REPORT_PATH,MPI_COMM_WORLD);
			}
		}

	}
	allDestruct();
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

int weightOf(int i, int j) {
	return abs(globalxCoord[i] - globalxCoord[j]) + abs(globalyCoord[i] - globalyCoord[j]);
}

int abs(int n) {
	return n >= 0 ? n : -n;
}

void calcDists() {
	int i, j;
	dists = (int**) malloc(globalCitiesNum *sizeof(int*));
	for(i = 0; i < globalCitiesNum; ++i) {
		dists[i] = (int*) malloc(globalCitiesNum * sizeof(int));
		for(j = 0; j < globalCitiesNum; j++)
			dists[i][j] = weightOf(i,j);
	}
}

void calcMinEdges() {
	int i, j;
	minNextEdgesWeight[0] = 0;
	int curMaxInd = 0;
	int curMax = 0;		// max in the minNextEdgesWeight array. will be replaced when finding lower weight
	for(i = 1; i < globalCitiesNum; ++i)
		minNextEdgesWeight[i] = weightOf(0,i);
	curMax = getMax(minNextEdgesWeight, &curMaxInd, globalCitiesNum);
	for(i = 1; i < globalCitiesNum; ++i)
		for(j = i + 1; j < globalCitiesNum; ++j) {
			int w = weightOf(i,j);
			if(w < curMax) {
				minNextEdgesWeight[curMaxInd] = w;
				curMax = getMax(minNextEdgesWeight, &curMaxInd, globalCitiesNum);
			}
		}
	int min = minNextEdgesWeight[1];
	for(i = 2; i < globalCitiesNum; ++i)
		if(minNextEdgesWeight[i] < min)
			min = minNextEdgesWeight[i];
	minNextEdgesWeight[curMaxInd] = min;
	sort(minNextEdgesWeight, globalCitiesNum);
	for(i = 2; i < globalCitiesNum; ++i)
		minNextEdgesWeight[i] += minNextEdgesWeight[i - 1];
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

int solve(int prefix[], int len, int initialWeight, int* bestPath) {
	int used[globalCitiesNum];
	memset(used, FALSE, globalCitiesNum * sizeof(int));
	int path[globalCitiesNum];
	memcpy(path, prefix, len * sizeof(int));
	int i;
	for(i = 0; i < len; ++i)
		used[prefix[i]] = TRUE;
	return recurseSolve(len - 1, initialWeight, path, used, bestPath);
}

int recurseSolve(int curInd, int curWeight, int* path, int* used, int* bestPath) {
	if(curInd == globalCitiesNum - 1) {
		//adding edge from last city to first city to close the cycle
		memcpy(bestPath, path, globalCitiesNum * sizeof(int));
		return curWeight + weightOf(path[0],path[globalCitiesNum - 1]);
	}

	int bestWeight = INT_MAX;
	int receivedPath[globalCitiesNum];
	int i;
	for(i = 0; i < globalCitiesNum; ++i) {
		if(used[i] == TRUE)
			continue;
		if(curWeight + weightOf(path[curInd],i) + minNextEdgesWeight[globalCitiesNum - curInd - 1] >= localBound)	//bestWeight before
			continue;
		int ww = curWeight + weightOf(path[curInd],i);
		path[curInd + 1] = i;
		used[i] = TRUE;
		int weight = recurseSolve(curInd + 1, ww, path, used, receivedPath);
		used[i] = FALSE;
		if(weight < bestWeight) {
			bestWeight = weight;
			memcpy(bestPath, receivedPath, globalCitiesNum * sizeof(int));
		}
	}
	return bestWeight;
}

void allInitialize(int citiesNum, int* xCoord, int* yCoord) {
	globalCitiesNum = citiesNum;
	globalxCoord = xCoord;
	globalyCoord = yCoord;
	localBound = INT_MAX;
	calcDists();
	int minEdges[citiesNum];
	minNextEdgesWeight = minEdges;
	calcMinEdges();
	task = (Task*) malloc(sizeof(Task));
}

void allDestruct() {
	int i;
	for(i = 0; i < globalCitiesNum; ++i)
		free(dists[i]);
	free(dists);
	free(task);
}

void masterInitialize(int prefix[]) {
	int i;
	for(i = 0; i < PREFIX_LENGTH; ++i)
		prefix[i] = i;
}

void workerInitialize() {

}

void createTask(int prefix[], char finish) {
	memcpy(task->prefix, prefix, PREFIX_LENGTH * sizeof(int));
	task->finish = finish;
}
