#include <mpi.h>
#include <iostream>

int main(int argc, char *argv[]){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::cout << "I am" << rank << " of " << size;
    MPI_Finalize();
    return 0;
}