#!/bin/bash
mkdir -p run
cd run

# cliControl
# 0b0100_0000 : 0x40  Every checkpoint
# 0b0010_0000 : 0x20  Every checkpoint when probe is active
# 0b0001_0000 : 0x10  Every checkpoint sync state change
# 0b0000_0100 : 0x04  Every probe sample
# 0b0000_0010 : 0x02  Every probe sample from trigger onward
# 0b0000_0001 : 0x01  Every probe state change

sst --checkpoint-sim-period=1us ../dbgcli-sanity.py -- --probePort=10000 --cliControl=64  --probeStartCycle=3000000 --probeEndCycle=8000000 --probePostDelay=10 --probeBufferSize=1024 &
sleep 2
../dbgcli-client.py --probePort=10000<<EOF
echo hello there sst component
echo help
help
echo help clicontrol
help clicontrol
echo help component
help component
echo help cycle
help cycle
echo help disconnect
help disconnect
echo help dump
help dump
echo help echo
help echo
echo help hostname
help hostname
echo help numrecs
help numrecs
echo help probestate
help probestate
echo help run
help run
echo help spin
help spin
echo help syncstate
echo syncstate
echo help trigstate
echo trigstate
echo run
run
echo disconnect
disconnect
EOF

wait

