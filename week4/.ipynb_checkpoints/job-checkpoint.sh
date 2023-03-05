#!/usr/bin/env bash
#SBATCH --job-name=Seismogram
#SBATCH --partition=modi_devel
#SBATCH --nodes=1 --ntasks=64 --threads-per-core=1
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

# set loop scheduling to static
export OMP_SCHEDULE=static

# Schedule one thread per core. Change to "threads" for hyperthreading
#export OMP_PLACES=cores
export OMP_PLACES=threads

# Place threads as close to each other as possible
export OMP_PROC_BIND=close

# Set and print number of cores / threads to use

echo "Core 1, seq" >> data.txt
singularity exec ~/modi_images/hpc-notebook-latest.sif ./seq.out >> data.txt

for i in 1 2 4 8 12 16 24 32 40 48 56 64
    do
        echo "Core $i, 1" >> data.txt
        export OMP_NUM_THREADS=$i
        echo Number of threads=$OMP_NUM_THREADS
        singularity exec ~/modi_images/hpc-notebook-latest.sif ./mpjonas.out >> data.txt
        echo "Core $i, 2" >> data.txt
        singularity exec ~/modi_images/hpc-notebook-latest.sif ./mp.out >> data.txt
    done

# uncomment to write info about binding and environment variables to screen
#export OMP_DISPLAY_ENV=true
