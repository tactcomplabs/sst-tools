#!/bin/bash -x

# Synopsis
#   ctest driver script for tests restoring checkpoints.
# Usage
#   sst-restore {cleanup} {mpi-ranks} {cpt-prefix} {checkpoint-file} {rest of sst options}
# Purpose
#   This script provides mpirun prefix if needed

cleanup=$1
ranks=$2
pfx=$3
cpt=$4

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
        echo "Cleaning " ${pfx}
        rm -rf ${pfx}
    fi
else
    echo "Error: Test failed"
    exit 1
fi

#EOF

