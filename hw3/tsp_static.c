#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
@author - Raviv Rachmiel
@since - 07/01/2017
*/

#define MAX_PATH 10000000

int* minNextEdgesWeight;

//normal ABSolute function as implemented in math.h
int ABS(int a) {
  return a>0? a : a*(-1);
}

//gets 2 cities and coordinates and citiesNum
//returns the manhatten distance
int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum) {
  return city1==city2? 0 : (ABS(xCoord[city1]-xCoord[city2])+ABS(yCoord[city1]-yCoord[city2]));
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

// calculates minNextEdgesWeight array, such that minNextEdgesWeight[i] is the sum of i lowest edge weights
void calcMinEdges(int* xCoord,int* yCoord,int citiesNum) {
	int i, j;
	minNextEdgesWeight[0] = 0;
	int curMaxInd = 0;
	int curMax = 0;		// max in the minNextEdgesWeight array. will be replaced when finding lower weight
	for(i = 1; i < citiesNum; ++i)
		minNextEdgesWeight[i] = getDist(0,i,xCoord,yCoord,citiesNum);
	curMax = getMax(minNextEdgesWeight, &curMaxInd, citiesNum);
	for(i = 1; i < citiesNum; ++i)
		for(j = i + 1; j < citiesNum; ++j) {
			int w = dists[i][j];
			if(w < curMax) {
				minNextEdgesWeight[curMaxInd] = w;
				curMax = getMax(minNextEdgesWeight, &curMaxInd, citiesNum);
			}
		}
	sort(minNextEdgesWeight, citiesNum);
	for(i = 2; i < citiesNum; ++i)
		minNextEdgesWeight[i] += minNextEdgesWeight[i - 1];
}




// solves the hamilton path of minimum weight in recursion
// curInd is the index of last city in the path until now
// used is a bit array specifying which cities are already in the path
int recurseSolve(int curInd, int curWeight, int* path, int* used, int* bestPath,int* xCoord,int* yCoord,int citiesNum) {
	if(curInd == citiesNum - 1) {
		//adding edge from last city to first city to close the cycle
		memcpy(bestPath, path, citiesNum * sizeof(int));
		return curWeight + dists[path[0]][path[citiesNum - 1]];
	}

	int bestWeight = MAX_PATH;
	int receivedPath[citiesNum];
	int i;
	for(i = 0; i < citiesNum; ++i) {
		if(used[i] == 1)
			continue;
		// check that the minimum weight that the path would have is not greater than minimum weight found till now.
		// we check that the weight of the path until now including the next city,
		//		plus the minimum weight that the left edges would have is lower than minimum weight till now
		if(curWeight + dists[path[curInd]][i] + minNextEdgesWeight[citiesNum - curInd - 1] >= bestWeight)
			continue;
		int ww = curWeight + getDist(path[curInd],i,xCoord,yCoord,citiesNum);
		path[curInd + 1] = i;
		used[i] = 1;
		int weight = recurseSolve(curInd + 1, ww, path, used, receivedPath,xCoord,yCoord,citiesNum);
		used[i] = 0;
		if(weight < bestWeight) {
			bestWeight = weight;
			memcpy(bestPath, receivedPath, citiesNum * sizeof(int));
		}
	}
	return bestWeight;
}





// return the best hamilton path and it's weight, such that it has some prefix of cities.
// initialWeight is the weight of the prefix
int solve(int prefix[], int len, int initialWeight, int* bestPath,int* xCoord,int* yCoord,int citiesNum) {
	int used[citiesNum];
	memset(used, 0, citiesNum * sizeof(int));
	int path[citiesNum];
	memcpy(path, prefix, len * sizeof(int));
	for(int i = 0; i < len; ++i)
		used[prefix[i]] = 1;
	return recurseSolve(len - 1, initialWeight, path, used, bestPath,xCoord, yCoord,citiesNum);
}

// The static parellel algorithm main function.
/*
gets the cities num and coordinates
returns the shortestPath
*/
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[]) {
  int mRank, numOfProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &mRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numOfProcs);
	MPI_Status status;

  if (mRank==0){
		//"master" part - send variables to other processes
		/*sending*/
		for(int i=1;i<numOfProcs;i++){
			MPI_Bsend(&citiesNum,1,MPI_INT,i,0,MPI_COMM_WORLD);
			MPI_Bsend(xCoord,citiesNum,MPI_INT,i,1,MPI_COMM_WORLD);
			MPI_Bsend(yCoord,citiesNum,MPI_INT,i,2,MPI_COMM_WORLD);
		}
	}
	else{
		MPI_Recv(&citiesNum,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		// xCoord=malloc(sizeof(int)*citiesNum);
		// yCoord=malloc(sizeof(int)*citiesNum);
		MPI_Recv(xCoord,citiesNum,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		MPI_Recv(yCoord,citiesNum,MPI_INT,0,2,MPI_COMM_WORLD,&status);
	}
	MPI_Barrier(MPI_COMM_WORLD);
  //int* path = (int*) malloc(sizeof(int)*(citiesNum));


  int minEdges[citiesNum];
	minNextEdgesWeight = minEdges;
	calcMinEdges(xCoord,yCoord, citiesNum);

  //using serial algorithm
  if(citiesNum < 6) {
    //for each process other then "master"
    if(mRank > 0)
      return MAX_PATH;
    int prefix[citiesNum];
    prefix[0] = 0;
    int bestPath[citiesNum];
    int minWeight = solve(prefix, 1, 0, bestPath,xCoord,yCoord,citiesNum);
    memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
    return minWeight;
  }

  return -1;
}
