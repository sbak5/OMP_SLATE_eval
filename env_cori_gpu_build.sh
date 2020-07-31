#!/bin/bash
GCC_VER=7.4.0
INTEL_VER=2019
#module swap PrgEnv-intel PrgEnv-gnu
#module swap PrgEnv-intel PrgEnv-gnu
module load PrgEnv-intel
module swap craype-haswell craype-x86-skylake
#module unload PrgEnv-gnu
#module unload gcc
module load gcc
#module unload cuda
module load cuda/10.2.89
#module load cuda/10.1.168
export CUDA_ROOT=/usr/common/software/cuda/10.2.89
export CPATH=$CUDA_ROOT/include:$CPATH
export LIBRARY_PATH=$CUDA_ROOT/lib64:$LIBRARY_PATH
export LD_LIBRARY_PATH=$CUDA_ROOT/lib64:$LD_LIBRARY_PATH
#/usr/common/software/cuda/10.1.168/extras/CUPTI/lib64:/usr/common/software/cuda/10.1.168/lib64:/usr/common/software/cudnn/7.6.5/cuda/10.1.168/lib64
#module load impi/2019.up3
#module load intel/19.0.3.199
#module load intel
#source /global/common/cori_cle7/software/intel/parallel_studio_xe_2020_cluster_edition/compilers_and_libraries_2020.0.166/linux/mpi/intel64/bin/mpivars.sh intel64
source /global/common/cori_cle7/software/intel/parallel_studio_xe_2020_cluster_edition/compilers_and_libraries_2020.0.166/linux/mkl/bin/mklvars.sh intel64
#source /global/common/cori_cle7/software/intel/parallel_studio_xe_2019_update6_composer_edition/compilers_and_libraries_2019/linux/mkl/bin/mklvars.sh intel64
module load openmpi/4.0.3
#module load impi/2019.up3
#source /soft/compilers/intel-2018/vtune_amplifier_2018/amplxe-vars.sh
