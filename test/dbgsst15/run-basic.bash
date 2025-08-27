#!/bin/bash

# Flags to use ICDebugSST15 interactive console
sst --interactive-console=dbgsst15.ICDebugSST15 --interactive-start=0 dbgsst15.py

# This version uses --checkpoint-sim-period to enable debug checkpoint action
# It sets it for a value > sim run time (which is 10us in this case)
#sst --interactive-console=dbgsst15.ICDebugSST15 --interactive-start=0 --checkpoint-sim-period=12us dbgsst15.py
