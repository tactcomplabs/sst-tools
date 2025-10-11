#!/bin/bash

#
# Drive interactive mode to perform a parameter sweep
#

REPLAYFILE="sweep.in"
LOGFILE="sweep.log"
SSTOPTS="--interactive-start=0 --replay=$REPLAYFILE"
IMAGE_DATA=$(realpath "../../image_data")


sequence=(0.0001 0.0004 0.0008 0.0010 0.0015 0.0020 0.0025)
for LR in "${sequence[@]}"; do
    echo -n "### LR=${LR} "
    cat > $REPLAYFILE <<EOF
# navigate to optimizer
cd loss
cd optimizer
# modify the learning rate
set learning_rate_ $LR
p learning_rate_
# do not prompt to confirm clearing watchpoints
confirm false
unwatch
run
EOF

    cmd="sst nn.py ${SSTOPTS} -- \
        --classImageLimit=2000 \
        --batchSize=128 \
        --epochs=10 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayerSize=128 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=2"

    $cmd > $LOGFILE
    wait

    # get validation accuracy only if predictions are correct
    ACC=$(awk 'BEGIN {matches=0; acc=0} \
        /tshirt.+TOP/ {matches++} \
        /pants.+TROUSER/ {matches++} \
        /epoch 9 validation/ {acc=$5} \
        END {if (matches==2) print(acc); else print(0); }' $LOGFILE)

    if (( $(echo "$ACC > 0" | bc -l) )); then
        echo "ACC=${ACC}"
    else
        echo "mispredict"
    fi
done

