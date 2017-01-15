#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LEVEL 11
#define TERMINATE -10

typedef struct {
	int level;
	int beforeMe;
} sendJob;

// if level == terminate -> termination job
// when sending level is the level of worker
//when feedback level is the minimum weight

void build_derived_type_send(sendJob* data, MPI_Datatype* message_type_ptr){
	int blocks_length [2];
	MPI_Aint displacemets[2];
	MPI_Datatype typelist[2];
	MPI_Aint addresses [3];
	
	typelist[0]=MPI_INT;
	typelist[1]=MPI_INT;
	
	blocks_length[0]=1;
	blocks_length[1]=1;
	
	MPI_Get_address(data,&addresses[0]);
	MPI_Get_address(&(data->level),&addresses[1]);
	MPI_Get_address(&(data->beforeMe),&addresses[2]);
	displacemets[0] = addresses[1]-addresses[0];
	displacemets[1] = addresses[2]-addresses[1];
	MPI_Type_create_struct(2,blocks_length,displacemets,typelist,message_type_ptr);
	MPI_Type_commit(message_type_ptr);
}


int facto(int max,int n){
	int returnMe=1;
	for(int i=max;i>(max-n);i--){
		returnMe*=i;
	}
	return returnMe;
}

//messegase with tag 0 is the messages between worker and master
//messegase with tag 1 is the messages between the workers


int abs(int x){
	if(x<0)
		return -x;
	return x;
}

void matrixCreate(int* xCoord,int* yCoord,int** Matrix,int n){//the matrix is always nXn
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if(i==j){
				Matrix[i][j]=-1;
			}else{
				Matrix[i][j]=(abs(xCoord[i]-xCoord[j])+abs(yCoord[i]-yCoord[j]));
			}
		}
	}
}

/*
*return the next city and fill traveldCities accordingly
*/
int nextCity(int* traveldCities,int zeroNumber,int size){
	int place=0;
	for(int i=0;i<size;i++){
		if(traveldCities[i]==0){
			zeroNumber--;
		}
		if(zeroNumber==0){
			place =i;
			break;
		}
	}
	traveldCities[place]=1;
	return place;
}

void copyPathes(int* dst,int* src,int size){
	for(int i=0;i<size;i++){
		dst[i]=src[i];
	}
}

//should implement it from advancing point and down
void nextPermutation(int* rout,int advancingPoint,int max){
	int carry=1;
	for(int i=advancingPoint;i>=0;i--){
		if((rout[i]==(max-i))&&(carry==1)){
			rout[i]=1;
			carry=1;
			continue;
		}
		if(carry==1){
			rout[i]++;
			carry=0;
		}
	}
}

void clearRestPath (int* rout,int advancingPoint,int max){
	for(int i=advancingPoint+1;i<max;i++){
		rout[i]=1;
	}
}

//clear traveldCities, tempShortestPath and weight
void clearPath(int* rout,int level,int* traveldCities,int* tempShortestPath,int size){
	/*for(int i=0;i<size;i++){
		traveldCities[i]=0;
		tempShortestPath[i]=0;
	}*/
	memset(traveldCities,0,size*sizeof(int));
	memset(tempShortestPath,0,size*sizeof(int));
	for(int i=0;i<=level;i++){
		int temp=rout[i];
		int j=0;
		while(temp>0){
			if(traveldCities[j]==0){
				temp--;
			}
			j++;
		}
		traveldCities[j-1]=1;
		tempShortestPath[i]=j-1;
	}
	
	
}

