#!/bin/bash

# This script is used in conjuction with the makefile

$SST $SSTOPTS $SSTCFG &

if [[ ! -z ${PROBE_PORT} ]]; then
    echo PROBE_PORT=$PROBE_PORT
    sleep 2
    ../../../dbgcli-client.py << EOF
run
run
run
run
run
quit
EOF

fi

wait

