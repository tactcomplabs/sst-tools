#!/bin/bash
GRIDTEST_SPINNER=1 ../../scripts/sst-chkpt.sh 1 2d_SAVE_ --num-threads=2 --add-lib-path=../..//build/sstcomp/grid 2d.py --checkpoint-period=10ns -- --x=2 --y=2
