//
// _SSTDbgAPI_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SSTDbgAPI_h_
#define _SSTDbgAPI_h_

#include <zmq_addon.hpp>
#include <string>

namespace SSTDEBUG::SSTDebug{

class SSTDebug{
public:
  /// SSTDebug: constructor
  SSTDebug( const std::string host, int port );

  /// SSTDebug: destructor
  ~SSTDebug();

  /// SSTDebug: establishes a connection to the debug probe
  bool connect();

  /// SSTDebug: disconnects from the debug probe
  bool disconnect();

  /// SSTDebug: retrieve the history of sent operations
  const std::vector getHistory();

  /// SSTDebug: replay the target command in the command queue
  bool replayCommand(unsigned int cmd);

  /// SSTDebug: send command
  /// SSTDebug: send command with argument

private:
  // ZMQ handlers
  zmq::context_t ctx;       ///< ZeroMQ Connection context

  inline zmq::socket_t  sendsock(ctx, zmq::socket_type::push);  ///< ZeroMQ Send socket
  inline zmq::socket_t  recvsock(ctx, zmq::socket_type::pull);  ///< ZeroMQ Recv socket

  bool connected;           ///< determines if connection is active
  const std::string hostStr;///< connection host string

  std::vector<std::string> history; ///< Command queue history

}; // class SSTDebug

} // namespace SSTDEBUG::Dbg

#endif

// EOF
