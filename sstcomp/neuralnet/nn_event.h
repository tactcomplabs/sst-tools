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

enum class OPTIMIZER_STATE: unsigned {
  INVALID = 0,
  PRE_UPDATE = 1,
  ACTIVE = 2,
  POST_UPDATE = 3
};

struct optimizer_data_t {
  OPTIMIZER_STATE optimizerState = OPTIMIZER_STATE::INVALID;
  // shadow version of common optimizer members
  double current_learning_rate = 0.;
  unsigned iterations = 0;
  optimizer_data_t() : 
    optimizerState(OPTIMIZER_STATE::INVALID),
    current_learning_rate(0), 
    iterations(0) {}
  optimizer_data_t(OPTIMIZER_STATE state, double lr, unsigned iter) :
    optimizerState(state),
    current_learning_rate(lr), 
    iterations(iter) {}
};

struct payload_t {
  MODE mode = MODE::INVALID;
  Eigen::MatrixXd data = {};
  Eigen::MatrixXi classes = {};
  optimizer_data_t optimizer_data = {};
  payload_t() {};
  payload_t(MODE m, Eigen::MatrixXd X, Eigen::MatrixXi y) :
    mode(m), data(X), classes(y) {}; 
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