//the NC is in level = iterationNum
int recLightPath(int citiesNum,int** matrix,int* rout,int shortestPath[],int* tempShort,
						int iterationNum, int* weight,int* minWeight,int* traveldCities,int currentCity){
	
	int flag=0; // check the last premutation of the iteration
	while((rout[iterationNum]<=citiesNum-iterationNum || iterationNum==citiesNum-1)&& iterationNum!=citiesNum &&rout[0]==1 && flag!=1){
		if(rout[iterationNum]==citiesNum-iterationNum ){
			flag=1;
		}
		int NC = nextCity(traveldCities,rout[iterationNum],citiesNum);
		(*weight)+=matrix[currentCity][NC];
		tempShort[iterationNum]=NC;
		if(iterationNum==citiesNum-1){
			(*weight)+=matrix[NC][0];
		}
		
		if(((*minWeight)==-1 || (*weight)<(*minWeight) )&& iterationNum==citiesNum-1){
			(*minWeight)=(*weight);
			copyPathes(shortestPath,tempShort,citiesNum);
			traveldCities[NC]=0;
			(*weight)-=matrix[currentCity][NC];
			(*weight)-=matrix[NC][0];
			return 0;
		}
		
		//the adding is too much
		if(((*weight))+matrix[NC][0]>=(*minWeight) && (*minWeight)!=-1){
			traveldCities[NC]=0;
			(*weight)-=matrix[currentCity][NC];
			if(iterationNum==citiesNum-1){
				(*weight)-=matrix[NC][0];
			}
			return 0;
		}
		if(recLightPath(citiesNum,matrix,rout,shortestPath,tempShort,iterationNum+1,weight,minWeight,traveldCities,NC)==0){ // son did next premutation for you
			nextPermutation(rout,iterationNum,citiesNum);
			clearRestPath(rout,iterationNum,citiesNum);
		}	
		traveldCities[NC]=0;
		(*weight)-=matrix[currentCity][NC];
		
		/*
		if((*weight)<(*minWeight) || (*minWeight)==-1){
			int temp = rout[iterationNum];
			recLightPath(citiesNum,matrix,rout,shortestPath,tempShort,iterationNum+1,weight,minWeight,traveldCities,NC);
			traveldCities[NC]=0;			
			(*weight)-=matrix[currentCity][NC];
			
			if(rout[iterationNum]==citiesNum-iterationNum){
				nextPermutation(rout,iterationNum+1,citiesNum);
				return 0;
			}
			if(temp != rout[iterationNum]){
				continue;
			}
			nextPermutation(rout,iterationNum+1,citiesNum);
			continue;
		}
		else{
			(*weight)-=matrix[currentCity][NC];
			traveldCities[NC]=0;
			if(iterationNum==citiesNum-1){
				(*weight)-=matrix[NC][0];
			}
			return 0;
		}
		
		if(iterationNum==citiesNum-1){
			(*weight)-=matrix[currentCity][NC];
			traveldCities[NC]=0;
			(*weight)-=matrix[NC][0];
			return 0;
		}
		if(rout[iterationNum] == citiesNum-iterationNum){
			break;
		}
		nextPermutation(rout,iterationNum,citiesNum);*/
	}
	return 1;

}

int lightPath(int citiesNum,int** matrix,int* rout,int level,int shortestPath[],int weight){
	/*
	initiate
	recLightPath
	*/
	int minWeight=weight;
	int startWeight=0;
	int currentCity=0;
	/*initiate all from  rout*/
	int* traveldCities = malloc(sizeof(int)*citiesNum);
	int* tempShort = malloc(sizeof(int)*citiesNum);
	for(int i=0;i<citiesNum;i++){
		traveldCities[i]=0;
	}
	for(int i=0;i<=level;i++){
		int temp=rout[i];
		int j=0;
		while(temp>0){
			if(traveldCities[j]==0){
				temp--;
			}
			j++;
		}
		traveldCities[j-1]=1;
		if(matrix[currentCity][j-1]!=-1){
			startWeight+=matrix[currentCity][j-1];
		}
		currentCity=j-1;
		shortestPath[i]=j-1;
		tempShort[i]=j-1;
	}
	/*initiate done*/

	recLightPath(citiesNum,matrix,rout,shortestPath,tempShort,level+1,&startWeight,&minWeight,traveldCities,currentCity);
	free(traveldCities);
	free(tempShort);
	return minWeight;
}


