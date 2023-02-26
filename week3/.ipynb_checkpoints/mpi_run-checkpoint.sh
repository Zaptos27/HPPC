#!/usr/bin/env bash
#SBATCH --job-name=HPPC_Group_1_code
#SBATCH --partition=modi_short
#SBATCH --time=120:00
#SBATCH --nodes=1
#SBATCH --ntasks=64
#SBATCH --exclusive

echo "Core 1, on file seq" >> data.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data.txt
mpiexec singularity exec ~/modi_images/hpc-notebook-latest.sif ./task_seq.out >> data.txt

for FILE in task_1.out task_2.out task_3.out
do
    for i in 2 4 8 12 16 24 32 40 48 56 64
    do
        echo "Core $i, on file $FILE" >> data.txt
        mpiexec -np $i singularity exec ~/modi_images/hpc-notebook-latest.sif ./$FILE >> data.txt
        mpiexec -np $i singularity exec ~/modi_images/hpc-notebook-latest.sif ./$FILE >> data.txt
        mpiexec -np $i singularity exec ~/modi_images/hpc-notebook-latest.sif ./$FILE >> data.txt
    done
done