#!/bin/bash

file_prefix=$(bash -c 'echo $SLURM_TASK_PID')
nsys profile -o result_${file_prefix}.nvvp --trace=mpi,osrt --mpi-impl=openmpi --sample=cpu ./tester potrf --origin h --target t  --ref n --nb 192 --type d --lookahead 1 --dim 10000:10000:5000 --repeat 3  --p 1 --q 2
