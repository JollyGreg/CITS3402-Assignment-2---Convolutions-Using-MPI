#!/bin/bash
# Student Name: Liam Bush
# Student Number: 24227223
# Student Name: Amir Husain
# Student Number: 23380159

#SBATCH --job-name=stride_conv_mpi
#SBATCH --output=results/stride_conv_%j.out
#SBATCH --error=results/stride_conv_%j.err
#SBATCH --nodes=4
#SBATCH --partition=work
#SBATCH --account=courses0101
#SBATCH --mem=4G
#SBATCH --time=00:05:00
##SBATCH --mail-type=BEGIN,END,FAIL
##SBATCH --mail-user=24227223@student.uwa.edu.au

# Clean and build the project
make clean
make

# Create data directory if it doesn't exist
mkdir -p data

srun ./Convolution_stride -H 10 -W 10               -kH 10 -kW 10       -sH 1 -sW 1 -p 0
srun ./Convolution_stride -H 100 -W 100             -kH 10 -kW 10       -sH 1 -sW 1 -p 0
srun ./Convolution_stride -H 1000 -W 1000           -kH 10 -kW 10       -sH 1 -sW 1 -p 0
srun ./Convolution_stride -H 10000 -W 10000         -kH 10 -kW 10       -sH 1 -sW 1 -p 0
srun ./Convolution_stride -H 10000 -W 10000         -kH 100 -kW 100     -sH 1 -sW 1 -p 0
srun ./Convolution_stride -H 10000 -W 10000         -kH 1000 -kW 1000   -sH 1 -sW 1 -p 0
srun ./Convolution_stride -H 10000 -W 10000         -kH 10000 -kW 10000 -sH 1 -sW 1 -p 0