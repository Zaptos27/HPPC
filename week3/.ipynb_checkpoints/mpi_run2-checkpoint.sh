#!/usr/bin/env bash
#SBATCH --job-name=HPPC_Group_1_code
#SBATCH --partition=modi_short
#SBATCH --time=120:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --exclusive

echo "Core 1, on file seq" >> data2.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data2.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data2.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data2.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data2.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data2.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data2.txt