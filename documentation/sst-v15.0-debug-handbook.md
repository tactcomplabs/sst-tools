# SST v15.0 Debug Handbook

**Prepared By:**

**Shannon Kuntz**

skuntz@tactcomplabs.com

**Version:**

**1.0**

**Release Date:**

**Changelog**

| Date | Author | Change |
| --- | --- | --- |
| 2025/03/01 | S. Kuntz | Initial draft |


**Table of Contents**
- [SST v15.0 Debug Handbook](#sst-v150-debug-handbook)
  - [Live Debug Documentation](#live-debug-documentation)
  - [Signals and RealTime Actions](#signals-and-realtime-actions)
    - [RealTime Actions](#realtime-actions)
    - [Signal Handler CLI](#signal-handler-cli)
    - [Sending Signals from a Terminal](#sending-signals-from-a-terminal)
    - [Example Usage](#example-usage)
      - [Exit Simulation (sst.rt.exit.clean)](#exit-simulation-sstrtexitclean)
      - [Exit Simulation with Error (sst.rt.exit.emergency)](#exit-simulation-with-error-sstrtexitemergency)
      - [SST-Core Status (sst.rt.status.core)](#sst-core-status-sstrtstatuscore)
      - [Simulation Status (sst.rt.status.all)](#simulation-status-sstrtstatusall)
      - [Heartbeat (sst.rt.heartbeat)](#heartbeat-sstrtheartbeat)
      - [Checkpoint (sst.rt.checkpoint)](#checkpoint-sstrtcheckpoint)
      - [Interactive Console (sst.rt.interactive)](#interactive-console-sstrtinteractive)
    - [Custom Real Time Actions](#custom-real-time-actions)
    - [Known Issues](#known-issues)
      - [OpenMPI mpirun](#openmpi-mpirun)
      - [Combining Checkpoint/Restart with Actions](#combining-checkpointrestart-with-actions)
      - [Disable Signal Handling](#disable-signal-handling)
  - [Simple Debug Prototype Interactive Console (sst.interactive.simpledebug)](#simple-debug-prototype-interactive-console-sstinteractivesimpledebug)
    - [Debug Commands](#debug-commands)
    - [Component Modifications to Enable Interactive Console Debugging](#component-modifications-to-enable-interactive-console-debugging)
    - [Example SimpleDebug Session](#example-simpledebug-session)
  - [Custom Interactive Console](#custom-interactive-console)
    - [Available Features](#available-features)
    - [Component Modifications for Custom Interactive Console Support](#component-modifications-for-custom-interactive-console-support)
  - [Custom Component-level Debug Support](#custom-component-level-debug-support)
  - [Watchpoints](#watchpoints)
  - [Profiling](#profiling)
    - [Profiling CLI](#profiling-cli)
    - [Available Profiling Tools](#available-profiling-tools)
      - [Component Code Segment Profile Tool](#component-code-segment-profile-tool)
        - [Code Segment Count (profile.component.codesegment.count)](#code-segment-count-profilecomponentcodesegmentcount)
        - [Code Segment High Resolution Time (profile.component.codesegment.time.high\_resolution)](#code-segment-high-resolution-time-profilecomponentcodesegmenttimehigh_resolution)
        - [Code Segment Steady Time (profile.component.codesegment.time.steady)](#code-segment-steady-time-profilecomponentcodesegmenttimesteady)
      - [Clock Handler Profile Tool](#clock-handler-profile-tool)
        - [Clock Handler Call Count (profile.handler.clock.count)](#clock-handler-call-count-profilehandlerclockcount)
        - [Clock Handler High Resolution Time (profile.handler.clock.time.high\_resolution)](#clock-handler-high-resolution-time-profilehandlerclocktimehigh_resolution)
        - [Clock Handler Steady Time (profile.handler.clock.time.steady)](#clock-handler-steady-time-profilehandlerclocktimesteady)
      - [Event Hander Profile Tool](#event-hander-profile-tool)
        - [Event Handler Count (profile.handler.event.count)](#event-handler-count-profilehandlereventcount)
        - [Event Hander High Resolution Time (profile.handler.event.time.high\_resolution)](#event-hander-high-resolution-time-profilehandlereventtimehigh_resolution)
        - [Event Handler Steady Time (profile.handler.event.time.steady)](#event-handler-steady-time-profilehandlereventtimesteady)
      - [Sync Profile Tool](#sync-profile-tool)
        - [Sync Count (profile.sync.count)](#sync-count-profilesynccount)
        - [Sync High Resolution Time (profile.sync.time.high\_resolution)](#sync-high-resolution-time-profilesynctimehigh_resolution)
        - [Sync Steady Time (profile.sync.time.steady)](#sync-steady-time-profilesynctimesteady)
    - [Custom Profile Tools](#custom-profile-tools)
  - [Data Serialization](#data-serialization)
  - [Checkpoint and Restart](#checkpoint-and-restart)


## Live Debug Documentation

SST provides many different levels of debugging support, from printing
built-in heartbeat information to exploring simulation state with
standard commands from an interactive console to customizing components
to provide targeted debug information. This document walks through the
current debugging support in SST 15.0 along with simple examples of how
to use it and any coding or usage issues to be aware of.

*Live debug provides mechanisms to:*

1.  *Control pausing the simulation.*

2.  *View simulation debug data at various levels of resolution.*

3.  *Modify the simulator state.*

4.  *Resume the simulation.*

*To provide maximum flexibility, multiple debug strategies will be
employed that use a general solution built on serialized checkpoint
data:*

1.  *Provide component level instrumentation for more precise control.*

2.  *Standardize APIs and data formats to allow extending
    functionality.*

*The resolution, or size, of debug data will be controlled by providing
a variety of triggering mechanisms based on:*

- *A periodic simulation clock*

- *A periodic wall clock*

- *Customizable events*

*Further options to manage the size of the debug data will include the
ability to select between the serialized data object (representing the
full simulator start) and/or component-level data captured through
instrumentation.*

*Finally, interactive debug modes will provide the ability to pause the
simulation at specific events and interact with the design via an
extensible command language interface before resuming the simulation.*

## Signals and RealTime Actions

SST 15.0 provides command line options to trigger standard debugging
features via signals. Signals provide the ability to interrupt an SST
simulation at a safe \'global synchronization\' point and execute
special handlers. Common applications may be to print debug information,
save execution state, or enter an interactive debug mode.

SST handles the SIGINT, SIGTERM, SIGUSR1, and SIGUSR2 signals and
uses SIGALRM to generate periodic actions that occur on a timer based on
simulation or wall time. Actions are defined based on
the RealTimeAction class. SST provides default actions for each of the
signals and allows users to override the default with custom actions
for SIGUSR1, SIGUSR2, and SIGALRM.

Refer to the SST documentation for additional information.

Signal Handling:
[[https://sst-simulator.org/sst-docs/docs/guides/features/signal]](https://sst-simulator.org/sst-docs/docs/guides/features/signal)

Real Time Actions:
[[https://sst-simulator.org/sst-docs/docs/core/realtime/class]](https://sst-simulator.org/sst-docs/docs/core/realtime/class)

### RealTime Actions

RealTimeAction objects are actions that execute in response to a signal.
SST provides several actions and element libraries may additionally
provide their own custom actions. The actions provided by SST 15.0 are:

- **Exit simulation** (sst.rt.exit.clean) - Simulation exits SST\'s run
  stage and proceeds to the complete stage.

- **Exit simulation with error** (sst.rt.exit.emergency) - Simulation
  exits immediately with an error and does not attempt to finish the
  simulation.

- **SST-Core Status** (sst.rt.status.core) - SST-Core reports the
  simulated time.

- **Simulation Status** (sst.rt.status.all) - SST-Core reports the
  simulated time
  and [printStatus()](https://sst-simulator.org/sst-docs/docs/core/component/lifecycle/printStatus) is
  called on every Component in the simulation.

- **Heartbeat** (sst.rt.heartbeat) - SST-Core reports the simulated time
  and some profiling metrics (size of memory pools, time in
  synchronization, etc.)

- **Checkpoint** (sst.rt.checkpoint) - Creates a checkpoint.

- **Interactive Console** (sst.rt.interactive) -- \[Experimental\]
  Breaks into an interactive console which allows the user to navigate
  the simulation model and explore simulation state.

Table 1 lists the supported Signals and RealTimeActions. For each signal
the default action is shown along with its description as well as the
command line options that are available to override the default action
and define a new associated action.

**Table 1: Signal Actions**
| **Signal** | **Default** | **Description**  | **Command line control**   |
| --- | --- | --- | --- |
| SIGINT     | sst.rt.exit.emergency | Exit simulation immediately with an error     | None                      |
| SIGTERM    | sst.rt.exit.emergency | Exit simulation immediately with an error    | None                      |
| SIGUSR1    | sst.rt.status.core    | Report status of SST-Core   | \--sigusr1                |
| SIGUSR2    | sst.rt.status.all     | Report status of SST-Core and all components   | \--sigusr2                |
| SIGALRM    | \-                    | \-                  | \--sigalrm <br>\--checkpoint-wall-period<br>\--heartbeat-wall-period<br> \--exit-after |

When SST receives a signal in a serial simulation, it executes the
associated RealTimeAction as soon as the currently executing event or
clock handler returns control to SST-Core. Actions in parallel
simulations are deferred until SST-Core\'s next synchronization point.
Therefore, actions may perform global read and synchronization
operations.

Signals are only detected and handled during SST\'s [run
phase](https://sst-simulator.org/sst-docs/docs/guides/concepts/lifecycle).
Signals sent earlier may be missed. Signals sent later will be ignored.

### Signal Handler CLI

\--sigusr1=MODULE HS Select handler for SIGUSR1 signal. See extended
help for detail.

\--sigusr2=MODULE HS Select handler for SIGUSR2 signal. See extended
help for detail.

\--sigalrm=MODULE HS Select handler for SIGALRM signals. Argument is a
semicolon

> separated list specifying the handlers to register along with a time interval for each. See extended help for detail.

`sst --help sigusr1`

RealTime Actions \[EXPERIMENTAL\]:

RealTimeActions are actions that execute in response to system signals
SIGUSR1, SIGUSR2, and/or SIGALRM. The following actions are available
from SST core or custom actions may also be defined.

● **sst.rt.exit.clean**: Exits SST normally.

● **sst.rt.exit.emergency**: Exits SST in an emergency state. Triggered
on SIGINT and SIGTERM.

● **sst.rt.status.core**: Reports brief state of SST core.

● **sst.rt.status.all**: Reports state of SST core and every simulated
component.

● **sst.rt.checkpoint**: Creates a checkpoint.

● **sst.rt.heartbeat**: Reports state of SST core and some profiling
state (e.g., memory usage).

● **sst.rt.interactive**: Breaks into interactive console to explore
simulation state. Ignored if \--interactive-console flag not used.
(Valid for SIGUSR1/2 only, invalid for SIGALRM)

An action can be attached to SIGUSR1 using \'\--sigusr1=\<handler\>\'
and SIGUSR2 using

\'\--sigusr2=\<handler\>\'

If not specified SST uses the defaults:

\--sigusr1=sst.rt.status.core and \--sigusr2=sst.rt.status.all.

Actions can be bound to SIGALRM by specifying
\'\--sigalrm=ACTION(interval=TIME)\' where ACTION is the action and TIME
is a wall-clock time in the format HH:MM:SS, MM:SS, SS, Hh, Mm, or Ss.
Capital letters represent numerics and lower case are units and required
for those formats. Multiple actions can be separated by semicolons or
multiple instances of \--sigalrm can be used.

Examples:

\--sigusr1=sst.rt.checkpoint

\--sigusr2=sst.rt.heartbeat

\--sigalrm=\"sst.rt.checkpoint(interval=2h);sst.rt.heartbeat(interval=30m)\"

Note that sst also provides a \--disable-signal-handlers flag. In SST
15.0, the behavior varies by signal as follows:

- **SIGKILL (kill -9)**: Unaffected by \--disable-signal-handlers
  (always processed)

- **SIGINT and SIGTERM**: First signal sent is ignored. If the signal is
  sent a second time it is processed.

- **SIGUSR1 and SIGUSR2**: Prints a message and exits the simulation.

- **SIGALRM**: Specified periodic action is ignored. This also applies
  to custom periodic action command-line options built on sigalrm such
  as \--heartbeat-wall-period, \--checkpoint-wall-period, and
  \--exit-after.

This behavior will likely change in future versions.

### Sending Signals from a Terminal

The kill command is used to send a signal to SST from a terminal:

```kill -s <SIGNAL> <PID>```

e.g.

```kill -s SIGUSR1 3348765```

SIGINT can also be sent from a terminal using Ctrl-C.

### Example Usage

#### Exit Simulation (sst.rt.exit.clean)

When executing a clean exit, the simulation exits SST\'s run stage and
proceeds to the complete stage to clean up the simulation before exit.
It can be invoked using \--sigusr1, \--sigusr2, or \--sigalrm as
described in Section 2.2.

Example:

```sst --sigalrm="sst.rt.exit.clean(interval=1s)" test_MessageMesh.py```

Sample Output:

```
EXIT-AFTER TIME REACHED; SHUTDOWN (0,0)!
# Simulated time: 163.548 us
```

This functionality enables the user to cleanly exit the simulation early
using a signal (unlike Ctrl-C which causes an emergency exit). Note that
using the clean exit action with sigalrm schedules a clean exit at the
given time and is equivalent to the \--exit-after command line option.

\--exit-after=TIME

> Set the maximum wall time after which simulation will end execution.
> Time is specified in hours, minutes and seconds, with the following
> formats supported: H:M:S, M:S, S, Hh, Mm, Ss (capital letters are the
> appropriate numbers for that value, lower case letters represent the
> units and are required for those formats). **Note** that this is
> implemented using SIGALRM and, as such, will be disabled when the
> \--disable-signal-handlers flag is used.

#### Exit Simulation with Error (sst.rt.exit.emergency)

The emergency exit action exits the simulation immediately with an error
and does not attempt to clean up. This is the default behavior for both
the SIGINT (Ctrl-C) and SIGTERM signals. It can also be invoked
explicitly using \--sigusr1, \--sigusr2, or \--sigalrm as described in
Section 2.2.

Example:

```sst --sigusr2=sst.rt.exit.emergency test_MessageMesh.py```

Sample Output:

```
EMERGENCY SHUTDOWN Complete (0,0)!
```

#### SST-Core Status (sst.rt.status.core)

The status action reports the simulated time and is the default behavior
for SIGUSR1. It can also be invoked explicitly using \--sigusr1,
\--sigusr2, or \--sigalrm as described in Section 2.2.

Example:

```sst --sigalrm2="sst.rt.status.core(interval=1s)" test_MessageMesh.py```

Sample Output:
```
CurrentSimCycle: 161686000
CurrentSimCycle: 323177000
CurrentSimCycle: 486247000
CurrentSimCycle: 648164000
```
Periodically printing the simulation time using sigalrm is useful to
show progress of a long-running simulation that does not generate
output, without having to modify the code. Similarly, printing the
simulation time via sigusr1/2 allows the user to query the current
simulation time for a long-running simulation to evaluate progress.

#### Simulation Status (sst.rt.status.all)

SST-Core reports the simulated time and printStatus() is called on every
Component in the simulation. This is the default behavior for SIGUSR2.
It can also be invoked explicitly using --sigusr1, --sigusr2, or
--sigalrm as described in Section 2.2.

Example:

```sst --sigalrm="sst.rt.status.all(interval=1s)" test_MessageMesh.py```

Sample Output:
```
CurrentSimCycle: 161822000
TimeVortex state: cannot iterate priority_queue
---- Components: ----
CurrentSimCycle: 324782000
TimeVortex state: cannot iterate priority_queue
---- Components: ----
CurrentSimCycle: 486659000
TimeVortex state: cannot iterate priority_queue
---- Components: ----
CurrentSimCycle: 648612000
TimeVortex state: cannot iterate priority_queue
---- Components: ----
```

Note that this action calls the printStatus() function on Components
only. Components, SubComponents, and ComponentExtensions must in turn
manually call printStatus() on their SubComponents and
ComponentExtensions. This allows developers to control both whether
those objects participate in printStatus and the order in which they
participate. The level of debug information provided by printStatus() in
sst-elements components varies significantly. Users developing their own
custom components can define the debug information they wish to see in
printStatus().

See SST-Core documentation for printStatus here:
[https://sst-simulator.org/sst-docs/docs/core/component/lifecycle/printStatus](https://sst-simulator.org/sst-docs/docs/core/component/lifecycle/printStatus)

#### Heartbeat (sst.rt.heartbeat)

The heartbeat feature reports simulation time and state information
including: the simulated time, the real CPU time since the last
heartbeat, the maximum memory pool usage, the global memory pool usage,
the number of globally active activities, the maximum depth of the time
vortex, and the maximum data size for global synchronization. It can be
invoked using \--sigusr1, \--sigusr2, or \--sigalrm as described in
Section 2.2.

Example:

```sst --sigalrm="sst.rt.heartbeat(interval=1s)" test_MessageMesh.py```

Sample Output:
```
# Simulation Heartbeat: Simulated Time 682 us (Real CPU time since last
period 0.00638 seconds)
Max mempool usage: 11.7965 MB
Global mempool usage: 11.7965 MB
Global active activities 70 activities
Max TimeVortex depth: 67 entries
Max Sync data size: 0 B
Global Sync data size: 0 B
```
The heartbeat information is particularly useful in understanding how
memory usage varies during simulation execution and to monitor for
issues with memory pool or synchronization data size.

There are also specialized command-line options available to generate
the heartbeat:

\--heartbeat-wall-period=PERIOD

> Set approximate frequency for heartbeats (SST-Core progress updates)
> to be published in terms of wall (real) time. PERIOD can be specified
> in hours, minutes, and seconds with the following formats supported:
> H:M:S, M:S, S, Hh, Mm, Ss (capital letters are the appropriate numbers
> for that value, lower case letters represent the units and are
> required for those formats). **Note** that this is implemented using
> SIGALRM and, as such, will be disabled when the
> \--disable-signal-handlers flag is used.

\--heartbeat-sim-period=PERIOD

> Set approximate frequency for heartbeats (SST-Core progress updates)
> to be published in terms of simulated time. PERIOD must include time
> units (s or Hz) and SI prefixes are accepted (e.g. us, ms).

\--heartbeat-period=PERIOD

> Set time for heartbeats to be published (these are approximate timings
> measured in simulation time, published by the core, to update on
> progress). This flag will eventually be removed in favor of
> \--heartbeat-sim-period

Note that in SST 15.0, heartbeat actions do not always work correctly
with checkpoint/restart. See Section 2.6.2 for additional information on
this issue.

#### Checkpoint (sst.rt.checkpoint) 

Checkpoints provide full simulation state dumps which can be triggered
on periodic simulation clock events, wall-clock events, or custom
software interrupts (signals). The serialized data can be reloaded to
restart the simulation from the checkpoint. Checkpointing can be invoked
using \--sigusr1, \--sigusr2, or \--sigalrm as described in Section 2.2.

A command-line option is also provided to specify the prefix for the
checkpoint file names.

\--checkpoint-prefix=PREFIX

> Set prefix for checkpoint filenames. The checkpoint prefix defaults to
> checkpoint if this option is not set and checkpointing is enabled.

Checkpointing can also be specified using one of the following
specialized command-line options:

\--checkpoint-wall-period=PERIOD

> Set approximate frequency for checkpoints to be generated in terms of
> lk, (real) time. PERIOD can be specified in hours, minutes, and
> seconds with the following formats supported: H:M:S, M:S, S, Hh, Mm,
> Ss (capital letters are the appropriate numbers for that value, lower
> case letters represent the units and are required for those formats).
> **Note** that this is implemented using SIGALRM and, as such, will be
> disabled when the \--disable-signal-handlers flag is used.

\--checkpoint-sim-period=PERIOD

> Set approximate frequency for checkpoints to be generated in terms of
> simulated time. PERIOD must include time units (s or Hz) and SI
> prefixes are accepted.

\--checkpoint-period=PERIOD

> Set approximate frequency for checkpoints to be generated in terms of
> simulated time. PERIOD must include time units (s or Hz) and SI
> prefixes are accepted. This flag will eventually be removed in favor
> of \--checkpoint-sim-period.

Example:

```sst --checkpoint-sim-period=1s test_Checkpoint.py```

Sample Output:
```
# Creating simulation checkpoint at simulated time period of 1s.
...
# Simulation Checkpoint: Simulated Time 1 s (Real CPU time since last
checkpoint 1.07128 seconds)
...
# Simulation Checkpoint: Simulated Time 2 s (Real CPU time since last
checkpoint 1.13572 seconds)
```
See Section REF (or separate document)? for details of serialization and
checkpointing. I think Gwen is working on release documentation.

#### Interactive Console (sst.rt.interactive)

The interactive action triggers a break to an interactive console that
can be used to query and modify the state of the simulation for
debugging purposes. The interactive console can be triggered using
\--sigusr1, \--sigusr2, or \--sigalrm as described in Section 2.2.

The console to use when the interactive console action is triggered must
be specified with the following command-line option.

\--interactive-console=ACTION

> \[EXPERIMENTAL\] Set console to use for interactive mode. NOTE: This
> currently only works for serial jobs and this option will be ignored
> for parallel runs.

SST 15.0 provides a prototype interactive console implementation,
simpledebug, to provide basic debug functionality.

Example:

```
sst --interactive-console=sst.interactive.simpledebug
--sigusr1=sst.rt.interactive test_MessageMesh.py
```

Sample Output:
```
1 us
Entering interactive mode at time 1000000
Interactive start at 1000000
\>
```

The interactive console can also be invoked using the following
specialized command line option:

\--interactive-start=\[TIME\]

> \[EXPERIMENTAL\] Drop into interactive mode at specified simulated
> time. If no time is specified, or the time is 0, then it will drop
> into interactive mode before any events are processed in the main run
> loop. This option is ignored if no interactive console was specified.
> NOTE: This currently only works for serial jobs and this option will
> be ignored for parallel runs.

The interactive console simple debug prototype provides basic debugging
functionality including the ability to navigate the current simulation
object map, print simulation state, modify simulation variables, manage
watch points (which can be used to trigger the interactive console when
a variable value matches a trigger condition), continue execution of the
simulation, and cleanly shutdown the simulation. This functionality is
built on the serialization object map and, as such, the component code
requires no modification as long as the data of interest has been
serialized (which should be the case with any codes that support
checkpointing, including most sst-elements components).

Refer to Section 3 for details of the simple debug interactive console
prototype and Section 4 for a discussion of custom interactive console
development. See SST-Core documentation here:
<https://sst-simulator.org/sst-docs/docs/guides/concepts/elementtypes#interactiveconsole>

Note that in SST 15.0, interactive console actions do not always work
correctly with checkpoint/restart. See Section 2.6.2 for additional
information on this issue.

### Custom Real Time Actions

SST provides support for user-defined actions that can be triggered via
signals. Similar to implementing user-defined components which inherit
from the Component class, a custom real time action inherits from the
RealTimeAction class and must be registered with SST.

The sst-tools repository provides an example user-defined real time
action, TestRTAction, in sst-tools/sstcomp/icdbg. The TestRTAction class
below shows the key components of the user-defined real time action and
is compiled into the icdbg library.

```
namespace SST::ICDbg{
class TestRTAction : public SST::RealTimeAction
{
public:
  SST_ELI_REGISTER_REALTIMEACTION(
  TestRTAction,
  "icdbg",
  "testrtaction",
  SST_ELI_ELEMENT_VERSION(1, 0, 0),
  "{EXPERIMENTAL} Test user-defined realtime action for use with
  signals")
  TestRTAction();
  ~TestRTAction() {}
  void execute() override;
};
} // namespace SST::ICDbg
```

Note that the registration format is as follows:

- Class Name: TestRTAction

- Library Name: icdbg

- Real Time Action Name: testrtaction (does not need to match class
  name)

- Element Version: 1.0.0

- Description

When invoking sst with a real time action, the registered interactive
console is specified as \<LibraryName\>.\<RealTimeActionName\>, e.g.

`sst --sigusr1=icdbg.testrtaction test.py`

The element version is used to assist library developers with
versioning. It is not checked by SST and does not need to match the SST
version.

### Known Issues

#### OpenMPI mpirun

When running SST with OpenMPI, signals sent to the mpirun process are
not always handled as expected. This can be avoided by sending the
signal directly to one of the SST ranks.

How to do this?

When sent to mpirun:

- SIGUSR1 and SIGUSR2 signals may be duplicated in SST.

- SIGINT and SIGTERM are not always delivered to the SST processes so
  emergency shutdown may not occur before SST is terminated.

#### Combining Checkpoint/Restart with Actions

Actions are not intended to be checkpointed and any actions in use
during checkpointing will not be saved in the checkpoint file. However,
SST does support using actions when restarting with a checkpointed file.
So, for example, you could run a simulation with both the core status
and checkpoint actions enabled. The core status actions would not be
checkpointed, so when restarting with the checkpointed file the status
would no longer be printed unless you specify the core status action on
the command line during restart e.g.

`sst --sigalrm="sst.rt.core.status(interval=1s)" --load-checkpoint
<ckpt_path>`

There are two known issues using checkpointing with other actions in SST
15.0. Using heartbeat with checkpoint may cause an error during
simulation restart with the checkpointed file. Using the interactive
console action with checkpoint may cause errors during the checkpoint
process. Modifications are currently under development to resolve these
issues.

------------------------------------------------------------------------

#### Disable Signal Handling

Disable signal handling does not work correctly with SIGUSR1 or SIGUSR2.
Instead of ignoring the signal, it prints a message that the signal
occurred and exits the simulation.

## Simple Debug Prototype Interactive Console (sst.interactive.simpledebug)

A simple interactive debug console prototype
(sst.interactive.simpledebug) is provided with SST 15.0 that allows you
to navigate, inspect, and modify the state of the simulation during
execution for 'live' debug. It uses serialization to create an object
map of the simulation model in order to navigate the objects in the
simulation from the console. As discussed in Section 2.4.7, command-line
options can be used to instruct the simulator to break into interactive
mode at a specified simulation time or in response to a signal.

Once in the simple debug interactive console, the user can navigate the
current simulation model object map to focus on objects of interest,
print current simulation state information, modify simulation data,
manage watch points, execute the simulation for a specified duration
before returning to the interactive console, or exit the interactive
console and complete simulation. Note that the interactive console is
currently only supported for serial jobs and will be ignored for
parallel execution.

### Debug Commands

The simple debug prototype demonstrate the following features of an
interactive console:

- Navigate the current simulation model object map

  - **pwd**: Print the current working directory in the object map

  - **cd**: Change directory level in the object map

  - **ls**: List objects in current level of the object map directory

- Print current simulation information

  - **time**: Print current simulation time in cycles

  - **print \[-rN\]\[\<obj\>\]**: Print objects in the current level of
    the object map. If -rN is provided it prints recursively N levels,
    the default is to print 4 levels

- Modify simulation variables

  - **set \<obj\> \<value\>**: Sets an object in the current scope to
    the provided value. The object must be a "fundamental type" e.g. int
    (it must also be marked as volatile?)

- Manage watch points which trigger the interactive console when a
  variable value matches the trigger condition

  - **watch**: Prints the current list of watch points and their
    associated indices

  - **watch \<var\>**: Adds the specified variable to the watch list.
    Interactive console is triggered when the variable's value changes

  - **watch \<var\> \<comp\> \<val\>**: Adds the specified variable to
    the watch list along with a comparison operation and reference
    value. The interactive console is triggered when the variable's
    comparison with \<val\> is true. Valid \<comp\> operators include:
    \<, \<=, \>, \>=, ==, !=

  - **unwatch \<index\>**: Removes the indexed watch point from the
    watch list where \<index\> is the associated index from the printed
    list of watch points

- Execute the simulation for a specified duration

  - **run \[TIME\]**: runs the simulation from the current point for
    TIME and then returns to interactive mode; if no time is given, the
    simulation runs to completion

- Exit the interactive console and complete the simulation

  - **quit**

  - **exit**

The following commands have been added to the sst-core development
branch since version 15.0:

- Help

  - **help**: display the help menu

- Shutdown

  - **shutdown**: exits the interactive console and does a clean
    shutdown of the simulation

### Component Modifications to Enable Interactive Console Debugging

In order to support debugging, the component developer must serialize
the data of interest in each component using the same technique used to
serialize data for checkpointing. However for debugging purposes, only
the data of interest need be serialized whereas for checkpointing all
data needed to support simulation restart must be serialized.

In the following code snippet, a simple cpu component is serialized to
expose

### Example SimpleDebug Session

## Custom Interactive Console

SST supports user-defined interactive consoles to customize what
commands are available. A custom interactive console could be used to
provide a more targeted live debug for a particular simulation or to
implement other tools that interact with the simulation in different
ways. Similar to implementing user-defined components which inherit from
the Component class, a custom interactive console inherits from the
InteractiveConsole class and must be registered with SST.

------------------------------------------------------------------------

### Available Features

The InteractiveConsole class provides functions to access simulation
information, such as:

- getCoreTimeBase

- getCurrentSimCycle

- getElapsedSimTime

- getEndSimCycle

- getRank

- getNumRanks

- getSimulationOutput

- getTimeVortexMaxDepth

- getSyncQueueDataSize

- getMemPoolUsage

- getTimeConverter

- getComponentObjectMap

It also provides the following actions:

- simulationRun

- schedule_interactive

------------------------------------------------------------------------

### Component Modifications for Custom Interactive Console Support

The sst-tools repository provides an example user-defined interactive
console, ICDebug, in sst-tools/sstcomp/icdbg, modeled after the
simpledebug prototype in sst-core.
([https://github.com/sstsimulator/sst-core/blob/v15.0.0_beta/src/sst/core/impl/interactive/simpleDebug.h](https://github.com/sstsimulator/sst-core/blob/v15.0.0_beta/src/sst/core/impl/interactive/simpleDebug.h))

------------------------------------------------------------------------

The ICDebug class below shows the key components of the user-defined
interactive console and is compiled into the icdbg library.

------------------------------------------------------------------------
```
namespace SST::ICDbg{
class ICDebug : public SST::InteractiveConsole
{
public:
  SST_ELI_REGISTER_INTERACTIVE_CONSOLE(
    ICDebug, // Class
  "icdbg", // Library
  "ICDebug", // Interactive Console Name
  SST_ELI_ELEMENT_VERSION(1, 0, 0),
  "{EXPERIMENTAL} Simple user-defined interactive debugging console for
  interactive mode.")
  ICDebug(Params& params);
  \~ICDebug() {}
  void execute(const std::string& msg) override;
};
} // namespace SST::ICDbg
```
Note that the registration format is as follows:

- Class Name: ICDebug

- Library Name: icdbg

- Interactive Console Name: ICDebug (does not need to match class name)

- Element Version: 1.0.0

- Description

When invoking sst with an interactive console, the console is specified
as \<LibraryName\>.\<InteractiveConsoleName\>, e.g.

`sst --interactive-console=icdbg.ICDebug --interactive-start=1us test.py`

The element version is used to assist library developers with
versioning. It is not checked by SST and does not need to match the SST
version.

sst-register -l lists the registered libraries and sst-info -q provides
a summary of the registered components (does Interactive console show up
in any of these?)

When an interactive console is triggered, the execute message (msg) is
printed and then the execute function is invoked.

## Custom Component-level Debug Support

Trigger from code, more complex

as providing access to debug functionality directly from code via the
base component.

The interactive console, and particularly watch points, typically break
at global synchronization points. The ability to instrument a component
directly allows the user to track state at a finer granularity or to
provide more detailed debugging and tracing information than what is
provided by the more generic simpledebug interface.

------------------------------------------------------------------------

The BaseComponent provides access to simulation state and

Component Information

- getType

- getID

- getStatisicLoadLevel

- getName

- getParentComponent

- printStatus

- getCoreTimeBase

- getCurrentSimCycle

- getCurrentPriority

- getElapsedSimTime

- getEndSimCycle

- getEndSimTime

- getRank

- getNumRanks

- getSimulationOutput

- getCurrentSimTime{Nano, Micro, Milli}

- get{Run, Init, Complete}PhaseElapsedRealTime

Actions

- addWatchPoint

- removeWatchPoint

- addWatchPointRecursive

- initiateInteractive

- emergencyShutdown

- 

Component functionality including statistics, setup, etc.

- 

------------------------------------------------------------------------

## Watchpoints

## Profiling

sst-register -l lists the registered libraries and sst-info -q provides
a summary of the registered components (does Interactive console show up
in any of these?)

https://sst-simulator.org/sst-docs/docs/tools/commandLine/sst-info#profile-points

### Profiling CLI

Profiling Points \[API Not Yet Final\]:

NOTE: Profiling points are still in development and syntax for enabling
profiling tools is subject to change. Additional profiling points may
also be added in the future.

Profiling points are points in the code where a profiling tool can be
instantiated. The profiling tool allows you to collect various data
about code segments. There are currently three

profiling points in SST core:

clock: profiles calls to user registered clock handlers

event: profiles calls to user registered event handlers set on Links

sync: profiles calls into the SyncManager (only valid for parallel
simulations)

The format for enabling profile point is a semicolon separated list
where each item specifies details for a given profiling tool using the
following format:

name:type(params)\[point\], where

name = name of tool to be shown in output

type = type of profiling tool in ELI format (lib.type)

params = optional parameters to pass to profiling tool, format is
key=value,key=value\...

point = profiling point to load the tool into

Profiling tools can all be enabled in a single instance of
\--enable-profiling, or you can use multiple instances of
\--enable-profiling to enable more than one profiling tool. It is also

possible to attach more than one profiling tool to a given profiling
point.

Examples:

\--enable-profiling=\"events:sst.profile.handler.event.time.high_resolution(level=component)\[event\]\"

\--enable-profiling=\"clocks:sst.profile.handler.clock.count(level=subcomponent)\[clock\]\"

\--enable-profiling=sync:sst.profile.sync.time.steady\[sync\]

### Available Profiling Tools

SST provides a number of profiling tools to track the number of calls
and time spent in marked code segments (? What is this?), clock and
event handlers, and synchronization.

Using sst-info sst will give a list of the profile tools currently
available from sst-core.

#### Component Code Segment Profile Tool

This tool tracks invocations of **marked** code segments. How to mark?
What are they?

- **Current ELI version**: 0.9.0

- **File**: src/sst/core/profile/componentProfileTool.h

- **Interface**: SST::Profile::ComponentCodeSegmentProfileTool

- **Parameters** (2 total)

  - **level**: Level at which to track profile (global, type, component,
    subcomponent) \[type\]

  - **track_points**: Determines whether independent profiling points
    are tracked \[true\]

##### Code Segment Count (profile.component.codesegment.count)

- **Description**: Profiler that will count the number of times through
  a marked code segment

#####  Code Segment High Resolution Time (profile.component.codesegment.time.high_resolution)

- **Description**: Profiler that will time component code segments using
  a high resolution clock

#####  Code Segment Steady Time (profile.component.codesegment.time.steady)

- Description: Profiler that will time component code segments using a
  steady clock

#### Clock Handler Profile Tool

- Current ELI version: 0.9.0

- File: src/sst/core/profile/clockHandlerProfileTool.h

- Interface: SST::Profile::ClockHandlerProfileTool

- Parameters (1 total)

  - level: Level at which to track profile (global, type, component,
    subcomponent) \[type\]

##### Clock Handler Call Count (profile.handler.clock.count)

- Description: Profiler that will count calls to clock handler functions

##### Clock Handler High Resolution Time (profile.handler.clock.time.high_resolution)

- Description: Profiler that will timeclock handlers using a high
  resolution clock

##### Clock Handler Steady Time (profile.handler.clock.time.steady)

- Description: Profiler that will time clock handlers using a steady
  clock

#### Event Hander Profile Tool

- Current ELI version: 0.9.0

- File: src/sst/core/profile/eventHandlerProfileTool.h

- Interface: SST::Profile::EventHandlerProfileTool

- Parameters (4 total)

  - level: Level at which to track profile (global, type, component,
    subcomponent) \[type\]

  - track_ports: Controls whether to track by individual ports \[false\]

  - profile_sends: Controls whether sends are profiled (due to location
    of profiling point in the code, turning on send profiling will incur
    relatively high overhead) \[false\]

  - profile_receives: Controls whether receives are profiled \[true\]

##### Event Handler Count (profile.handler.event.count)

- Description: Profiler that will count calls to event handler functions

##### Event Hander High Resolution Time (profile.handler.event.time.high_resolution)

- Description: Profiler that will time event handlers using a high
  resolution clock

##### Event Handler Steady Time (profile.handler.event.time.steady)

- Description: Profiler that will time event handlers using a steady
  clock

#### Sync Profile Tool

- Current ELI version: 0.9.0

- File: src/sst/core/profile/syncProfileTool.h

- Interface: SST::Profile::SyncProfileTool

- Parameters (0 total)

##### Sync Count (profile.sync.count)

- Description: Profiler that will count calls to sync

##### Sync High Resolution Time (profile.sync.time.high_resolution)

- Description: Profiler that will time synchronization using a high
  resolution clock

##### Sync Steady Time (profile.sync.time.steady)

- Description: Profiler that will time synchronization using a steady
  clock

### Custom Profile Tools

https://sst-simulator.org/sst-docs/docs/core/eli/register/sst_eli_register_profiletool

#include \<sst/core/profile/profiletool.h\>

https://sst-simulator.org/sst-docs/docs/core/component/load/registerProfilePoint

template \<typename T\>

typename T::ProfilePoint\* registerProfilePoint(const std::string &
pointName);

[[https://sst-simulator.org/sst-docs/docs/core/eli/document/sst_eli_document_profile_points]{.underline}](https://sst-simulator.org/sst-docs/docs/core/eli/document/sst_eli_document_profile_points)

## Data Serialization

<https://sst-simulator.org/sst-docs/docs/core/serialization/overview>

(This info is in overview) Serialization is used both for checkpoint
restart and to build an object map for the interactive console. For
checkpointing, all data needed for restart must be serialized. For
debugging, only the data needed for debugging must be serialized. If you
are doing both, you can specify that serialized data should not appear
in the object map in order to limit the amount of information that
appears during debug.

Serialization Macros

- SST_SER(var) // No options needed

- SST_SER(var, opt) // Options needed

- SST_SER_NAME(var, name, opt) // Provide an alias name to be used for
  mapping/interactive introspection

Serialization Options

- SerOption::as_ptr - If you are adding a data member that is not itself
  a pointer but other pointers to it exist and will be serialized

- SerOption::as_ptr_elem - Specifically for containers, if you have
  pointers to non-pointer elements in a container

- SerOption::map_read_only - Do not allow a user to modify this object
  in an interactive console

- SerOption::no_map: Do not serialize this object when mapping

## Checkpoint and Restart

Do we want checkpoint here or in its own doc?

<https://sst-simulator.org/sst-docs/docs/guides/features/checkpoint>
