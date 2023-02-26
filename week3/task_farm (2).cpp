/*
  Assignment: Make an MPI task farm. A "task" is a randomly generated integer.
  To "execute" a task, the worker sleeps for the given number of milliseconds.
  The result of a task should be send back from the worker to the master. It
  contains the rank of the worker
*/

#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <array>

// To run an MPI program we always need to include the MPI headers
#include <mpi.h>

const int NTASKS=100;  // number of tasks
const int RANDOM_SEED=1234;

void master (int nworker) {
    std::array<int, NTASKS> task, result;

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
    
    //Initialise send task to all workers
    int i;
    for (i = 0; i < nworker; i++) {
        MPI_Send(&task[i], 1, MPI_INT, i+1, i, MPI_COMM_WORLD);
    }
    
    
    MPI_Status status0;
    while (i < NTASKS) {
        //check if any worker is done
        MPI_Recv(&result[i], 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status0);
        
        //if worker done, send new task.
        MPI_Send(&task[i], 1, MPI_INT, status0.MPI_SOURCE, i, MPI_COMM_WORLD);
        i++;
    }
    
    
    // Because we do not send more tasks in while loop (we reached NTASKS), it stops.
    // BUT we have not got the last tasks, which we should also recieve.
    for (int k = i - nworker; k < i; k++) {
        MPI_Recv(&result[k], 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status0);
    }
    // CHECK FOR SIZE OF result
    //# define my_sizeof(type) ((char *)(&type+1)-(char*)(&type))
    //std::cout << my_sizeof(result)/my_sizeof(result[0]) << "\n";
    
    //stop the while loop in worker.
    int kill = -1;
    for (int k = 1; k < nworker+1; k++) {
        MPI_Send(&kill, 1, MPI_INT, k, k + NTASKS, MPI_COMM_WORLD);
    }
    
    
    // Print out a status on how many tasks were completed by each worker
    for (int worker=1; worker<=nworker; worker++) {
        int tasksdone = 0; int workdone = 0;
        for (int itask=0; itask<NTASKS; itask++)
        if (result[itask]==worker) {
            tasksdone++;
            workdone += task[itask];
        }
        std::cout << "Master: Worker " << worker << " solved " << tasksdone << 
                    " tasks\n";    
    }
}

// call this function to complete the task. It sleeps for task milliseconds
void task_function(int task) {
    std::this_thread::sleep_for(std::chrono::milliseconds(task));
}

void worker (int rank) {
    int recieve;
    MPI_Status status1;
    MPI_Recv(&recieve, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status1);
    
    while (recieve != -1) {
        MPI_Send(&rank, 1, MPI_INT, 0, status1.MPI_TAG, MPI_COMM_WORLD);
        task_function(recieve);
        
        MPI_Recv(&recieve, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status1);
    }
}

int main(int argc, char *argv[]) {
    int nrank, rank;

    MPI_Init(&argc, &argv);               // set up MPI
    MPI_Comm_size(MPI_COMM_WORLD, &nrank); // get the total number of ranks
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // get the rank of this process

    if (rank == 0)       // rank 0 is the master
        master(nrank-1); // there is nrank-1 worker processes
    else                 // ranks in [1:nrank] are workers
        worker(rank);
    MPI_Finalize();      // shutdown MPI
}
