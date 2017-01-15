#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
@author - Raviv Rachmiel
@since - 07/01/2017
*/

#define MAX_PATH 10000000

//normal ABSolute function as implemented in math.h
int ABS(int a) {
  return a>0? a : a*(-1);
}

//gets 2 cities and coordinates and citiesNum
//returns the manhatten distance
int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum) {
  return city1==city2? 0 : (ABS(xCoord[city1]-xCoord[city2])+ABS(yCoord[city1]-yCoord[city2]));
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

  //using serial algorithm
  if(citiesNum < 6) {
    //for each process other then "master"
    if(mRank > 0)
      return MAX_PATH;
    int prefix[citiesNum];
    prefix[0] = 0;
    int bestPath[citiesNum];
    int minWeight = solve(prefix, 1, 0, bestPath);
    memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
    return minWeight;
  }

  return -1;
}
