# SST Interactive Console Release Notes for SST 15.1

## SST Command Line Changes for the Interactive Console 
- It is no longer required to specify the ` –-interactive-console` option. The default console, `sst.interactive.simpledebug`, will be automatically selected unless an alternate is specified.
- For initial support of generating checkpoints as a debug action, a new `--checkpoint-enable` option must be specified at the command line.
- Added `--replay=<filename>` to enable automatic execution of commands upon the first entry into the interactive console.
  
## Interactive Console Features
- Introduced debug watchpoints which allow defining conditional expressions of mapped variables which result in user defined actions such as entering interactive mode or generating a checkpoint
- Introduced watchpoint trace buffers, internal circular buffers, for capturing watchpoint data over time. This data can be viewed in the interactive console or printed as the result of a watchpoint action.
- Provided support for several logic operators for creating watchpoint trigger expressions.
- Provided sampling controls controlling debug action at the entry or the exit of clock and event handlers.
- Provided auto-completion and history retrieval similar to bash supporting common arrow, tab, and control keys.
- Provided console command history and history access controls.
- Provided logging console commands to a file.
- Provided reading and replay console commands from a file.
- Improved displaying floating point data types.
- No longer require --interactive-console option to use the default console handler
- Added --checkpoint-enable to support checkpointing from the interactive console
- Improved help and detailed help for console commands.

## Interactive Console Restrictions
- The interactive console is still considered an experimental feature and will continue to change as development and testing progresses.
- Use of the SST Serialization macros provides automatic mapping of many types of data for visibility in the interactive console. Arithmetic types, arrays, and several common containers are supported. The only indication that a variable is not mapped is that it will not be visible in the interactive console. For these cases, the object can be manually mapped by the user.

