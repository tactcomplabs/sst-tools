#!/bin/bash
#
# Unregisters Rev from the SST infrastructure
#

#-- unregister it
sst-register -u cptsubcomp
sst-register -u dbgcli
sst-register -u grid

#-- forcible remove it from the local script
CONFIG=~/.sst/sstsimulator.conf
if test -f "$CONFIG"; then
  echo "Removing configuration from local config=$CONFIG"
  sed -i.bak '/cptsubcomp/d' $CONFIG
  sed -i.bak '/dbgcli/d' $CONFIG
  sed -i.bak '/grid/d' $CONFIG
fi

