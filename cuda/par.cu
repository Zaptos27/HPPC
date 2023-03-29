#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>
#include <numeric>
//#include "argparse.hpp"
#include <cassert>
#include <fstream>
typedef float real_t;
#include <cooperative_groups.h>
using namespace cooperative_groups;


__constant__ size_t N_x;
__constant__ size_t N_y;
__constant__ real_t d_x;
__constant__ real_t d_y;
__constant__ real_t d_t;
__constant__ real_t C_i;
__constant__ real_t K_i;
__constant__ real_t D_p_i;
__constant__ real_t D_q_i;

class Sim_Configuration {
public:
    int iter = 1000;  // Number of iterations
    int data_period = 100;  // how often to save coordinate to file
    int size = 1024;
    real_t K = 9.0, C = 4.5, D_p = 1.0, D_q = 8.0;
    std::string filename = "chemicals.data";   // name of the output file with history

    Sim_Configuration(std::vector <std::string> argument){
        for (long unsigned int i = 1; i<argument.size() ; i += 2){
            std::string arg = argument[i];
            if(arg=="-h"){ // Write help
                std::cout << "./par --iter <number of iterations> --dt <time step>"
                          << " --g <gravitational const> --dx <x grid size> --dy <y grid size>"
                          << "--fperiod <iterations between each save> --out <name of output file>\n";
                exit(0);
            } else if (i == argument.size() - 1) {
                throw std::invalid_argument("The last argument (" + arg +") must have a value");
            } else if(arg=="--iter"){
                if ((iter = std::stoi(argument[i+1])) < 0) 
                    throw std::invalid_argument("iter most be a positive integer (e.g. -iter 1000)");
            } else if (arg=="--size"){
                if ((size = std::stoi(argument[i+1])) < 0) 
                    throw std::invalid_argument("size most be a positive integer (e.g. --size 100)");
            } else if(arg=="--fperiod"){
                if ((data_period = std::stoi(argument[i+1])) < 0) 
                    throw std::invalid_argument("dy most be a positive integer (e.g. -fperiod 100)");

            } else if(arg=="--out"){
                filename = argument[i+1];
            } else{
                std::cout << "---> error: the argument type is not recognized \n";
            }
        }
    }
};

/** Representation of the Chemicals domain including ghost lines, which is a "1-cell padding" of rows and columns
 *  around the world. These ghost lines are used to implement Neumann Boundary conditions. */
class Chemicals {
public:
    size_t NX, NY; // The shape of the Chemicals world including ghost lines.
    real_t D_p, D_q, C, K; 
    real_t dx, dy, dt;
    std::vector<real_t> P; // Virtual Chemical 1;
    std::vector<real_t> Q; // Virtual Chemical 2;
    std::vector<real_t> dPdt; // Time derivative of Chemical 1;
    std::vector<real_t> dQdt; // Time derivative of Chemical 2;

    Chemicals(size_t NX, size_t NY, real_t K = 9.0, real_t C = 4.5, real_t D_p = 1.0, real_t D_q = 8.0) : 
        P(NX*NY, 0), Q(NX*NY, 0), dQdt(NX*NY,0), dPdt(NX*NY,0), NX(NX), NY(NY), K(K), C(C), D_p(D_p), D_q(D_q)
    {
        //Initial conditions is a centered square of different initial concentrations.
        for (size_t i = NY/4; i < NY - NY/4 ; ++i) 
        for (size_t j = NX/4; j < NX - NX/4 ; ++j) {
            Q[i*NX + j] =  (K / C) + 0.2;
            P[i*NX + j] =  C + 0.1;
        }

        //Largest approximate timestep determined empirically from function fitting.
        dt = 0.02 * std::pow(real_t(std::min(NX,NY))/40.0,-2);

        //The effective spatial domain is kept fixed by adjusting the spatial resolution dx, dy.
        dx = 40/real_t(NX); dy = 40.0/real_t(NY);
    }
};

void to_file(const std::vector<std::vector<real_t>> &chemical_history, const std::string &filename){
    std::ofstream file(filename);
    std::cout << "Produced " << chemical_history.size() << " frames" <<std::endl;
    for (size_t i = 0; i < chemical_history.size(); i++)
    {
        file.write((const char*)(chemical_history.at(i).data()), sizeof(real_t)*chemical_history.at(0).size());
    }
    
}

/** Exchange the horizontal ghost lines i.e. copy the second data row to the very last data row and vice versa.
 *
 * @param data   The data update, which could be the Chemicals P or Q.
 * @param shape  The shape of data including the ghost lines.
 */
 
 __global__
void horizontal_boundary(real_t* data) {        
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < N_x; i += stride){
        const uint64_t top_inside = 2 * N_x + i;
        const uint64_t bot_ghost = (N_y - 1) * N_x + i;
        const uint64_t bot_inside = (N_y - 3) * N_x + i;
        const uint64_t top_ghost = 0 * N_x + i;

        data[bot_ghost]  = data[bot_inside];
        data[top_ghost]  = data[top_inside];   
    }
}

/** Exchange the vertical ghost lines.
 *
 * @param data   The data update, which could be the Chemicals P or Q.
 * @param shape  The shape of data including the ghost lines.
 */
 
 __global__
void vertical_boundary(real_t* data) {
    
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < N_y; i += stride){
    
        const uint64_t left_ghost  = i * N_y + 0;
        const uint64_t right_inside = i * N_y + N_x - 3;
        const uint64_t right_ghost = i * N_x + N_x - 1;
        const uint64_t left_inside  = i * N_x + 2;
        
        data[left_ghost]  = data[right_inside];
        data[right_ghost] = data[left_inside];   
    }
}



