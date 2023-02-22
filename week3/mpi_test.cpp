#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <mpi.h>

#define MAX_TASKS 16
#define NO_MORE_TASKS MAX_TASKS+1

void farmer (int workers);
void worker (int rank);

int main(int argc, char *argv[])
{

  int np, rank;

  srand((int) 1003123124214);  // generator from outside
  printf("test %d",rand()%3+1);
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  if (rank == 0) {
     farmer(np-1);
  } else {
     worker(rank);
  }
  MPI_Finalize();
}

void farmer (int workers)
{
   int i, task[MAX_TASKS], result[MAX_TASKS], temp, tag, who;
   MPI_Status status;
   
   for (i=0; i<MAX_TASKS; i++) {
     task[i] = rand()%3+1;   // set up some "tasks"
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
   printf("Worker %d solved %d tasks totalling %d units of work \n", rank, tasksdone, workdone);
}