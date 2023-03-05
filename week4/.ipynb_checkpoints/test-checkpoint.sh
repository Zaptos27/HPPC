#!/usr/bin/env bash
g++  sies_omp_Jonas.cpp -O3 -ffast-math -Wall -march=native -g -std=c++14 -fopenmp -o mpjonas.out
sbatch job_1.sh