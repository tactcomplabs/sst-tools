//
// _nn_event_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_NN_EVENT_H_
#define _SST_NN_EVENT_H_

// clang-format off
// -- Standard Headers
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// -- SST Headers
#include "SST.h"

// clang-format on

namespace SST::NeuralNet{

enum class PortTypes { forward_i, forward_o, backward_i, backward_o, monitor };
const std::map<PortTypes, std::string> PortNames {
  { PortTypes::forward_i, "forward_i"}, { PortTypes::forward_o, "forward_o"},
  { PortTypes::backward_i, "backward_i" }, { PortTypes::backward_o, "backward_o" },
  { PortTypes::monitor, "monitor"}
};

// -------------------------------------------------------
// NNEvent
// -------------------------------------------------------
class NNEvent : public SST::Event{
public:
  /// NNEvent : standard constructor
  NNEvent() : SST::Event() {}

  /// NNEvent: constructor
  NNEvent(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// NNEvent: destructor
  virtual ~NNEvent() {}

  /// NNEvent: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< NNEvent: data payload

  /// NNEvent: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data);
  }

  /// NNEvent: serialization implementor
  ImplementSerializable(SST::NeuralNet::NNEvent);

};  // class GridTestNodeEvent

} //namespace SST::NeuralNet

#endif  // _SST_NN_EVENT_H_

// EOF
