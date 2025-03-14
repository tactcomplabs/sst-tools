#!/bin/bash

# Test break from code into interactive console via real time actions
#
# 0) Set up pipe
# 1) launch the program in the background
# 4) run commands in interactive consols
# 5) wait for completion
# 6) check results 

# Settings
CLOCK=1060
LIBPATH="../../sstcomp/igrid"


# 0) Set up the pipe
pipe=/tmp/testpipe
if [[ ! -p $pipe ]]; then
  echo "Creating pipe: $pipe"
  mkfifo $pipe
fi

# 1) Launch the program in the background, running long enough to send signal
if [[ -f test.igrid.out ]]; then
  rm test.igrid.out
fi

# --------------------------------------------------------------------------
# TEST 1: repeat run until simulation complete  
# ---------------------------------------------------------------------------
echo "-- Test 1 --" >> test.igrid.out
LAUNCH="sst --add-lib-path=$LIBPATH --interactive-console=sst.interactive.simpledebug igrid2d.py -- --clock $CLOCK --breakEnable"

echo $LAUNCH
echo $LAUNCH >> test.igrid.out
echo >> test.igrid.out

$LAUNCH < $pipe >> test.igrid.out &
exec 3>$pipe    # Opens pipe for writing
sleep 2


# 2) Send commands in interactive console
sleep 2
echo run > $pipe
echo run > $pipe
echo run > $pipe
echo run > $pipe

# 3) Wait for completion
wait
exec 3>&- # close pipe
echo InteractiveConsole Break Test 1 Complete
echo >> test.igrid.out

# 4) Check results
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "ERROR $sigusr=$action return code"
  exit $retVal
fi

# --------------------------------------------------------------------------
# TEST 2: clear breakEnable after first run
# ---------------------------------------------------------------------------
echo "-- Test 2 --" >> test.igrid.out
LAUNCH="sst --add-lib-path=$LIBPATH --interactive-console=sst.interactive.simpledebug igrid2d.py -- --clock $CLOCK --breakEnable"

echo $LAUNCH
echo $LAUNCH >> test.igrid.out
echo >> test.igrid.out

$LAUNCH < $pipe >> test.igrid.out &
exec 3>$pipe    # Opens pipe for writing
sleep 2


# 2) Send commands in interactive console
sleep 2
echo run > $pipe
echo cd cp0 > $pipe
echo set breakEnable 0 > $pipe
echo cd .. > $pipe
echo cd cp1 > $pipe
echo set breakEnable 0 > $pipe
echo run > $pipe

# 3) Wait for completion
wait
exec 3>&- # close pipe
echo InteractiveConsole Break Test 2 Complete
echo >> test.igrid.out

# 4) Check results
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "ERROR $sigusr=$action return code"
  exit $retVal
fi

# --------------------------------------------------------------------------
# TEST 3: Check that it disables break if interactive-console not specified  i
# ---------------------------------------------------------------------------i
echo "-- Test 3 --" >> test.igrid.out 
LAUNCH="sst --add-lib-path=$LIBPATH igrid2d.py -- --clock $CLOCK --breakEnable"

echo $LAUNCH
echo $LAUNCH >> test.igrid.out
echo >> test.igrid.out

$LAUNCH < $pipe >> test.igrid.out &
exec 3>$pipe    # Opens pipe for writing
sleep 2


# 2) Send commands in interactive console
sleep 2

# 3) Wait for completion
wait
exec 3>&- # close pipe
echo InteractiveConsole Break Test 3 Complete
echo >> test.igrid.out

# 4) Check results
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "ERROR $sigusr=$action return code"
  exit $retVal
fi

# --------------------------------------------------------------------------
# TEST 4: Check that it completes with no breakEnable 
# ---------------------------------------------------------------------------
echo "-- Test 4 --" >> test.igrid.out
LAUNCH="sst --add-lib-path=$LIBPATH igrid2d.py -- --clock $CLOCK"

echo $LAUNCH
echo $LAUNCH >> test.igrid.out
echo >> test.igrid.out

$LAUNCH < $pipe >> test.igrid.out &
exec 3>$pipe    # Opens pipe for writing
sleep 2


# 2) Send commands in interactive console
sleep 2

# 3) Wait for completion
wait
exec 3>&- # close pipe
echo InteractiveConsole Break Test 4 Complete
echo >> test.igrid.out

# 4) Check results
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "ERROR $sigusr=$action return code"
  exit $retVal
fi

# Diff results with reference file
$(diff test.igrid.ref test.igrid.out)
retVal=$?
if [ $retVal -ne 0 ]; then 
  echo "ERROR Output does not match reference"
  exit $retVal
fi
echo

echo "PASS"
exit $retVal

