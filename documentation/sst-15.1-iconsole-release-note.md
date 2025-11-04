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
- The following standard types currently have *built-in* mapping mode support
  - bool, char, float, double, signed/unsigned int/long, float, double ...
  - size_t
  - enum, arrays, string
  - dequeue, forward_list, list, vector
  - map, unordered_map, unordered_multimap,
  - set, unordered_set, multiset, unordered_multiset

## Interactive Console Support and Restrictions for Standard Types
- The following standard types currently do not have *built-in* mapping mode support (these require manual mapping for use in the interactive console)
  - pair, tuple
  - stack, queue, priority_queue
  - atomic
  - bitset
  - shared_ptr, weak_ptr, unique_ptr
  - variant, optional
