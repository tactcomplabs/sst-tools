#!/bin/bash
#
# Derives the SST major version
# In SST 14.1.0; this script returns "1"
#

sst --version | awk '{print $3}' | tr -d '()' | awk '{split($0,a,"."); print a[2]}'

# EOF
