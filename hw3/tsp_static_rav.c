#include <mpi.h>
/*
@author - Raviv Rachmiel
@since - 07/01/2017
*/

//normal absolute function as implemented in math.h
int abs(int a, int b) {
  return a-b>0? a-b : b-a;
}

//gets 2 cities and coordinates and citiesNum
//returns the manhatten distance
int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum) {
  return city1==city2? 0 : (abs(xCoord[i]-xCoord[j])+abs(yCoord[i]-yCoord[j]));
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
		for(int i=1;i<numberOfProcs;i++){
			MPI_Bsend(&citiesNum,1,MPI_INT,i,0,MPI_COMM_WORLD);
			MPI_Bsend(xCoord,citiesNum,MPI_INT,i,1,MPI_COMM_WORLD);
			MPI_Bsend(yCoord,citiesNum,MPI_INT,i,2,MPI_COMM_WORLD);
		}
	}
	else{
		MPI_Recv(&citiesNum,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		xCoord=malloc(sizeof(int)*localCitiesNum);
		yCoord=malloc(sizeof(int)*localCitiesNum);
		MPI_Recv(xCoord,citiesNum,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		MPI_Recv(yCoord,citiesNum,MPI_INT,0,2,MPI_COMM_WORLD,&status);
	}
	MPI_Barrier(MPI_COMM_WORLD);
  int* path = (int*) malloc(sizeof(int)*(citiesNum));

  //using serial algorithm
  if(citiesNum < 6) {
    //for each process other then "master"
    if(myRank > 0)
      return INT_MAX;
    int prefix[citiesNum];
    prefix[0] = 0;
    int bestPath[citiesNum];
    int minWeight = solve(prefix, 1, 0, bestPath);
    memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
    return minWeight;
  }

  return -1;
}
