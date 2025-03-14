#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# 2d.py
#

import argparse
import os
import sst

parser = argparse.ArgumentParser(description="2d grid network test 1 with checkpoint/restart checks")
parser.add_argument("--x", type=int, help="number of horizonal components", default=2)
parser.add_argument("--y", type=int, help="number of vertical components", default=1)
parser.add_argument("--numBytes", type=int, help="Internal state size (4 byte increments)", default=16384)
parser.add_argument("--minData", type=int, help="Minimum number of dwords transmitted per link", default=10)
parser.add_argument("--maxData", type=int, help="Maximum number of dwords transmitted per link", default=256)
parser.add_argument("--clocks", type=int, help="number of clocks to run sim", default=10000)
parser.add_argument("--minDelay", type=int, help="min number of clocks between transmissions", default=50)
parser.add_argument("--maxDelay", type=int, help="max number of clocks between transmissions", default=100)
parser.add_argument("--rngSeed", type=int, help="seed for random number generator", default=1223)
parser.add_argument("--demoBug", type=int, help="induce bug for debug demonstration", default=0)
parser.add_argument("--breakEnable", help="Enables breaks to interactive console from code (requires --interactive-console)", action='store_true', default=False)
parser.add_argument("--verbose", type=int, help="verbosity level", default=1)
args = parser.parse_args()

print("Interactive 2d grid test SST Simulation Configuration:")
for arg in vars(args):
  print("\t", arg, " = ", getattr(args, arg))

# the number of ports must always be 8 but may change this later
PORTS = 8
comp_params = {
  "verbose" : args.verbose,
  "numBytes" : args.numBytes,
  "numPorts" : PORTS,
  "minData" : args.minData,
  "maxData" : args.maxData,
  "minDelay" : args.minDelay,
  "maxDelay" : args.maxDelay,
  "clocks" : args.clocks,
  "rngSeed" : args.rngSeed,
  "clockFreq" : "1Ghz",
  "demoBug" : args.demoBug,
  "breakEnable" : args.breakEnable,
}

class IGRIDNODE():
  def __init__(self, x, y):
    id = f"cp_{x}_{y}"
    self.id = id
    self.comp = sst.Component(id, "igrid.IGridNode" )
    self.comp.addParams(comp_params)
    # everyone gets 8 links, up/down/left/right, send/rcv
    # links here are associated with this component's send ports
    self.upLink = sst.Link(f"upLink_{x}_{y}")
    self.downLink = sst.Link(f"downLink_{x}_{y}")
    self.leftLink = sst.Link(f"leftLink_{x}_{y}")
    self.rightLink = sst.Link(f"rightLink_{x}_{y}")
    # identify neighborhood
    self.neighbor = {}
    self.neighbor['u']  = f"cp_{x}_{(y+1)%args.y}"
    self.neighbor['d']  = f"cp_{x}_{(y-1)%args.y}"
    self.neighbor['l']  = f"cp_{(x-1)%args.x}_{y}"
    self.neighbor['r']  = f"cp_{(x+1)%args.x}_{y}"

if args.x==2 and args.y==1:
  # for known good check
  cp0 = sst.Component("cp0", "igrid.IGridNode")
  cp0.addParams(comp_params)
  cp1 = sst.Component("cp1", "igrid.IGridNode")
  cp1.addParams(comp_params)
  link = [None] * PORTS
  for i in range(0, PORTS):
      print(f"Creating link {i}")
      link[i] = sst.Link(f"link{i}")
      link[i].connect( (cp0, f"port{i}", "1us"), (cp1, f"port{i}", "1us") )
else:
  # create grid components
  grid = {}
  for x in range(args.x):
    for y in range(args.y):
      comp = IGRIDNODE(x,y)
      grid[comp.id] = comp

  # connect send ports to adjacent rcv ports. Edge nodes wrap around
  #  send: up=0, down=1, left=2, right=3
  #  rcv:  up=4, down=5, left=6, right=7
  for node in grid:
    print(f"Connecting {node}")
    tile = igrid[node]
    comp=tile.comp
    tile.upLink.connect(    (comp, f"port{0}", "1us"), (igrid[tile.neighbor['u']].comp, f"port{5}", "1us") )
    tile.downLink.connect(  (comp, f"port{1}", "1us"), (igrid[tile.neighbor['d']].comp, f"port{4}", "1us") )
    tile.leftLink.connect(  (comp, f"port{2}", "1us"), (igrid[tile.neighbor['l']].comp, f"port{7}", "1us") )
    tile.rightLink.connect( (comp, f"port{3}", "1us"), (grid[tile.neighbor['r']].comp, f"port{6}", "1us") )

# EOF
