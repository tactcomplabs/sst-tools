#!/bin/bash

echo "### checking run without subcomponent"

sst --num-threads=2 \
    --add-lib-path=/Users/kgriesser/work/sst-tools/build/sstcomp/grid \
    2d.py -- \
    --x=2 --y=2 --verbose=1

if [ $? != 0 ]; then
    echo "### run with no subcomponent failed"
    exit 1
fi

echo "### checking run with subcomponent"

sst --num-threads=2 \
    --add-lib-path=/Users/kgriesser/work/sst-tools/build/sstcomp/grid \
    2d.py -- \
    --x=2 --y=2 --verbose=1 \
    --subcomp=grid.CPTSubCompVecInt

if [ $? != 0 ]; then
    echo "run with subcomponent failed"
    exit 2
fi
