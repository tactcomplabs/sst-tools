#!/bin/bash
#
# Derives the SST major version
# In SST 14.0.0; this script returns "14"
# For development branches (-dev); return "90" and above
#

ver=$(sst --version | awk '{print $3}' | tr -d '()')
if [ "$ver" == "-dev-schema" ]; then
    echo 90
elif [ "$ver" == "-dev" ]; then
    echo 99
else
    echo $ver | awk '{split($0,a,"."); print a[1]}'
fi

# EOF
