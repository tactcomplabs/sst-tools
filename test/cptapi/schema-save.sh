#!/bin/bash
#export GRID_SPINNER=1
#export SST_SPINNER=1
#numThreads="--num-threads=2'
#verbose="--verbose=10"

../../scripts/sst-chkpt.sh 1 2d_SAVE_ ${verbose} ${numThreads} --add-lib-path=../..//build/sstcomp/grid 2d.py --checkpoint-period=1us --gen-checkpoint-schema -- --x=1 --y=2

cpt=2d_SAVE__0_1000000
pushd 2d_SAVE_/${cpt} || exit 1
t=_0_0
hexdump -C ${cpt}${t}.bin > ${cpt}${t}.hex
cat ${cpt}${t}.json | c++filt -t >${cpt}${t}.schema.json
${HOME}/work/sst-tools/scripts/checkpoint_dump.py ${cpt}${t}.schema.json ${cpt}${t}.bin || exit 2
popd

echo schema-save.sh finished normally







