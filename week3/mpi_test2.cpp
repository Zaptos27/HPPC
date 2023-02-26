#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define MAX_TASKS 16
#define NO_MORE_TASKS MAX_TASKS+1

void master (int workers);
void worker (int rank);

int main(int argc, char *argv[])
{
  int np, rank;

  srand((int) time(NULL));  // generator from outside
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  if (rank == 0) {
    master(np-1);
  } else {
     worker(rank);
  }
  MPI_Finalize();
}

void master (int workers)
{
    //create tasks
    int i, result[MAX_TASKS], temp, tag, who;
    MPI_Status status;
   int task[MAX_TASKS];// = {5, 5, 5, 2, 5, 2, 2, 2, 3, 5, 2, 1, 2, 1, 1, 2};
  for (int i = 0; i < MAX_TASKS; i++)
  {
    task[i] = rand() % 5 + 1;
  }
  
  


   // Assume at least as many tasks as workers
   for (i=0; i<workers; i++) {
     MPI_Send(&task[i], 1, MPI_INT, i+1, i, MPI_COMM_WORLD);
   }  

   while (i<MAX_TASKS) {
     MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, 
                                 MPI_COMM_WORLD, &status);
     who = status.MPI_SOURCE;
     tag = status.MPI_TAG;
     printf("time %d , who %d||", task[i],who);
     result[tag] = temp;
     MPI_Send(&task[i], 1, MPI_INT, who, i, MPI_COMM_WORLD);
     i++;
   }

   for (i=0; i<workers; i++) {
     MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, 
                                 MPI_COMM_WORLD, &status);
     who = status.MPI_SOURCE;
     tag = status.MPI_TAG;
     result[tag] = temp;
     MPI_Send(&task[i], 1, MPI_INT, who, NO_MORE_TASKS, 
                                            MPI_COMM_WORLD);
   }  
}


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
     result = rank;
     workdone+=task;
     tasksdone++;
     MPI_Send(&result, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
     MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                                                        &status);
     tag = status.MPI_TAG;
   }
   printf("Worker %d did %d tasks totalling a wait of %d seconds \n", rank, tasksdone, workdone);
}