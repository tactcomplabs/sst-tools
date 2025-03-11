#!/bin/bash -x

cleanup=$1

regex_list=()
# regex_list+=( 'GridNode[cp_0_1:finish:10000000]: cp_0_1 finish() clocks 10000 check 0xcc6800' )
# regex_list+=( 'CPTSubCompPairOfStructs[cp_0_1:CPTSubComp:finish:10000000]: finish() clocks 10000 check 0xb97d34ed' )
# regex_list+=( 'GridNode[cp_1_1:finish:10000000]: cp_1_1 finish() clocks 10000 check 0xcc6800' )
# regex_list+=( 'CPTSubCompPairOfStructs[cp_1_1:CPTSubComp:finish:10000000]: finish() clocks 10000 check 0x1f0aa967' )
# regex_list+=( 'GridNode[cp_0_0:finish:10000000]: cp_0_0 finish() clocks 10000 check 0xcc6800' )
# regex_list+=( 'CPTSubCompPairOfStructs[cp_0_0:CPTSubComp:finish:10000000]: finish() clocks 10000 check 0x8771baba' )
# regex_list+=( 'GridNode[cp_1_0:finish:10000000]: cp_1_0 finish() clocks 10000 check 0xcc6800' )
# regex_list+=( 'CPTSubCompPairOfStructs[cp_1_0:CPTSubComp:finish:10000000]: finish() clocks 10000 check 0xea51823d' )
# regex_list+=( 'Simulation is complete, simulated time: 10 us' )

check() {
    echo "### checking $1"
    for regex in "${regex_list[@]}"; do
        grep -qF "$regex" $1
        if [ $? != 0 ]; then
            echo "error: file $1 missing regex: ${regex}"
            return 1
        fi
    done
    return 0
}

pfx=2d_SAVE_PairOfStructs
logs=${pfx}_logs
rm -rf $pfx $logs
mkdir -p $logs

gridlib=$(realpath ../../build/sstcomp/grid)

echo "### creating checkpoints"
sst --checkpoint-prefix=2d_SAVE_PairOfStructs --num-threads=2 --checkpoint-period=1us \
    --add-lib-path=${gridlib} \
    2d.py -- --x=2 --y=2 --subcomp=grid.CPTSubCompPairOfStructs --verbose=2 > ${logs}/save.log
if [ $? != 0 ]; then
    echo "error: checkpoint save failed"
    exit 1
fi
check ${logs}/save.log
if [ $? != 0 ]; then
    echo "error: simulation check failed"
    exit 10
fi

for i in 0_1 1_2 2_3 3_4 4_5 5_6 6_7 7_8 8_9 9_10 10_11
do
    cpt=${pfx}_${i}000000
    echo "### loading checkpoint ${cpt}"
    sst --load-checkpoint ${pfx}/${cpt}/${cpt}.sstcpt \
        --num-threads=2 \
        --add-lib-path=${gridlib} > ${logs}/${i}.log
    check ${logs}/${i}.log
    if [ $? != 0 ]; then
        echo "error: restore check failed for ${cpt}"
        exit 11
    fi
done

if [ $? != 0 ]; then
    echo "error: checkpoint load failed"
    exit 2
fi

# optional clean
if [ "$cleanup" == "ON" ]; then
    echo "Cleaning " ${logs} ${pfx}
    rm -rf ${logs} ${pfx}
fi

# for ctest pass regexp
echo "Simulation is complete"