// The dynamic parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	int myrank, numberOfProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcs);
	int global_min;
	
	int* buf = malloc(sizeof(char)*5*(1<<20));
	
	int** matrix = malloc(sizeof(int*)*citiesNum);
	for(int i=0;i<citiesNum;i++){
		matrix[i]=malloc(sizeof(int)*citiesNum);
	}
	matrixCreate(xCoord,yCoord,matrix,citiesNum);
	
	sendJob job;
	MPI_Datatype job_type;
	build_derived_type_send(&job,&job_type);
	sendJob* job_send = &job;
	if(numberOfProcs == 1){
		return -2;
	}
	
	MPI_Status status, status2;
    MPI_Request request; 	
	MPI_Buffer_attach(buf,5*(1<<20)); // so there will be enogh memory not to crush
	
	//master code
	if(myrank==0){
		/*
		calculate the level for worker
			wait for worker to ask for a job
				give job
				advance the rout
			give terminating job
		*/
		
		global_min = -1;
		int level=citiesNum-MAX_LEVEL;
		if(level<0){
			level=0;
		}
		int COUNTER_MAX=facto(citiesNum,level);
		int counter=0;
		int* tempShortPath = malloc(sizeof(int)*(citiesNum+1));
		job_send->level=level;
		job_send->beforeMe=0;
		
		for(int i=1;(i<numberOfProcs && counter<COUNTER_MAX);i++){
            MPI_Bsend(&job, 1, job_type, i, 0, MPI_COMM_WORLD);
			counter++;
			job_send->beforeMe=counter;
		}
		
		int flag=0;
		int done_procs=0;
		while(done_procs<numberOfProcs-1){
			MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);
			if(flag){
				MPI_Recv(tempShortPath, citiesNum+1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status2);
				
				if(tempShortPath[citiesNum]< global_min || global_min==-1){
					global_min = tempShortPath[citiesNum];
					copyPathes(shortestPath,tempShortPath,citiesNum);
				}
				
				if(counter<COUNTER_MAX){
					MPI_Bsend(&job, 1, job_type, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
					counter++;
					job_send->beforeMe=counter;
				}
				else{
					job_send->level = TERMINATE;
					MPI_Bsend(&job, 1, job_type, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
					done_procs++;
				}
				flag=0;
			}
		}
	}
	
	//worker code
	
	else{
		/*
		ask the master for a job
		check terminate -> terminate
		execute the job
		check for new borodcast minimum
			if broadcast minimum< current global minimum -> change minimum
		check the minimum of your job
		send back feedback if needed and notidy all processes about the minimum
		*/
		int* rout = malloc(sizeof(int)*citiesNum);
		int* shortPath = malloc(sizeof(int)*(citiesNum+1));
		for(int i=0;i<citiesNum;i++){
			rout[i]=1;
		}
		
		int myMinWeight=-1;
		int myplace=0;
		int tempMin;
		int flag=1;
		int prev;
		int premutationFlag=0;
		
		while(1){
			MPI_Recv(&job, 1, job_type, 0, 0, MPI_COMM_WORLD, &status);
			for(int i=0;i<(job.beforeMe-myplace-premutationFlag);i++){
				nextPermutation(rout,job.level,citiesNum);
			}
			premutationFlag=0;
			myplace=job.beforeMe;

			if (job.level == TERMINATE) {
                break;
            }
			flag=1;
			while(flag==1){
				flag=0;
				MPI_Iprobe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &flag, &status);
				if(flag){
					MPI_Recv(&tempMin, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD, &status2);
					if(tempMin<myMinWeight){
						myMinWeight=tempMin;
					}
				}
			}


			prev= rout[job.level];
			shortPath[citiesNum] = lightPath(citiesNum,matrix,rout,job.level,shortPath,myMinWeight);
			clearRestPath(rout,job.level,citiesNum);
			if(prev != rout[job.level]){
				premutationFlag=1;
			}
			if(shortPath[citiesNum]<myMinWeight || myMinWeight==-1){
				myMinWeight = shortPath[citiesNum];
				for(int i=1;i<numberOfProcs;i++){
					if(i==myrank){
						continue;
					}
					MPI_Bsend(&myMinWeight, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
				}
			}		
			MPI_Bsend(shortPath, citiesNum+1, MPI_INT, 0, 0, MPI_COMM_WORLD);			
		}
	}
	void* ptr;
	int pt;
	MPI_Buffer_detach(&ptr,&pt);
	MPI_Barrier(MPI_COMM_WORLD);
	return global_min;	//TODO
}
