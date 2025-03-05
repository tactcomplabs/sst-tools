#!/bin/bash

# Synopsis
#   ctest driver script for tests restoring checkpoints.
# Usage
#   sst-restore {cleanup} {mpi-ranks} {checkpoint-file} {rest of sst options}
# Purpose
#   This script provides mpirun prefix if needed

cleanup=$1
ranks=$2
cpt=$3

if [ -d ${cpt} ]; then
    echo "WARNING: removing ${cpt}"
    rm -rf ${cpt}
fi

if (( $ranks > 1)); then
    mpipfx="mpirun -n ${ranks}"
fi

args=${@:4:$#}
cmd="${mpipfx} sst --load-checkpoint ${cpt} $args"
echo $cmd
eval "${cmd}"

if [ "$?" -eq 0 ]; then
    echo "Test passed"
    if [ "$cleanup" == "ON" ]; then
        echo "Cleaning " ${cpt}:r
        rm -rf ${cpt}:r
    fi
else
    echo "Error: Test failed"
    return 1
fi

#EOF

