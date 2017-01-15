#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
@author - Raviv Rachmiel
@since - 07/01/2017
*/

#define MAX_PATH 10000000
#define SERIAL_VAR 15
//====TODO: Doron, HARD refactor this
#define PREFIX_LENGTH 3
const int root = 0;
//====end


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


//========== TODO: DORON, refactor this hard
// returns all path prefixes the process have to solve, according to it's rank and number of processes
int** initialPrefixes(int mRank,int numOfProcs,int citiesNum, int* size) {
	int totalPrefixes = (citiesNum - 1) * (citiesNum - 2);
	int regularCount = totalPrefixes / numOfProcs; // the remainder is given to the #remainder first processes
	*size = mRank < totalPrefixes % numOfProcs ? regularCount + 1 : regularCount;
	int** prefixes = malloc((*size) * sizeof(int*));
	int firstIndex = mRank * regularCount + (mRank < totalPrefixes % numOfProcs ? mRank : totalPrefixes % numOfProcs);
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
//============= END


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
	//============TODO: Doron, refactor HARD from here:
	/* compute best path at each process */

	int size;
	// returns path prefixes that the process have to compute, according to it's rank and total num of processes
	int** prefixes = initialPrefixes(mRank,numOfProcs,citiesNum, &size);
	int minWeight = MAX_PATH; // stores the weight of the best path found until some point
	int bestPath[citiesNum]; // stores best path of all paths found until some point
	int path[citiesNum]; // stores the best path with one of the prefixes
	for(int i = 0; i < size; ++i) {
		// weight of the prefix
		int ww = getDist(prefixes[i][0],prefixes[i][1],xCoord,yCoord,citiesNum) + getDist(i,j,xCoord,yCoord,citiesNum);
		int weight = find(prefixes[i], PREFIX_LENGTH, ww, path,xCoord,yCoord,citiesNum);
		free(prefixes[i]);
		if(weight < minWeight) {
			minWeight = weight;
			memcpy(bestPath,path,citiesNum*sizeof(int));
		}
	}
	free(prefixes);

	/* collecting data from all processes into root process (0) */
	int* weights;
	int* paths;
	if(mRank == root) {
		weights = malloc(numOfProcs * sizeof(int));
		paths = malloc(numOfProcs * citiesNum * sizeof(int));
	}

	// root gathers all the data from other processes to compute final solution
	// using collective communications as was requested in hw, and process 0 computes the solution, as was requested too
	MPI_Gather(&minWeight, 1, MPI_INT, weights, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Gather(bestPath, citiesNum, MPI_INT, paths, citiesNum, MPI_INT, root, MPI_COMM_WORLD);
	if(mRank == root) {
		// find best path of all paths received
		int best = 0;
		for(i = 0; i < numOfProcs; ++i)
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
	//=====================END
}