/** One integration step
 *
 * @param c The Chemicals world to update.
 */
__global__
void integrate(real_t* P, real_t* Q, real_t* dPdt, real_t* dQdt) {

    int index = blockIdx.x * blockDim.x + threadIdx.x + N_x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < N_y*N_x - N_x; i += stride){
        real_t P2Q = P[i]*P[i] * Q[i];
        dPdt[i] = D_p_i * ( P[i - 1] + P[i + 1] + P[i+N_x] + P[i-N_x] - 4*P[i]) / (d_x * d_y) + C_i + P2Q - (K_i + 1.0) * P[i];
        dQdt[i] = D_q_i * ( Q[i - 1] + Q[i + 1] + Q[i+N_x] + Q[i-N_x] - 4*Q[i]) / (d_x * d_y) + P[i] * K_i - P2Q;
    
    }
    
    index = blockIdx.x * blockDim.x + threadIdx.x + N_x;
    stride = blockDim.x * gridDim.x;
    for (int i = index; i < N_y*N_x - N_x; i += stride){
            //Euler integration
            P[i] += d_t * dPdt[i];
            Q[i] += d_t * dQdt[i];
        }
}

/** Simulation of Chemicals
 *
 * @param num_of_iterations  The number of time steps to simulate
 * @param size               The x and y domain size
 * @param output_filename    The filename of the written Chemicals history
 */
void simulate(const Sim_Configuration &config) {
    using namespace std::chrono_literals;
    // We pad the world with ghost lines (two in each dimension)
    Chemicals chemicals = Chemicals(config.size, config.size);

    std::vector <std::vector<real_t>> chemicals_history;
    double checksum = 0;
    size_t N = chemicals.NX * chemicals.NY;
    
    real_t* P = chemicals.P.data(); 
    real_t* Q = chemicals.Q.data();

    cudaMemcpyToSymbol(N_x, &chemicals.NX, sizeof(size_t));
    cudaMemcpyToSymbol(N_y, &chemicals.NY, sizeof(size_t));
    cudaMemcpyToSymbol(d_x, &chemicals.dx, sizeof(real_t));
    cudaMemcpyToSymbol(d_y, &chemicals.dy, sizeof(real_t));
    cudaMemcpyToSymbol(d_t, &chemicals.dt, sizeof(real_t));
    cudaMemcpyToSymbol(D_p_i, &chemicals.D_p, sizeof(real_t));
    cudaMemcpyToSymbol(D_q_i, &chemicals.D_q, sizeof(real_t));
    cudaMemcpyToSymbol(C_i, &chemicals.C, sizeof(real_t));
    cudaMemcpyToSymbol(K_i, &chemicals.K, sizeof(real_t));
    
    real_t *p;
    cudaMalloc((void **)&p, N*sizeof(real_t));
    cudaMemcpy(p, chemicals.P.data(), N*sizeof(real_t), cudaMemcpyHostToDevice);
        
    real_t *q;
    cudaMalloc((void **)&q, N*sizeof(real_t));
    cudaMemcpy(q, chemicals.Q.data(), N*sizeof(real_t), cudaMemcpyHostToDevice);
    
    real_t *dp;
    cudaMalloc((void **)&dp, N*sizeof(real_t));
    cudaMemcpy(dp, chemicals.dPdt.data(), N*sizeof(real_t), cudaMemcpyHostToDevice);
    
    real_t *dq;
    cudaMalloc((void **)&dq, N*sizeof(real_t));
    cudaMemcpy(dq, chemicals.dQdt.data(), N*sizeof(real_t), cudaMemcpyHostToDevice);
    
    int threads = 1024;
    int blocks = 14;
    
    auto begin = std::chrono::steady_clock::now();
    
    for (uint64_t t = 0; t < config.iter; ++t) {
        
        vertical_boundary<<<blocks,threads>>>(p);
        vertical_boundary<<<blocks,threads>>>(q);
        horizontal_boundary<<<blocks,threads>>>(p);
        horizontal_boundary<<<blocks,threads>>>(q);
        
        integrate<<<blocks,threads>>>(p,q,dp,dq);
        
        if (t % config.data_period == 0) {
            cudaMemcpy(chemicals.P.data(), p, N*sizeof(real_t), cudaMemcpyDeviceToHost);
            chemicals_history.push_back(chemicals.P);
            std::cout << t;
            
        }
    
    }
    cudaDeviceSynchronize();
    auto end = std::chrono::steady_clock::now();

    /**If you want to check the output: **/
    to_file(chemicals_history, config.filename);    
    
    cudaMemcpy(chemicals.P.data(), p, N*sizeof(real_t), cudaMemcpyDeviceToHost);
    checksum += std::accumulate(chemicals.P.begin(), chemicals.P.end(), 0.0);
    std::cout << "checksum: " << checksum << std::endl;
    std::cout << "elapsed time: " << (end - begin).count() / 1000000000.0 << " sec" << std::endl;
}

/** Main function that parses the command line and start the simulation */
int main(int argc, char **argv) {
    auto config = Sim_Configuration({argv, argv+argc});
    simulate(config);
    return 0;
}
__global__ void device_copy_vector4_kernel(int* d_in, int* d_out, int N) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  for(int i = idx; i < N/4; i += blockDim.x * gridDim.x) {
    reinterpret_cast<real_t*>(d_out)[i] = reinterpret_cast<int4*>(d_in)[i];
  }