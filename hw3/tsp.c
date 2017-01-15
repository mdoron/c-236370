#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#define PREF_SIZE 5
#define TRUE 1
#define FALSE 0

typedef struct job_t {
	int pref[PREF_SIZE];
	char isDone;
} Job;

// int find(int pref[], int len, int initialWeight, int* bestPath);
// int findRec(int curInd, int curWeight, int* path, int* used, int* bestPath);
// int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum);
// int ABS(int n);
// int find(int* pref, int len, int initialWeight, int* bestPath,int* xCoord,int* yCoord,int citiesNum);
// int findRec(int current, int curWeight, int* path, int* used, int* bestPath,int* xCoord,int* yCoord,int citiesNum);

	
int getDist(int city1, int city2, int *xCoord, int* yCoord, int citiesNum);
int ABS(int n);
int find(int* pref, int len, int initialWeight, int* bestPath,int* xCoord,int* yCoord,int citiesNum);
int findRec(int current, int curWeight, int* path, int* used, int* bestPath, int* xCoord, int* yCoord, int citiesNum);
void calcMinEdges();
int max(int arr[], int size, int* ind);

void sort(int arr[], int size);
void swap(int* x, int* y);
int nextPermut(int* pref);
void allInitialize(int citiesNum, int* xCoord, int* yCoord);
void allDestruct();
void masterInitialize(int pref[]);
void workerInitialize();
void createJob(int pref[], char isDone);
int prefWeight(int pref[]);
void calcDists();

const int root = 0;
int* globalxCoord;
int* globalyCoord;
int* minNextEdgesWeight;
int globalCitiesNum;
int** dists;
int localBound;
int procsNum;
int myRank;
Job* job;

enum Tag { ASK_FOR_JOB, REPORT, NOTHING_TO_REPORT, REPORT_WEIGHT, REPORT_PATH, NEW_JOB, NEW_BOUND };

void setData(Job* data, MPI_Datatype* message_type_ptr) {
	int block_lengths[2];
	MPI_Aint displacements[2];
	MPI_Datatype typelist[2];
	MPI_Aint addresses[3];  // Helper array

	// First specify the types
	typelist[0] = MPI_INT;
	typelist[1] = MPI_CHAR;

	// Specify the number of elements of each type
	block_lengths[0] = PREF_SIZE;
	block_lengths[1] = 1;

	// Calculate the displacements of the members  relative to indata
	MPI_Get_address(data, &addresses[0]);
	MPI_Get_address(&(data->pref), &addresses[1]);
	MPI_Get_address(&(data->isDone), &addresses[2]);

	displacements[0] = addresses[1] - addresses[0];
	displacements[1] = addresses[2] - addresses[0];

	// Create the derived type
	MPI_Type_create_struct(2, block_lengths,displacements, typelist, message_type_ptr);

	// Commit it so that it can be used
	MPI_Type_commit(message_type_ptr);
}

