#!/bin/bash

#sst dbgsst15.py

#sst --interactive-console=sst.interactive.simpledebug --interactive-start=0 dbgsst15.py

#sst --interactive-console=dbgsst15.ICDebugSST15 --interactive-start=0 dbgsst15.py

#sst --checkpoint-sim-period=1us ../dbgcli-sanity.py -- --probePort=10000 --cliControl=64  --probeStartCycle=3000000 --probeEndCycle=8000000 --probePostDelay=10 --probeBufferSize=1024 &

#sst --checkpoint-sim-period=1us --interactive-console=dbgsst15.ICDebugSST15 --interactive-start=0 dbgsst15.py --  --cliControl=64 --probeStartCycle=3000000 --probeEndCycle=8000000 --probePostDelay=4 --probeBufferSize=128 

sst --checkpoint-sim-period=1us  dbgsst15.py -- --probePort=10000 --cliControl=64 --probeStartCycle=3000000 --probeEndCycle=8000000 --probePostDelay=4 --probeBufferSize=128

#--probeC0=1 --probeC1=1 

