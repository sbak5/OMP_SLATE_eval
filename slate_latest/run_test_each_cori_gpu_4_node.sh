#!/bin/bash

#APP=$4
SIZE="${1}:${2}:${3}"
NUM_PROCS=$SLURM_JOB_NUM_NODES #$5
NSOCKETS=$4
HOST= #"-host skylake09,skylake10,skylake11,skylake12"
TOTAL_PROCS=$(( $NUM_PROCS * $NSOCKETS )) 
# You should change this root to proper directory where you clone this artifact

EXP_ROOT=$HOME/scratch/PPoPP_exp
cd $EXP_ROOT/slate_latest/test
source $EXP_ROOT/hclib/hclib-install/bin/hclib_setup_env.sh
source $EXP_ROOT/env_cori_gpu.sh

EXP_DATE=`TZ='America/New_York' date +%Y%m%d_%H%M%S`
mkdir $EXP_DATE 

PE_PER_RANK=$9
N_PANEL=$(( ($PE_PER_RANK-1) / 2 ))

NUM_RANKS=$(($TOTAL_PROCS * $PE_PER_RANK))

IB=16
CORES_PER_TASK=$((80/$NSOCKETS)) 
# OpenMP setting 
export OMP_NUM_THREADS=$PE_PER_RANK
export KMP_BLOCKTIME=0
export KMP_AFFINITY=granularity=core,compact,1,0
export MKL_DYNAMIC=false
export OMP_NESTED=true

# HCLIB OpenMP Setting
export HCLIB_WORKERS=$PE_PER_RANK
export OMP_GANG_SCHED=1
TYPE="d"
GRID=$8
NB=$6
NB_SCALAPACK=$7
MPI_GRID="--p 4 --q 40"
ITER=3
if [ "$5" = "getrf" ] || [ "$5" = "geqrf" ]; then
  for app in $5 #getrf geqrf
  do
      if [ $app = "getrf" ]; then
        MPI_GRID="--p 4 --q 40"
    	NB_SCALAPACK=128
      elif [ $app = "geqrf" ]; then
        MPI_GRID="--p 10 --q 16"
	    NB_SCALAPACK=64
      fi

      export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
      export MKL_NUM_THREADS=1
# SLATE with LLVM OpenMP in optimal configuration
      srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK  $HOST ./tester $app --origin h --target t --ref n --nb $NB --ib $IB --type $TYPE --lookahead 1 --panel-threads $N_PANEL --dim $SIZE --repeat $ITER $GRID --check n &> ${EXP_DATE}/log_org_${app}
      sleep 2
# reference (MKL_multithreaded)
      export OMP_NUM_THREADS=1
      export MKL_NUM_THREADS=1
      srun -n $NUM_RANKS --ntasks-per-node $(($NUM_RANKS/$NUM_PROCS)) -c 2 $HOST ./tester $app --origin h --target t --ref o --nb $NB_SCALAPACK --ib $IB --type $TYPE --lookahead 1 --panel-threads 1 --dim $SIZE --repeat $ITER $MPI_GRID --check n &> ${EXP_DATE}/log_ref_mpi_${app}

      sleep 2
# SLATE with HCLIB OpenMP with gang scheduling
      export OMP_NUM_THREADS=$PE_PER_RANK
      export MKL_NUM_THREADS=1
      export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
      srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK  $HOST ./tester $app --origin h --target t --ref n --nb $NB --ib $IB --type $TYPE --lookahead 1 --panel-threads $N_PANEL --dim $SIZE --repeat $ITER $GRID --check n &> ${EXP_DATE}/log_hclib_${app}
  done
elif [ "$5" = "potrf" ]; then
  for app in potrf
  do
      MPI_GRID="--p 10 --q 16"
      export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
      export MKL_NUM_THREADS=1
      srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK $HOST ./tester $app --origin h --target t --ref n --nb $NB --type $TYPE --lookahead 1 --dim $SIZE --repeat $ITER $GRID --check n &> ${EXP_DATE}/log_org_${app}
      sleep 2
      # MPI + OpenMP(Multithreaded MKL)
      # MPI Only
      export OMP_NUM_THREADS=1
      export MKL_NUM_THREADS=1
      srun -n $NUM_RANKS --ntasks-per-node $(($NUM_RANKS/$NUM_PROCS)) -c 2 $HOST ./tester $app --origin h --target t --ref o --nb 64 --type $TYPE --lookahead 1 --dim $SIZE --repeat $ITER $MPI_GRID --check n &> ${EXP_DATE}/log_ref_mpi_${app}    

      sleep 2
      export OMP_NUM_THREADS=$PE_PER_RANK
      export MKL_NUM_THREADS=1
      export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
      srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK $HOST ./tester $app --origin h --target t --ref n --nb $NB --type $TYPE --lookahead 1 --dim $SIZE --repeat $ITER $GRID --check n &> ${EXP_DATE}/log_hclib_${app}
  done
fi