// The dynamic parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[]) {
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &procsNum);
	if(procsNum == 1) 
		exit(0);
	int i;
	int bestWeight = INT_MAX;
	int bestPath[citiesNum];
	MPI_Datatype MPI_Job;
	Job t;
	setData(&t,&MPI_Job);
	MPI_Status status;
	char info;
	int path[citiesNum];
	int weight;
	allInitialize(citiesNum, xCoord, yCoord);

	if(myRank == root) {
		int pref[PREF_SIZE];
		masterInitialize(pref);

		do {
			createJob(pref, FALSE);
			while(TRUE) {
				MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				int source = status.MPI_SOURCE;
				if(status.MPI_TAG == ASK_FOR_JOB) {
					// check if need irecv
					MPI_Recv(&info, 1, MPI_CHAR, source, ASK_FOR_JOB, MPI_COMM_WORLD, &status);
					MPI_Ssend(job, 1, MPI_Job, source, NEW_JOB, MPI_COMM_WORLD);
					break;
				}
				else if(status.MPI_TAG == REPORT) {
					MPI_Recv(&weight, 1, MPI_INT, source, REPORT_WEIGHT, MPI_COMM_WORLD, &status);
					MPI_Recv(path, citiesNum, MPI_INT, source, REPORT_PATH, MPI_COMM_WORLD, &status);
					if(weight < bestWeight) {
						bestWeight = weight;
						memcpy(bestPath, path, citiesNum * sizeof(int));
						for(i = 0; i < procsNum; ++i)
							if(i != root)
								MPI_Ssend(&bestWeight, 1, MPI_INT, i, NEW_BOUND, MPI_COMM_WORLD);
					}
				}
			}
		} while(nextPermut(pref));
		createJob(pref, TRUE);
		for(i = 0; i < procsNum; ++i)
			if(i != root)
				MPI_Ssend(job, 1, MPI_Job, i, NEW_JOB, MPI_COMM_WORLD);
		int count = 0;
		while(count < procsNum - 1) {
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
					for(i = 0; i < procsNum; ++i)
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
					MPI_Recv(job,1,MPI_Job,root,NEW_JOB, MPI_COMM_WORLD, &status);
				else if(status.MPI_TAG == NEW_BOUND)
					MPI_Recv(&localBound,1,MPI_INT,root,NEW_BOUND, MPI_COMM_WORLD, &status);
			} while(status.MPI_TAG != NEW_JOB);

			if(job->isDone) {
				MPI_Ssend(&info,1,MPI_CHAR,root,NOTHING_TO_REPORT,MPI_COMM_WORLD);
				break;
			}
			weight = find(job->pref, PREF_SIZE, prefWeight(job->pref), path, xCoord, yCoord, citiesNum);
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
int nextPermut(int* pref) {
	int i, j;
	for(i = PREF_SIZE-2; i >= 1; --i)
		if(pref[i] < pref[i+1])
			break;
	if(i < 1) // last permutation
		return 0;
	for(j = PREF_SIZE-1; j > i; --j)
		if(pref[j] > pref[i])
			break;
	swap(pref+i, pref+j);
	++i;
	for(j = PREF_SIZE-1; j > i; --j, ++i)
		swap(pref+i, pref+j);
	return 1;
}

int prefWeight(int pref[]) {
	int w = 0;
	int i;
	for(i = 0; i < PREF_SIZE - 1;++i)
		w += dists[pref[i]][pref[i+1]];
	return w;
}

int getDist(int i, int j) {
	return ABS(globalxCoord[i] - globalxCoord[j]) + ABS(globalyCoord[i] - globalyCoord[j]);
}

int ABS(int n) {
	return n >= 0 ? n : -n;
}

void calcDists() {
	int i, j;
	dists = (int**) malloc(globalCitiesNum *sizeof(int*));
	for(i = 0; i < globalCitiesNum; ++i) {
		dists[i] = (int*) malloc(globalCitiesNum * sizeof(int));
		for(j = 0; j < globalCitiesNum; j++)
			dists[i][j] = getDist(i,j);
	}
}

void calcMinEdges() {
	int i, j;
	minNextEdgesWeight[0] = 0;
	int curMaxInd = 0;
	int curMax = 0;		// max in the minNextEdgesWeight array. will be replaced when finding lower weight
	for(i = 1; i < globalCitiesNum; ++i)
		minNextEdgesWeight[i] = getDist(0,i);
	curMax = max(minNextEdgesWeight, globalCitiesNum, &curMaxInd);
	for(i = 1; i < globalCitiesNum; ++i)
		for(j = i + 1; j < globalCitiesNum; ++j) {
			int w = getDist(i,j);
			if(w < curMax) {
				minNextEdgesWeight[curMaxInd] = w;
				curMax = max(minNextEdgesWeight, globalCitiesNum, &curMaxInd);
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

int max(int arr[], int size, int* ind) {
	int i;
	int maxId = 0;
	for(i = 0; i < size; i++)
		if(arr[i] > arr[maxId])
			maxId = i;
	*ind = maxId;
	return arr[maxId];
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

void allInitialize(int citiesNum, int* xCoord, int* yCoord) {
	globalCitiesNum = citiesNum;
	globalxCoord = xCoord;
	globalyCoord = yCoord;
	localBound = INT_MAX;
	calcDists();
	int minEdges[citiesNum];
	minNextEdgesWeight = minEdges;
	calcMinEdges();
	job = (Job*) malloc(sizeof(Job));
}

void allDestruct() {
	int i;
	for(i = 0; i < globalCitiesNum; ++i)
		free(dists[i]);
	free(dists);
	free(job);
}

void masterInitialize(int pref[]) {
	int i;
	for(i = 0; i < PREF_SIZE; ++i)
		pref[i] = i;
}

void workerInitialize() {

}

void createJob(int pref[], char isDone) {
	memcpy(job->pref, pref, PREF_SIZE * sizeof(int));
	job->isDone = isDone;
}
