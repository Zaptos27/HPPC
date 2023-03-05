#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <random>
#include <chrono>
#include <thread>
#include <array>
#include <vector>
#include <mpi.h>

const int MAX_TASKS=5000;  // number of tasks
const int NO_MORE_TASKS = MAX_TASKS+1;
const int RANDOM_SEED=1234;

void master(int workers);
void worker(int rank);

int main(int argc, char *argv[])
{
  int np, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  if (rank == 0)
  {
    master(np - 1);
  }
  else
  {
    worker(rank);
  }
  MPI_Finalize();
}

void master(int workers)
{
  int i, tag, who;
    MPI_Status status;
    std::array<int, MAX_TASKS> temp, task, result;
    for (int j=0; j<MAX_TASKS;j++){
        task[j] = j;
    }

  
  MPI_Request request[workers], recived_request[workers];
  for (i = 0; i < workers; i++)
  {
    MPI_Isend(&task[i] , 1, MPI_INT , i+1 , i, MPI_COMM_WORLD , &request[i]);
    MPI_Irecv(&temp[i], 1, MPI_INT, i+1 , i , MPI_COMM_WORLD , &recived_request[i]);
  }
  MPI_Waitall(workers, request, MPI_STATUS_IGNORE);
  for (i = workers; i<2*workers;i++){
    MPI_Isend(&task[i], 1, MPI_INT, i+1-workers, i , MPI_COMM_WORLD , &request[i-workers]);
    MPI_Irecv(&temp[i], 1, MPI_INT, i+1-workers , i , MPI_COMM_WORLD , &recived_request[i-workers]);
  }
  int index;
  while (i < MAX_TASKS)
  {
    MPI_Waitany(workers , recived_request , &index , &status);
    MPI_Irecv(&temp[i], 1, MPI_INT, index+1, i, MPI_COMM_WORLD, &recived_request[index]);
    MPI_Wait(&request[index] , &status); // confusion
    MPI_Isend(&task[i], 1, MPI_INT, index+1, i, MPI_COMM_WORLD, &request[index]);
    i++;
  }
  MPI_Waitall(workers, request , MPI_STATUS_IGNORE);
  MPI_Waitall(workers, recived_request , MPI_STATUS_IGNORE);
  for (i = 0; i < workers; i++)
  {
    MPI_Isend(&task[i], 1, MPI_INT, i+1, NO_MORE_TASKS, MPI_COMM_WORLD, &request[i]);
  }
  MPI_Waitall(workers, request , MPI_STATUS_IGNORE);
  for (int worker=1; worker<=workers; worker++) {
        int tasksdone = 0; int workdone = 0;
        for (int itask=0; itask<MAX_TASKS; itask++)
        if (result[itask]==worker) {
            tasksdone++;
            workdone += task[itask];
        }
        printf("Master: Worker %d , solved %d tasks\n", worker, tasksdone);
    }
  
}



void worker(int rank)
{
  int task, result, tag;
  int recived;
  MPI_Request request[2];
  MPI_Status status;
  bool first=true;
  std::vector<int> arr;
  MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  tag = status.MPI_TAG;
  while (tag != NO_MORE_TASKS)
  {
    MPI_Irecv(&recived , 1 , MPI_INT , 0 , MPI_ANY_TAG , MPI_COMM_WORLD , &request[1]);
    arr.push_back(task);
    
    if(first!=true){
      MPI_Wait(&request[0] , MPI_STATUS_IGNORE);
    }
    
    first=false;
    
    MPI_Isend(&rank, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &request[0]); 
    MPI_Wait(&request[1] , &status);
    
    

    task = recived;
    tag = status.MPI_TAG;
  }
  MPI_Wait(&request[0] , MPI_STATUS_IGNORE);
  for(int i = 0; i<arr.size();i++){
    printf("Worker %d, have %d \n", rank, arr[i]);
  }
}