#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <random>
#include <chrono>
#include <thread>
#include <array>
#include <mpi.h>

const int MAX_TASKS=5000;  // number of tasks
const int NO_MORE_TASKS = MAX_TASKS+1;
const int RANDOM_SEED=1234;

void master (int workers);
void worker (int rank);

int main(int argc, char *argv[])
{
  int np, rank;

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
    int i, temp, tag, who;
    MPI_Status status;
    std::array<int, MAX_TASKS> task, result;

    // set up a random number generator
    std::random_device rd;
    //std::default_random_engine engine(rd());
    std::default_random_engine engine;
    engine.seed(RANDOM_SEED);
    // make a distribution of random integers in the interval [0:30]
    std::uniform_int_distribution<int> distribution(0, 30);

    for (int& t : task) {
        t = distribution(engine);   // set up some "tasks"
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
     std::this_thread::sleep_for(std::chrono::milliseconds(task));
     MPI_Send(&rank, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
     MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                                                        &status);
     tag = status.MPI_TAG;
   }
}