#!/bin/bash

# Test interactive console real time actions
#
# 0) Set up pipe
# 1) launch the program in the background
# 4) run commands in interactive consols
# 5) wait for completion
# 6) check results 

# Settings
CLOCK=400000
LIBPATH="../../sst-bench/igrid"
CONFIG="igrid2d.py"

# 0) Set up the pipe
pipe=/tmp/testpipe
#mkfifo $pipe
if [[ ! -p $pipe ]]; then
  echo "Creating pipe: $pipe"
  mkfifo $pipe
fi

# 1) Launch the program in the background, running long enough to send signal
if [[ -f test.interactive.out ]]; then
  rm test.interactive.out
fi

LAUNCH="sst --add-lib-path=$LIBPATH --interactive-console=sst.interactive.simpledebug --interactive-start=1us $CONFIG -- --clock $CLOCK"
echo $LAUNCH
$LAUNCH < $pipe > test.interactive.out &
exec 3>$pipe    # Opens pipe for writing
sleep 2


# 2) Send commands in interactive console
sleep 2
echo pwd > $pipe
echo ls > $pipe
echo cd cp0 > $pipe
echo ls > $pipe
echo pwd > $pipe
echo print my_info > $pipe
echo time > $pipe
echo set maxData 100 > $pipe
echo ls > $pipe
echo set maxData 256 > $pipe
echo ls > $pipe
echo run 1us > $pipe
echo run > $pipe

# 3) Wait for completion
wait
exec 3>&- # close pipe
echo InteractiveConsole Test Complete

# 4) Check results
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "ERROR $sigusr=$action return code"
  exit $retVal
fi

# Diff results with reference file
$(diff test.interactive.ref test.interactive.out)
retVal=$?
if [[ $retVal -ne 0 ]]; then 
  echo "ERROR Output does not match reference"
  exit $retVal
fi
echo

echo "PASS"
exit $retVal







