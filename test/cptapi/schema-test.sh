#!/bin/bash

#export GRID_SPINNER=1
#export SST_SPINNER=1
LIBGRID=$(realpath ../../build/sstcomp/grid)
SCRIPTS=$(realpath ../../SCRIPTS)

# Check version
version=$($SCRIPTS/sst-major-version.sh)
if [[ "$version" != "90" ]]; then
    echo "error: schema-test currently only works with sst version -dev-schema"
    exit 1
fi

# generate checkpoints and json files from sst
${SCRIPTS}/sst-chkpt.sh 1 SCHEMA_SAVE_ --verbose=1 --num-threads=4 --add-lib-path=${LIBGRID} 2d.py --checkpoint-period=1us --gen-checkpoint-schema -- --x=2 --y=2

# run checkpoint json files through c++filt
for j in $(find SCHEMA_SAVE_ -name '*.json')
do
    (cd $(dirname $j); cat $(basename $j) | c++filt -t > $(basename -s json $j)schema.json)
done

# generate hex dumps for each binary
for j in $(find SCHEMA_SAVE_ -name '*.bin')
do
    (cd $(dirname $j);  hexdump -C $(basename $j) > $(basename -s bin $j)hex )
done

# Independently load and verify the checkpoint files.
export PYTHONPATH="${SCRIPTS}:$PYTHONPATH"
cpt_verify.py  || exit 1

echo schema-test.sh finished normally
