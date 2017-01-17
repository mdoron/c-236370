#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_PATH 10000000
#define SERIAL_VAR 7
#define PREF_SIZE 3
#define TRUE 1
#define FALSE 0
#define LISTEN while(TRUE)

typedef struct job_t {
	int prefix[PREF_SIZE];
	char is_done;
} Job;

int *global_x, *global_y;
int global_cities_num, procs_num, my_rank, prev_bound, local_bound;
Job* job;

enum Tag { ASK_FOR_JOB, REPORT, NOTHING_TO_REPORT, REPORT_WEIGHT, REPORT_PATH, NEW_JOB, NEW_BOUND };

void build(Job* data, MPI_Datatype* message_type_ptr) {
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
	MPI_Get_address(&(data->prefix), &addresses[1]);
	MPI_Get_address(&(data->is_done), &addresses[2]);

	displacements[0] = addresses[1] - addresses[0];
	displacements[1] = addresses[2] - addresses[0];

	// Create the derived type
	MPI_Type_create_struct(2, block_lengths,displacements, typelist, message_type_ptr);

	// Commit it so that it can be used
	MPI_Type_commit(message_type_ptr);
}

int ABS(int a) {
  return a>0 ? a : a*(-1);
}

// The dynamic parellel algorithm main function.

void swap(int* x, int* y) {
	int tmp = *x;
	*x = *y;
	*y = tmp;
}

//keep first node always 0
int next_permut(int* prefix) {
	int i, j;
	for(i = PREF_SIZE-2; i >= 1; --i)
		if(prefix[i] < prefix[i+1])
			break;
	if(i < 1) { 
		return 0;
	}
	for(j = PREF_SIZE-1; j > i; --j)
		if(prefix[j] > prefix[i])
			break;
	swap(prefix+i, prefix+j);
	++i;
	for(j = PREF_SIZE-1; j > i; --j, ++i)
		swap(prefix+i, prefix+j);
	return 1;
}

//gets 2 cities and coordinates and citiesNum
//returns the manhatten distance
int get_dist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum) {
  return city1==city2? 0 : (ABS(xCoord[city1]-xCoord[city2])+ABS(yCoord[city1]-yCoord[city2]));
}

int prefix_weight(int prefix[],int* xCoord, int* yCoord, int citiesNum) {
	int w = 0;
	int i;
	for(i = 0; i < PREF_SIZE - 1;++i)
		w += get_dist(prefix[i],prefix[i+1], xCoord, yCoord, citiesNum);
	return w;
}

void create_job(int prefix[], char is_done) {
	memcpy(job->prefix, prefix, PREF_SIZE * sizeof(int));
	job->is_done = is_done;
}

