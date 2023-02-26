#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define MAX_TASKS 16
#define NO_MORE_TASKS MAX_TASKS + 1

void farmer(int workers);
void worker(int rank);

int main(int argc, char *argv[])
{
  int np, rank;

  srand((int)time(NULL));
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  if (rank == 0)
  {
    farmer(np - 1);
  }
  else
  {
    worker(rank);
  }
  MPI_Finalize();
}

void farmer(int workers)
{
  
  int i, result[MAX_TASKS], temp[MAX_TASKS], tag, who;
  MPI_Status status;
  int task[MAX_TASKS];// = {5, 5, 5, 2, 5, 2, 2, 2, 3, 5, 2, 1, 2, 1, 1, 2};
  
  for (int i = 0; i < MAX_TASKS; i++)
  {
    task[i] = rand() % 5 + 1;
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
    printf("tag %d, temp[index] %d, result %d \n",status.MPI_TAG, temp[index],result[status.MPI_TAG]);
    MPI_Wait(&request[index] , &status); // confusion
    printf("tag: %d, source: %d \n", status.MPI_TAG,status.MPI_SOURCE);
    MPI_Isend(&task[i], 1, MPI_INT, index+1, i, MPI_COMM_WORLD, &request[index]);
    printf("time %d , who %d|| \n", task[i],index+1);
    i++;
  }
  printf("Wait\n");
  MPI_Waitall(workers, request , MPI_STATUS_IGNORE);
  MPI_Waitall(workers, recived_request , MPI_STATUS_IGNORE);
  for (i = 0; i < workers; i++)
  {
    MPI_Isend(&task[i], 1, MPI_INT, i+1, NO_MORE_TASKS, MPI_COMM_WORLD, &request[i]);
  }
  MPI_Waitall(workers, request , MPI_STATUS_IGNORE);
  
  for (i=0; i<MAX_TASKS;i++){
    printf("%d, ",temp[i]);
  }
  printf(",,\n");
  
  for (i=0; i<MAX_TASKS;i++){
    printf("%d, ",task[i]);
  }
  printf(",,\n");
}


/*
void worker (int rank)
{
   int tasksdone = 0;
   int workdone  = 0;
   int task, result, tag;
   MPI_Status status;

   MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                                                      &status);
   tag = status.MPI_TAG;
   while (tag != NO_MORE_TASKS) {
     sleep(task);
     workdone+=task;
     tasksdone++;
     MPI_Send(&rank, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
     MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                                                        &status);
     tag = status.MPI_TAG;
   }
   printf("Worker %d did %d tasks totalling a wait of %d seconds \n", rank, tasksdone, workdone);
}*/

void worker(int rank)
{
  int tasksdone = 0;
  int workdone = 0;
  int task, result, tag;
  int recived;
  MPI_Request request[2];
  MPI_Status status;
  bool first=true;
  printf("%d", rank);
  MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  tag = status.MPI_TAG;
  while (tag != NO_MORE_TASKS)
  {
    MPI_Irecv(&recived , 1 , MPI_INT , 0 , MPI_ANY_TAG , MPI_COMM_WORLD , &request[1]);
    sleep(task);
    result = rank;
    workdone += task;
    tasksdone++;
    
    if(first!=true){
      MPI_Wait(&request[0] , MPI_STATUS_IGNORE);
    }
    
    first=false;
    
    MPI_Isend(&result, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &request[0]); 
    MPI_Wait(&request[1] , &status);
    
    

    task = recived;
    tag = status.MPI_TAG;
  }
  MPI_Wait(&request[0] , MPI_STATUS_IGNORE); 
  printf("Worker %d solved %d tasks totalling %d units of work \n", rank, tasksdone, workdone);
}