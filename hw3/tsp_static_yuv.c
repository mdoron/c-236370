#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int** initialPrefixes(int citiesNum, int* size);
int weightOf(int i, int j);
int abs(int n);
int solve(int prefix[], int len, int initialWeight, int* bestPath);
int recurseSolve(int curInd, int curWeight, int* path, int* used, int* bestPath);
void calcMinEdges();
int getMax(int arr[], int* ind, int size);
void sort(int arr[], int size);
void calcDists();

const int root = 0;
int* globalxCoord;
int* globalyCoord;
int* minNextEdgesWeight; // in index i, stores the sum of i weight of minimum edges
int globalCitiesNum;
int** dists; // dist[i][j] stores the weight of edge i--j
int numProcs;
int myRank;

#define PREFIX_LENGTH 3
#define TRUE 1
#define FALSE 0

enum Message { CITIES_NUM_TAG, XCOORD_TAG, YCOORD_TAG }; // tags for distributing parameters from root to all processes

// The static parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

	/* transfering data from root to all processes, in a tree - each receiving process distibutes to 2 other processes */
	MPI_Status status;
	int i;
	if(myRank == 0) {
		for(i = 1; i <= 2; i++)
			if(numProcs > 2*myRank+i) {
				// using point to point communication as requested in the hw
				MPI_Ssend(&citiesNum,1,MPI_INT,i,CITIES_NUM_TAG,MPI_COMM_WORLD);
				MPI_Ssend(xCoord,citiesNum,MPI_INT,i,XCOORD_TAG,MPI_COMM_WORLD);
				MPI_Ssend(yCoord,citiesNum,MPI_INT,i,YCOORD_TAG,MPI_COMM_WORLD);
			}
	}
	else {
		// using point to point communication as requested in the hw
		MPI_Recv(&citiesNum,1,MPI_INT,(myRank-1)/2,CITIES_NUM_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(xCoord,citiesNum,MPI_INT,(myRank-1)/2,XCOORD_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(yCoord,citiesNum,MPI_INT,(myRank-1)/2,YCOORD_TAG, MPI_COMM_WORLD, &status);
		for(i = 1; i <= 2; i++)
			if(numProcs > 2*myRank+i) {
				// using point to point communication as requested in the hw
				MPI_Ssend(&citiesNum,1,MPI_INT,2*myRank+i,CITIES_NUM_TAG,MPI_COMM_WORLD);
				MPI_Ssend(xCoord,citiesNum,MPI_INT,2*myRank+i,XCOORD_TAG,MPI_COMM_WORLD);
				MPI_Ssend(yCoord,citiesNum,MPI_INT,2*myRank+i,YCOORD_TAG,MPI_COMM_WORLD);
			}
	}

	/* wait for all prcoesses to receive the data before continuing execution */
	MPI_Barrier(MPI_COMM_WORLD);

	/* initializations */
	globalCitiesNum = citiesNum;
	globalxCoord = xCoord;
	globalyCoord = yCoord;
	calcDists();
	int minEdges[citiesNum];
	minNextEdgesWeight = minEdges;
	calcMinEdges();
	/*******/

	/* for low citiesNum we will use only root process because the communication overhead might make it slower than serial */
	if(citiesNum < 6) {
		if(myRank > 0)
			return INT_MAX;
		int prefix[citiesNum];
		prefix[0] = 0;
		int bestPath[citiesNum];
		int minWeight = solve(prefix, 1, 0, bestPath);
		memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
		return minWeight;
	}


	// only citiesNum^2 prcoesses allowed
	if(numProcs > (citiesNum-2) * (citiesNum-1)) {
		numProcs = (citiesNum-2) * (citiesNum-1);
		if(myRank >= numProcs)
			return INT_MAX;
	}

	/* compute best path at each process */

	int size;
	// returns path prefixes that the process have to compute, according to it's rank and total num of processes
	int** prefixes = initialPrefixes(citiesNum, &size);
	int minWeight = INT_MAX; // stores the weight of the best path found until some point
	int bestPath[citiesNum]; // stores best path of all paths found until some point
	int path[citiesNum]; // stores the best path with one of the prefixes
	for(i = 0; i < size; ++i) {
		// weight of the prefix
		int ww = dists[prefixes[i][0]][prefixes[i][1]] + dists[prefixes[i][1]][prefixes[i][2]];
		int weight = solve(prefixes[i], PREFIX_LENGTH, ww, path);
		free(prefixes[i]);
		if(weight < minWeight) {
			minWeight = weight;
			memcpy(bestPath,path,citiesNum*sizeof(int));
		}
	}
	free(prefixes);
	for(i = 0; i < citiesNum; ++i)
		free(dists[i]);
	free(dists);

	/* collecting data from all processes into root process (0) */
	int* weights;
	int* paths;
	if(myRank == root) {
		weights = malloc(numProcs * sizeof(int));
		paths = malloc(numProcs * citiesNum * sizeof(int));
	}

	// root gathers all the data from other processes to compute final solution
	// using collective communications as was requested in hw, and process 0 computes the solution, as was requested too
	MPI_Gather(&minWeight, 1, MPI_INT, weights, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Gather(bestPath, citiesNum, MPI_INT, paths, citiesNum, MPI_INT, root, MPI_COMM_WORLD);
	if(myRank == root) {
		// find best path of all paths received
		int best = 0;
		for(i = 0; i < numProcs; ++i)
			if(weights[i] < minWeight) {
				minWeight = weights[i];
				best = i;
			}
		// copy the best path to output
		memcpy(shortestPath, paths + best*citiesNum, citiesNum * sizeof(int));
		free(weights);
		free(paths);
	}

	return minWeight;
}

// returns all path prefixes the process have to solve, according to it's rank and number of processes
int** initialPrefixes(int citiesNum, int* size) {
	int totalPrefixes = (citiesNum - 1) * (citiesNum - 2);
	int regularCount = totalPrefixes / numProcs; // the remainder is given to the #remainder first processes
	*size = myRank < totalPrefixes % numProcs ? regularCount + 1 : regularCount;
	int** prefixes = malloc((*size) * sizeof(int*));
	int firstIndex = myRank * regularCount + (myRank < totalPrefixes % numProcs ? myRank : totalPrefixes % numProcs);
	int i,j;
	for(j = 0, i = firstIndex; i < firstIndex + (*size); ++i, ++j) {
		prefixes[j] = malloc(PREFIX_LENGTH * sizeof(int));
		prefixes[j][0] = 0;	// all prefixes start at city 0
		prefixes[j][1] = 1 + i / (citiesNum - 2);	// according to the number of branch in the tree of prefixes
		prefixes[j][2] = 1 + i % (citiesNum - 3);	// according to the number of branch in the tree of prefixes
		if(prefixes[j][1] <= prefixes[j][2])
			++prefixes[j][2];
	}
	return prefixes;
}

int weightOf(int i, int j) {
	return abs(globalxCoord[i] - globalxCoord[j]) + abs(globalyCoord[i] - globalyCoord[j]);
}

int abs(int n) {
	return n >= 0 ? n : -n;
}

// calculates the distances between each 2 cities, into dists matrix
void calcDists() {
	int i, j;
	dists = (int**) malloc(globalCitiesNum * sizeof(int*));
	for(i = 0; i < globalCitiesNum; ++i) {
		dists[i] = (int*) malloc(globalCitiesNum * sizeof(int));
		for(j = 0; j < globalCitiesNum; j++)
			dists[i][j] = weightOf(i,j);
	}
}

// calculates minNextEdgesWeight array, such that minNextEdgesWeight[i] is the sum of i lowest edge weights
void calcMinEdges() {
	int i, j;
	minNextEdgesWeight[0] = 0;
	int curMaxInd = 0;
	int curMax = 0;		// max in the minNextEdgesWeight array. will be replaced when finding lower weight
	for(i = 1; i < globalCitiesNum; ++i)
		minNextEdgesWeight[i] = dists[0][i];
	curMax = getMax(minNextEdgesWeight, &curMaxInd, globalCitiesNum);
	for(i = 1; i < globalCitiesNum; ++i)
		for(j = i + 1; j < globalCitiesNum; ++j) {
			int w = dists[i][j];
			if(w < curMax) {
				minNextEdgesWeight[curMaxInd] = w;
				curMax = getMax(minNextEdgesWeight, &curMaxInd, globalCitiesNum);
			}
		}
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

// sorts the array
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

// return the best hamilton path and it's weight, such that it has some prefix of cities.
// initialWeight is the weight of the prefix
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

// solves the hamilton path of minimum weight in recursion
// curInd is the index of last city in the path until now
// used is a bit array specifying which cities are already in the path
int recurseSolve(int curInd, int curWeight, int* path, int* used, int* bestPath) {
	if(curInd == globalCitiesNum - 1) {
		//adding edge from last city to first city to close the cycle
		memcpy(bestPath, path, globalCitiesNum * sizeof(int));
		return curWeight + dists[path[0]][path[globalCitiesNum - 1]];
	}

	int bestWeight = INT_MAX;
	int receivedPath[globalCitiesNum];
	int i;
	for(i = 0; i < globalCitiesNum; ++i) {
		if(used[i] == TRUE)
			continue;
		// check that the minimum weight that the path would have is not greater than minimum weight found till now.
		// we check that the weight of the path until now including the next city,
		//		plus the minimum weight that the left edges would have is lower than minimum weight till now
		if(curWeight + dists[path[curInd]][i] + minNextEdgesWeight[globalCitiesNum - curInd - 1] >= bestWeight)
			continue;
		int ww = curWeight + dists[path[curInd]][i];
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