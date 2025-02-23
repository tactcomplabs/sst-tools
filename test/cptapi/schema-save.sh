#!/bin/bash
#export GRID_SPINNER=1
export SST_SPINNER=1
#numThreads="--num-threads=2'
verbose="--verbose=10"

../../scripts/sst-chkpt.sh 1 2d_SAVE_ ${verbose} ${numThreads} --add-lib-path=../..//build/sstcomp/grid 2d.py --checkpoint-period=1us --gen-checkpoint-schema -- --x=2 --y=2


