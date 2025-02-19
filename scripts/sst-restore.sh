#!/bin/bash

# Synopsis
#   ctest driver script for tests restoring checkpoints.
# Usage
#   sst-restore {mpi-ranks} {checkpoint-file} {rest of sst options}
# Purpose
#   This script provides mpirun prefix if needed

ranks=$1
cpt=$2

if [ -d ${cpt} ]; then
    echo "WARNING: removing ${cpt}"
    rm -rf ${cpt}
fi

if (( $ranks > 1)); then
    mpipfx="mpirun -n ${ranks}"
fi

args=${@:3:$#}
cmd="${mpipfx} sst --load-checkpoint ${cpt} $args"
echo $cmd
eval "${cmd}"

#EOF

