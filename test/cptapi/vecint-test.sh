#!/bin/bash

rm -rf 2d_SAVE_VecInt*

echo "### saving checkpoint"
sst --checkpoint-prefix=2d_SAVE_VecInt --num-threads=2 --add-lib-path=/Users/kgriesser/work/sst-tools/build/sstcomp/grid 2d.py --checkpoint-period=1us -- --x=2 --y=2 --subcomp=grid.CPTSubCompVecInt

if [ $? != 0 ]; then
    echo "checkpoint save failed"
    exit 1
fi

echo "### loading checkpoint"
sst --load-checkpoint 2d_SAVE_VecInt/2d_SAVE_VecInt_9_10000000/2d_SAVE_VecInt_9_10000000.sstcpt --num-threads=2 --add-lib-path=/Users/kgriesser/work/sst-tools/build/sstcomp/grid

if [ $? != 0 ]; then
    echo "checkpoint load failed"
    exit 2
fi
