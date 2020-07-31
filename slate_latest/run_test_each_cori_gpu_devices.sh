#!/bin/bash

#APP=$4
SIZE="${1}:${2}:${3}"
NUM_PROCS=$SLURM_JOB_NUM_NODES #$5
NUM_GPUS=4
NTASKS_NODE=$4
HOST= #"-host skylake09,skylake10,skylake11,skylake12"
TOTAL_PROCS=$(( $NUM_PROCS * $NTASKS_NODE )) 
GRID="$7"

# You should change this root to proper directory where you clone this artifact

EXP_ROOT=$HOME/scratch/PPoPP_exp

source $EXP_ROOT/hclib/hclib-install/bin/hclib_setup_env.sh
source $EXP_ROOT/env_cori_gpu.sh
cd $EXP_ROOT/slate_latest/test

EXP_DATE=`TZ='America/New_York' date +%Y%m%d_%H%M%S`
mkdir $EXP_DATE 

PE_PER_RANK=$8
N_PANEL=$(( ($PE_PER_RANK-1) / 2 ))
NB=$6
DEV=d
# OpenMP setting 
export OMP_NUM_THREADS=$PE_PER_RANK
export KMP_BLOCKTIME=0
export KMP_AFFINITY=granularity=core,compact,1,0
export MKL_DYNAMIC=false
export OMP_NESTED=true
# HCLIB OpenMP Setting
export HCLIB_WORKERS=$PE_PER_RANK

CORES_PER_TASK=$((80/$NTASKS_NODE)) 


export OMP_GANG_SCHED=1
if [ "$5" = "getrf" ] || [ "$5" = "geqrf" ]; then
  for app in $5 #getrf geqrf
  do
      export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
      export MKL_NUM_THREADS=1
# SLATE with LLVM OpenMP in optimal configuration
      srun -n $TOTAL_PROCS --ntasks-per-node $NTASKS_NODE -c $CORES_PER_TASK --gpus-per-node=$NUM_GPUS  $HOST ./tester $app --origin h --target $DEV --ref n --nb $NB --type d --lookahead 1 --panel-threads $N_PANEL --dim $SIZE $GRID --check n --repeat 4 &> ${EXP_DATE}/log_org_${app}
      sleep 2
# reference (MKL_multithreaded)
#      export MKL_NUM_THREADS=$PE_PER_RANK
#      srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c 40 $HOST ./tester $app --origin h --target t --ref o --nb $NB --type d,c --lookahead 1 --panel-threads 1 --dim $SIZE --repeat 6 &> ${EXP_DATE}/log_ref_${app}
    
#      sleep 2
# SLATE with HCLIB OpenMP with gang scheduling
      export MKL_NUM_THREADS=1
      export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
      srun -n $TOTAL_PROCS --ntasks-per-node $NTASKS_NODE -c $CORES_PER_TASK --gpus-per-node=$NUM_GPUS $HOST ./tester $app --origin h --target $DEV --ref n --nb $NB --type d --lookahead 1 --panel-threads $N_PANEL --dim $SIZE $GRID --repeat 4 --check n &> ${EXP_DATE}/log_hclib_${app}
  done
elif [ "$5" = "potrf" ]; then
  for app in potrf
  do
      export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
      export MKL_NUM_THREADS=1
      srun -n $TOTAL_PROCS --ntasks-per-node $NTASKS_NODE -c $CORES_PER_TASK --gpus-per-node=$NUM_GPUS $HOST ./tester $app --origin h --target $DEV --ref n --nb $NB --type d --lookahead 1 --dim $SIZE --repeat 4 $GRID &> ${EXP_DATE}/log_org_${app}
      sleep 2
#      export MKL_NUM_THREADS=$PE_PER_RANK
#      srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c 40  $HOST ./tester $app --origin h --target t --ref o --nb $NB --type d,c --lookahead 1 --dim $SIZE --repeat 6 &> ${EXP_DATE}/log_ref_${app}    
#     sleep 2
      export MKL_NUM_THREADS=1
      export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
      srun -n $TOTAL_PROCS --ntasks-per-node $NTASKS_NODE -c $CORES_PER_TASK --gpus-per-node=$NUM_GPUS $HOST ./tester $app --origin h --target $DEV --ref n --nb $NB --type d --lookahead 1 --dim $SIZE --repeat 4 $GRID &> ${EXP_DATE}/log_hclib_${app}
  done
fi
