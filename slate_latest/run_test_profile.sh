#!/bin/bash

#APP=$4
NUM_PROCS=$SLURM_JOB_NUM_NODES #$5
NSOCKETS=$4
HOST= #"-host skylake09,skylake10,skylake11,skylake12"
TOTAL_PROCS=$(( $NUM_PROCS * $NSOCKETS )) 
NUM_GPU=4
# You should change this root to proper directory where you clone this artifact

EXP_ROOT=$HOME/scratch/PPoPP_exp
cd $EXP_ROOT/slate_latest/test
source $EXP_ROOT/hclib/hclib-install/bin/hclib_setup_env.sh
source $EXP_ROOT/env_cori_gpu.sh

EXP_DATE=`TZ='America/New_York' date +%Y%m%d_%H%M%S`
mkdir $EXP_DATE 
mkdir $EXP_DATE/getrf
mkdir $EXP_DATE/geqrf
mkdir $EXP_DATE/potrf
PE_PER_RANK=$9
N_PANEL=$(( ($PE_PER_RANK-1) / 2 ))
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
NB=192
NB_SCALAPACK=$7
ITER=3
TRACE="--trace y --trace-scale 2000"
rm *.svg
  for app in getrf geqrf
  do
    for mat in 6000 25000
    do
      SIZE="$mat:$mat:100"
      for trace_iter in `seq 3`
      do
        export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
        export MKL_NUM_THREADS=1
        if [ $app = "getrf" ]; then
          GRID="--p 1 --q 4"
          MPI_GRID="--p 2 --q 20"
        elif [ $app = "geqrf" ]; then
          GRID="--p 2 --q 2"
          MPI_GRID="--p 5 --q 8"
        fi
# SLATE with LLVM OpenMP in optimal configuration
        srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK  $HOST ./tester $app --origin h --target t --ref n --nb $NB --ib $IB --type $TYPE --lookahead 1 --panel-threads $N_PANEL --dim $SIZE --repeat $ITER $GRID $TRACE &> ${EXP_DATE}/log_org_${app}_${mat}_t_${trace_iter}
        mv *.svg ${EXP_DATE}/$app
        sleep 2
# SLATE with HCLIB OpenMP with gang scheduling
        export MKL_NUM_THREADS=1
        export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
        srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK  $HOST ./tester $app --origin h --target t --ref n --nb $NB --ib $IB --type $TYPE --lookahead 1 --panel-threads $N_PANEL --dim $SIZE --repeat $ITER $GRID $TRACE &> ${EXP_DATE}/log_hclib_${app}_${mat}_t_${trace_iter}
        mv *.svg ${EXP_DATE}/$app
      done
    done
  done

  for app in getrf geqrf
  do
    for mat in 25000 35000
    do
      SIZE="$mat:$mat:100"
      for trace_iter in `seq 3`
      do
        export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
        export MKL_NUM_THREADS=1
        if [ $app = "getrf" ]; then
          GRID="--p 1 --q 4"
          MPI_GRID="--p 2 --q 20"
        elif [ $app = "geqrf" ]; then
          GRID="--p 2 --q 2"
          MPI_GRID="--p 5 --q 8"
        fi
# SLATE with LLVM OpenMP in optimal configuration
        srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK --gpus-per-node=$NUM_GPU $HOST ./tester $app --origin h --target d --ref n --nb 256 --ib $IB --type $TYPE --lookahead 1 --panel-threads $N_PANEL --dim $SIZE --repeat $ITER $GRID $TRACE --check n &> ${EXP_DATE}/log_org_${app}_${mat}_d_${trace_iter}
        mv *.svg ${EXP_DATE}/$app
        sleep 2
# SLATE with HCLIB OpenMP with gang scheduling
        export MKL_NUM_THREADS=1
        export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
        srun -n $TOTAL_PROCS --ntasks-per-node $NSOCKETS -c $CORES_PER_TASK  --gpus-per-node=$NUM_GPU $HOST ./tester $app --origin h --target d --ref n --nb 256 --ib $IB --type $TYPE --lookahead 1 --panel-threads $N_PANEL --dim $SIZE --repeat $ITER $GRID $TRACE --check n &> ${EXP_DATE}/log_hclib_${app}_${mat}_d_${trace_iter}
        mv *.svg ${EXP_DATE}/$app
      done
    done
  done

  for app in potrf
  do
    GRID="--p 1 --q 2"
    export OMP_NUM_THREADS=20
    export HCLIB_WORKERS=20
    for mat in 6000 14000
    do
      SIZE="$mat:$mat:100"
      for trace_iter in `seq 3`
      do
        export LD_PRELOAD=$EXP_ROOT/openmp/build_org/runtime/src/libomp.so
        export MKL_NUM_THREADS=1
        srun -n 2 --ntasks-per-node 2 -c 40 $HOST ./tester $app --origin h --target t --ref n --nb $NB --type $TYPE --lookahead 1 --dim $SIZE --repeat $ITER $GRID $TRACE --check n &> ${EXP_DATE}/log_org_${app}_${mat}_${trace_iter}
        mv *.svg ${EXP_DATE}/$app

        sleep 2
        export MKL_NUM_THREADS=1
        export LD_PRELOAD="$EXP_ROOT/hclib/hclib-install/lib/libhclib_omp.so"
        srun -n 2 --ntasks-per-node 2 -c 40 $HOST ./tester $app --origin h --target t --ref n --nb $NB --type $TYPE --lookahead 1 --dim $SIZE --repeat $ITER $GRID $TRACE --check n &> ${EXP_DATE}/log_hclib_${app}_${mat}_${trace_iter}
        mv *.svg ${EXP_DATE}/$app
      done
    done
  done
