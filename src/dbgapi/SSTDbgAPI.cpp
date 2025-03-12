//
// _SSTDbgAPI_cpp_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "SSTDbgAPI.h"

SSTDebug::SSTDebug( const std::string host, int port ) : connected(false) {
  hostStr = "tcp://" + host + ":" + std::to_string(port);
}

SSTDebug::~SSTDebug(){
  this->disconnect();
}

bool SSTDebug::connect(){
  sendsock.bind(hostStr);
  const std::string lastEnd = sendsock.get(zmq::sockopt::last_endpoint);
  recvsock.connect(lastEnd);
  connected = true;
  return true;
}

bool SSTDebug::disconnect(){
  if( connected ){
    const std::string lastEnd = sendsock.get(zmq::sockopt::last_endpoint);
    recvsock.disconnect(lastEnd);
    sendsock.unbind(hostStr);
    connected = false;
  }
  return true;
}

// EOF