/*
@param current - the current index we are working on - last one will be the stop
@param path - current path
@param curr_weight - curr path weight
@param min_path - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int find_rec(int current, int curr_weight, int* path, int* used, int* min_path,int* xCoord,int* yCoord,int citiesNum) {
	if(current == citiesNum - 1) {
		memcpy(min_path, path, citiesNum * sizeof(int));
		return curr_weight + get_dist(path[0],path[citiesNum - 1],xCoord,yCoord,citiesNum);
	}
	int min_weight = MAX_PATH;
	int receivedPath[citiesNum];
	for(int i = 0; i < citiesNum; ++i) {
		if(used[i] == 1)
			continue;
		int check_now = curr_weight + get_dist(path[current],i,xCoord,yCoord,citiesNum);
		path[current + 1] = i;
		used[i] = 1;
		int weight = find_rec(current + 1, check_now, path, used, receivedPath,xCoord,yCoord,citiesNum);
		used[i] = 0;
		if(weight < min_weight) {
			min_weight = weight;
			memcpy(min_path, receivedPath, citiesNum * sizeof(int));
		}
	}
	return min_weight;
}



/*
@param prefix - gets prefix of a path
@param len - the prefix length
@param initialWeight - prefix path weight
@param min_path - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int find(int* prefix, int len, int initialWeight, int* min_path,int* xCoord,int* yCoord,int citiesNum) {
	int used[citiesNum];
  int path[citiesNum];
	memset(used, 0, citiesNum * sizeof(int));
	memcpy(path, prefix, len * sizeof(int));
	for(int i = 0; i < len; ++i)
		used[prefix[i]] = 1;
	return find_rec(len - 1, initialWeight, path, used, min_path,xCoord, yCoord,citiesNum);
}


// returns all path prefixes the process have to solve, according to it's rank and number of processes
void fill_prefs(int procs_num,int citiesNum, int r, int size, int firstIndex, int** prefs) {
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

int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortest_path[])
{
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procs_num);
	if(procs_num == 1)
		exit(0);
	int i;
	int min_weight = INT_MAX;
	prev_bound = INT_MAX;
	local_bound = INT_MAX;
	int min_path[citiesNum];
	MPI_Datatype MPI_Job;
	Job t;
	build(&t,&MPI_Job);
	MPI_Status status;
	char info;
	int path[citiesNum];
	int weight;
	
	global_cities_num = citiesNum;
	global_x = xCoord;
	global_y = yCoord;
	local_bound = INT_MAX;
	job = (Job*) malloc(sizeof(Job));

	MPI_Request request;
	
	if(my_rank == 0) {
		int prefix[PREF_SIZE];
		prefix[0] = 0;
		int i=0, j=0, k=0;
		for(i = 1; i < citiesNum; i++) { 
			printf("# %d\n", i);
			for (int j = i; j < citiesNum; j++) {
				if (i == j) 
					continue;
				printf("##### %d\n", j);
				for (int k = j; k < citiesNum; k++) {
					if (j == k || i == k)
						continue;
					printf("########## %d\n", k);
					prefix[1] = i;
					prefix[2] = j;
					prefix[3] = k;

					printf("%d %d %d %d\n", prefix[0], prefix[1], prefix[2], prefix[3]);
					do {
						create_job(prefix, FALSE);
						LISTEN {
							MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
							int source = status.MPI_SOURCE;
							if(status.MPI_TAG == ASK_FOR_JOB) {
								// check if need irecv
								MPI_Recv(&info,1,MPI_CHAR,source,ASK_FOR_JOB, MPI_COMM_WORLD, &status);
								MPI_Issend(job, 1, MPI_Job, source, NEW_JOB, MPI_COMM_WORLD,&request);
								break;
							}
							else if(status.MPI_TAG == REPORT) {
								MPI_Recv(&info,1,MPI_CHAR,source,REPORT, MPI_COMM_WORLD, &status);
								MPI_Recv(&weight,1,MPI_INT,source,REPORT_WEIGHT, MPI_COMM_WORLD, &status);
								MPI_Recv(path,citiesNum,MPI_INT,source,REPORT_PATH, MPI_COMM_WORLD, &status);
								if(weight < min_weight) {
									min_weight = weight;
									memcpy(min_path, path, citiesNum * sizeof(int));
									for(i = 1; i < procs_num; ++i)
										MPI_Issend(&min_weight, 1, MPI_INT, i, NEW_BOUND, MPI_COMM_WORLD, &request);
								}
							}
						} 
					} while(next_permut(prefix));
				}
			}
		}
		// 
		create_job(prefix, TRUE);
		for(i = 1; i < procs_num; ++i)
			MPI_Issend(job, 1, MPI_Job, i, NEW_JOB, MPI_COMM_WORLD, &request);
		int count = 0;
		while(count < procs_num - 1) {
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int source = status.MPI_SOURCE;
			if(status.MPI_TAG == ASK_FOR_JOB) {
				MPI_Recv(&info,1,MPI_CHAR,source,ASK_FOR_JOB, MPI_COMM_WORLD, &status);
				MPI_Issend(job, 1, MPI_Job, source, NEW_JOB, MPI_COMM_WORLD, &request);
				continue;
			}
			else if(status.MPI_TAG == REPORT) {
				MPI_Recv(&info,1,MPI_CHAR,source,REPORT, MPI_COMM_WORLD, &status);
				MPI_Recv(&weight,1,MPI_INT,source,REPORT_WEIGHT, MPI_COMM_WORLD, &status);
				MPI_Recv(path,citiesNum,MPI_INT,source,REPORT_PATH, MPI_COMM_WORLD, &status);
				if(weight < min_weight) {
					min_weight = weight;
					memcpy(min_path, path, citiesNum * sizeof(int));
				}
			}
			else if(status.MPI_TAG == NOTHING_TO_REPORT) {
				MPI_Recv(&info,1,MPI_CHAR,source,NOTHING_TO_REPORT, MPI_COMM_WORLD, &status);
				++count;
			}
		}
		memcpy(shortest_path, min_path, citiesNum * sizeof(int));
	}
	else {
		LISTEN {
			info = ASK_FOR_JOB;
			MPI_Issend(&info,1,MPI_CHAR,0,ASK_FOR_JOB,MPI_COMM_WORLD, &request);
			do {
				MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				if(status.MPI_TAG == NEW_JOB)
					MPI_Recv(job,1,MPI_Job,0,NEW_JOB, MPI_COMM_WORLD, &status);
				else if(status.MPI_TAG == NEW_BOUND) {
					int prevBOund = local_bound;
					MPI_Recv(&local_bound,1,MPI_INT,0,NEW_BOUND, MPI_COMM_WORLD, &status);
					if(prevBOund < local_bound)
						local_bound = prev_bound;
				}
			} while(status.MPI_TAG != NEW_JOB);

			if(job->is_done) {
				MPI_Issend(&info,1,MPI_CHAR,0,NOTHING_TO_REPORT,MPI_COMM_WORLD, &request);
				break;
			}
			int* a = job->prefix;
			int b = prefix_weight(job->prefix, xCoord, yCoord, citiesNum);
			weight = find(a, PREF_SIZE, b, path,xCoord,yCoord,citiesNum);
			if(weight < local_bound) {
				info = REPORT;
				MPI_Ssend(&info,1,MPI_CHAR,0,REPORT,MPI_COMM_WORLD);
				MPI_Ssend(&weight,1,MPI_INT,0,REPORT_WEIGHT,MPI_COMM_WORLD);
				MPI_Ssend(path,citiesNum,MPI_INT,0,REPORT_PATH,MPI_COMM_WORLD);
			}
		}
		
	}
	free(job);

	return min_weight;
}