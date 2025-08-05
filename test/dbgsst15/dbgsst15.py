#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# dbgsst15.py
#

import argparse
import sst

parser = argparse.ArgumentParser(description="debug probe demo")
parser.add_argument("--probeStartCycle", type=int, help="cycle to initiate debug probe. 0=Off", default=0)
parser.add_argument("--probeEndCycle", type=int, help="cycle to end debug probe. 0=Never", default=0)
parser.add_argument("--probeBufferSize", type=int, help="number of records in circular buffer", default=16)
parser.add_argument("--probePostDelay", type=int, help="number of events to capture after trigger event", default=8)
parser.add_argument("--probePort", type=int, help="sst probe starting socket. 0=None", default=0 )
parser.add_argument("--verbose", type=int, help="verbosity. 5=send/recv", default=1)
# 0b0100_0000 : 0x40 : 64 Every checkpoint
# 0b0010_0000 : 0x20 : 32 Every checkpoint when probe is active
# 0b0001_0000 : 0x10 : 16 Every checkpoint sync state change
# 0b0000_0100 : 0x04 : 04 Every probe sample
# 0b0000_0010 : 0x02 : 02 Every probe sample from trigger onward
# 0b0000_0001 : 0x01 : 01 Every probe state change,
parser.add_argument("--cliControl", type=int, help="event types on which to break into interactive mode"
" [64 Every checkpoint]"
" [32 Every checkpoint when probe is active]"
" [16 Every checkpoint sync state change]"
" [04 Every probe sample]"
" [02 Every probe sample from trigger onward]"
" [01 Every probe state change]", default=0)

args = parser.parse_args()
print("debug probe demo configuration:")
for arg in vars(args):
  print("\t", arg, " = ", getattr(args, arg))

# Component custom trace controls
TRACE_SEND = 1
TRACE_RECV = 2

MIN_DATA = 1
MAX_DATA = 100

cp0 = sst.Component("cp0", "dbgsst15.DbgSST15")
cp0.addParams({
  "verbose" : args.verbose,
  "numPorts" : 1,
  "minData" : MIN_DATA,
  "maxData" : MAX_DATA,
  "clockDelay" : 100,
  "clocks" : 10000,
  "rngSeed" : 1223,
  "clockFreq" : "1Ghz",
  # common probe controls
  #"probeMode" : 1,
  "probeStartCycle" : args.probeStartCycle,
  "probeEndCycle"   : args.probeEndCycle,
  "probeBufferSize" : args.probeBufferSize,
  "probePostDelay"  : args.probePostDelay,
  "probePort"       : args.probePort,
  "cliControl"      : args.cliControl,
  # component specific probe controls
  "traceMode"       : TRACE_RECV,
})

cp1 = sst.Component("cp1", "dbgsst15.DbgSST15")
cp1.addParams({
  "verbose" : args.verbose,
  "numPorts" : 1,
  "minData" : MIN_DATA,
  "maxData" : MAX_DATA,
  "clockDelay" : 100,
  "clocks" : 10000,
  "rngSeed" : 1223,
  "clockFreq" : "1Ghz",
  # common probe controls
  "probeMode" : 1,
  "probeStartCycle" : args.probeStartCycle,
  "probeEndCycle"   : args.probeEndCycle,
  "probeBufferSize" : args.probeBufferSize,
  "probePostDelay"  : args.probePostDelay,
   #"probePort" : PROBE_PORT+1,
   #"cliControl"     : CLI_CONTROL,
   # component specific probe controls
   "traceMode"      : TRACE_SEND,
})

link0 = sst.Link("link0")
link0.connect( (cp0, "port0", "1us"), (cp1, "port0", "1us") )

# EOF
