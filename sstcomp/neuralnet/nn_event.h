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

// -- External Headers
#include "EIGEN.h"
#include "SST.h"

// clang-format on

namespace SST::NeuralNet{

enum class MODE {INVALID, TRAINING, VALIDATION, EVALUATION};

enum class PortTypes { forward_i, forward_o, backward_i, backward_o, monitor };
const std::map<PortTypes, std::string> PortNames {
  { PortTypes::forward_i, "forward_i"}, { PortTypes::forward_o, "forward_o"},
  { PortTypes::backward_i, "backward_i" }, { PortTypes::backward_o, "backward_o" },
  { PortTypes::monitor, "monitor"}
};

struct payload_t {
  MODE mode = MODE::INVALID;
  Eigen::MatrixXd X_batch = {};
  Eigen::MatrixXi y_batch = {};
  payload_t() {};
  payload_t(MODE m, Eigen::MatrixXd X, Eigen::MatrixXi y) :
    mode(m), X_batch(X), y_batch(y) {}; 
};

// -------------------------------------------------------
// NNEvent
// -------------------------------------------------------
class NNEvent : public SST::Event{
public:
  NNEvent() : SST::Event() {}
  NNEvent(const payload_t& p) : SST::Event(), payload_(p) {}
  virtual ~NNEvent() {}
  const payload_t& payload() { return payload_; }
private:
  payload_t payload_;
}; //class NNEvent

} //namespace SST::NeuralNet

#endif  // _SST_NN_EVENT_H_

// EOF
