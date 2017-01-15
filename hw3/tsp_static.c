#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
@author - Raviv Rachmiel
@since - 07/01/2017
*/

#define MAX_PATH 10000000
#define SERIAL_VAR 7

int* minNowArr;
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

/*
@param arr - the array
@param len - the length
@param index - a pointer to the index of the maximum value in array
@return the max value, also update the index pointer to the index with max val
*/
int getMax(int* arr, int len, int* index) {
	*index = 0;
	for(int i = 0; i < len; i++)
		if(arr[i] > arr[*index])
			*index = i;
	return arr[*index];
}

/* simple min sort
@param arr - the array
@param len - the length
changes the array to be a sorted array from min value to max
*/
void sort(int* arr, int len) {
	int temp = 0;
	int minIndex = 0;
	for(int i  = 0; i < len; i++) {
		for(int j = i + 1; j < len; j++)
			if(arr[j] < arr[minIndex])
				minIndex = j;
		temp = arr[i];
		arr[i] = arr[minIndex];
		arr[minIndex] = temp;
	}
}

/*
@param coordinates and citiesNum
@returns an array which for each city, holds the min Dist
very very greedy
//TODO: Doron, refactor this please
*/
void calcMinEdges(int* xCoord,int* yCoord,int citiesNum) {
	int i, j;
	minNowArr[0] = 0;
	int curMaxInd = 0;
	int curMax = 0;		// max in the minNowArr array. will be replaced when finding lower weight
	for(i = 1; i < citiesNum; ++i)
		minNowArr[i] = getDist(0,i,xCoord,yCoord,citiesNum);
	printf("MUST\n");
	curMax = getMax(minNowArr, citiesNum, &curMaxInd);
	printf("BE\n");
	for(i = 1; i < citiesNum; ++i)
		for(j = i + 1; j < citiesNum; ++j) {
			int w = getDist(i,j,xCoord,yCoord,citiesNum);
			if(w < curMax) {
				minNowArr[curMaxInd] = w;
				curMax = getMax(minNowArr, citiesNum, &curMaxInd);
			}
		}
		printf("AN\n");
	sort(minNowArr, citiesNum);
	printf("ANGEL\n");
	for(i = 2; i < citiesNum; ++i)
		minNowArr[i] += minNowArr[i - 1];
}


void calcMinEdges2(int* xCoord,int* yCoord,int citiesNum) {
	int i, j;
	minNextEdgesWeight[0] = 0;
	int curMaxInd = 0;
	int curMax = 0;		// max in the minNextEdgesWeight array. will be replaced when finding lower weight
	for(i = 1; i < citiesNum; ++i)
		minNextEdgesWeight[i] = getDist(0,i,xCoord,yCoord,citiesNum);
	curMax = getMax(minNextEdgesWeight, &curMaxInd, citiesNum);
	for(i = 1; i < citiesNum; ++i)
		for(j = i + 1; j < citiesNum; ++j) {
			int w = getDist(i,j,xCoord,yCoord,citiesNum);
			if(w < curMax) {
				minNextEdgesWeight[curMaxInd] = w;
				curMax = getMax(minNextEdgesWeight, &curMaxInd, citiesNum);
			}
		}
	sort(minNextEdgesWeight, citiesNum);
	for(i = 2; i < globalCitiesNum; ++i)
		minNextEdgesWeight[i] += minNextEdgesWeight[i - 1];
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
	printf("BEEN\n");
	if(current == citiesNum - 1) {
		printf("TRYING\n");
		memcpy(bestPath, path, citiesNum * sizeof(int));
		return curWeight + getDist(path[0],path[citiesNum - 1],xCoord,yCoord,citiesNum);
	}
	int bestWeight = MAX_PATH;
	int receivedPath[citiesNum];
	for(int i = 0; i < citiesNum; ++i) {
		if(used[i] == 1)
			continue;
		// check that the minimum weight that the path would have is not greater than minimum weight found till now.
		// we check that the weight of the path until now including the next city,
		//		plus the minimum weight that the left edges would have is lower than minimum weight till now
		if(curWeight + getDist(path[current],i,xCoord,yCoord,citiesNum) + minNextEdgesWeight[citiesNum - current - 1] >= bestWeight)
			continue;
		printf("TO\n");
		int ww = curWeight + getDist(path[current],i,xCoord,yCoord,citiesNum);
		path[current + 1] = i;
		used[i] = 1;
		printf("MEET\n");
		int weight = findRec(current + 1, ww, path, used, receivedPath,xCoord,yCoord,citiesNum);
		used[i] = 0;
		printf("YOU\n");
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
  printf("HEY\n");
	int used[citiesNum];
  int path[citiesNum];
	memset(used, 0, citiesNum * sizeof(int));
	memcpy(path, prefix, len * sizeof(int));
	for(int i = 0; i < len; ++i)
		used[prefix[i]] = 1;
	return findRec(len - 1, initialWeight, path, used, bestPath,xCoord, yCoord,citiesNum);
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
	printf("TAM TAM TAM\n");
	int min[citiesNum];
	minNowArr = min;
	minNextEdgesWeight = min;
	calcMinEdges2(xCoord,yCoord, citiesNum);

  //using serial algorithm
  if(citiesNum < SERIAL_VAR) {
    //for each process other then "master"
    if(mRank > 0)
      return MAX_PATH;
    int prefix[citiesNum];
    prefix[0] = 0;
    int bestPath[citiesNum];
    int minWeight = find(prefix, 1, 0, bestPath,xCoord,yCoord,citiesNum);
    memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
    return minWeight;
  }

  return -1;
}
