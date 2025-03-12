# SST Debug Network Protocol

## Requirements
* TCL
* Support across systems (not just across loopback address)
* Security
* One outstanding command at a time.  One client, one server
* Client should track outstanding requests and buffer commands until previous commands are completed

## Protocol
### Protocol Basis

All protocol requests are in the form of vectors of elements where the first
vector or array element is the command and all subsequent elements are the command
arguments.  We define all the requests in terms of the vector element

V[0] : COMMAND
V[1] : ARG

#### Request Structure
V[0] : REQUEST_COMMAND
V[1] : ARG

#### Response Structure
V[0] : RESPONSE_COMMAND
V[1] : ARG

### Connect Security
The probe component/subcomponent should contain an extra parameter that 
is a security key read in from the SDL model file.  This key should be 
sent from the client debug interface in order to verify connectivity.  This 
is certainly not a full proof way of securing the connection, but it ensures 
that accidental connections are rare.

### Command Responses
* SUCCESS : command success
* ERROR : Error occurred, error string follows in buffer
* NODATA: Soft error, no data present

### Basic Connectivity
#### Connect Key
* Server: open and listen to incoming requests
* Client: open a new connection with key and bind to a socket for send/recv

#### Disconnect
* Server: receive disconnect signal and close socket
* Client: send disconnect command to server then unbind

### Basic Info Query
#### Help
* Server: N/A
* Client: print help menu

#### State
* Server: receive STATE request, respond with the running state: {RUNNING,PAUSED,INIT}
* Client: Send STATE request, receive STATE response

#### Cycle
* Server: receive state request, respond with the current cycle.  If STATE == INIT, Cycle = 0
* Client: send CYCLE request, receive the current component cycle

#### Commands
* Server: receive commands request, respond with vector of custom commands.
* Client: send COMMANDS request, receive vector of custom component commands

#### Component
* Server: receive the component request, respond with the name of the component
* Client: send COMPONENT request, receive the name of the target component

#### History
* Server: N/A
* Client: print the command history

### Component Control

### Probe State
#### ProbeState
* Server: receive the probestate request, respond with the state: {IDLE, PRESAMPLING, POSTSAMPLING, WAIT}
* Client: send the probestate request, receive the state of the probe

#### ProbePurge
* Server: receive the probepurge request, purge the probe buffer.  No response
* Client: send the probepurge request, no response

### I/O for Checkpoint Files
* Do we send I/O requests for checkpoint files over the socket?
* Do we send file locations?
* Do we rely upon shared filesystems and user knowledge?

### Custom Commands
* Need an internal struct to describe custom probe commands
