Evaluation Repo For Integration of HClib and OpenMP
=================================================================

This repo includes src codes for Habanero-C library(HClib) and 
LLVM OpenMP runtime modified to run on HClib. 

HClib is forked from the main repo (https://github.com/habanero-rice/hclib). 
The base commit where our changes are applied is 32ada897f673edd56375e83a58c1a8cfd5ba5dcd.

LLVM OpenMP runtime is forked from https://github.com/llvm-mirror/openmp. 
The commit where we started our modification is ba56714719294ad7aa15a1351c201e642445e2ab.

SLATE library we used for evaluation is forked from 
https://bitbucket.org/icl/slate/commits/25d580fb1b02a411d088d5838d10b27cee2124d9. 

We tested on NERSC Cori Haswell nodes(Intel E5-2698v3-16C/32T, 2 socket/node) and GPU nodes (Intel Skylake 6148-20C/40T+V100)
and ANL JLSE Skylake(Intel Skylake 8180M-28C/56T, 2 socket/node) nodes

.. image:: https://zenodo.org/badge/256908571.svg
   :target: https://zenodo.org/badge/latestdoi/256908571

Dependencies
---------------------------------------------
We used following libraries to build HClib, LLVM OpenMP runtime and SLATE

- gcc 8.3.0
- Open MPI 4.0.3 / Intel MKL 2020.0.166 (For SLATE)
- hwloc 2.1.0 (For HClib)
- netlib-scalapack, netlib-lapack (For SLATE)
- CUDA 10.2.89 (For SLATE)

env_cori_gpu.sh (for Cori GPU) include environment settings on each machine

Installation
---------------------------------------------
First, set up environmental variables with env_cori_build.sh.

HClib should be built first and then LLVM OpenMP, SLATE can be built. 

- HClib Installation
  
  .. code-block:: console
    
    $ cd hclib
    $ CXX=g++ CC=gcc ./install.sh

- LLVM OpenMP installation

  .. code-block:: console
  
    $ cd openmp
    $ make

- SLATE

  .. code-block:: console
    
    $ cd slate_latest
    $ make # Configuration for SLATE is stored in slate/make.inc

Instruction for Experiments
---------------------------------------------
- Scripts to run experiments are stored in "slate/test". 


- The following is an example to run the script with appropriate paramters on slurm-supported systems. 

  .. code-block:: console

    $ sbatch -n 16 -q gpu --contiguous -t 06:00:00 ./run_test_each_cori.sh 10000 80000 10000 1 [getrf,geqrf,potrf] [process_grid]
    # Run this script on multi nodes with input matrices of [10000:80000:10000] columns

- Each script will generate results at slate/test/<DATE>_<TIME>. You can use parse_cori.sh to generate average of multiple runs in the result directory

  .. code-block:: console

    $ cd test/20200421_004804 
    $ ../../parse_<jlse/cori>.sh getrf 1000 10000 1000
    # Generate mean and standard deviation of runs on LU (getrf) with size of matrices [1000:10000:1000] x [1000:10000:1000]
