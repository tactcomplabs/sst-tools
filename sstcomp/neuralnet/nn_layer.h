//
// _nn_layer_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_NN_LAYER_H_
#define _SST_NN_LAYER_H_

// clang-format off
#include <map>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "nn_layer_base.h"
#include "nn_event.h"
// clang-format on

namespace SST::NeuralNet{

// -------------------------------------------------------
// NNLayer (registered)
// -------------------------------------------------------
class NNLayer : public NNLayerBase {
public:
  NNLayer( SST::ComponentId_t id, const SST::Params& params );
  ~NNLayer();

  // Component Lifecycle
  void init( unsigned int phase ) override;     // post-construction, polled events
  void setup() override;                        // pre-simulation, called once per component
  void complete( unsigned int phase ) override; // post-simulation, polled events
  void finish() override;                       // pre-destruction, called once per component
  void emergencyShutdown() override;            // SIGINT, SIGTERM
  void printStatus(Output& out) override;       // SIGUSR2

  // Clocking
  bool clockTick( SST::Cycle_t currentCycle );  // return true if clock should be disabled

  SST_ELI_REGISTER_COMPONENT( NNLayer,     // component class
                              "neuralnet", // component library
                              "NNLayer",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "NNLayer SST Component",
                              COMPONENT_CATEGORY_UNCATEGORIZED )

private:
  // SST handlers
  SST::Output    output; 
  // event handling
  std::map<SST::NeuralNet::PortTypes,SST::Link*> linkHandlers = {};
  void forward_i_snd() { assert(false); }
  void forward_i_rcv(SST::Event *ev);
  void forward_o_snd();
  void forward_o_rcv(SST::Event *ev) { assert(false);}
  void backward_i_snd() { assert(false); }
  void backward_i_rcv(SST::Event *ev);
  void backward_o_snd();
  void backward_o_rcv(SST::Event *ev) { assert(false); }
  void monitorEvent(SST::Event *ev) { assert(false); }
  void monitor_rcv(SST::Event *ev) {assert(false); };
  void monitor_snd();
  // Internals
  bool lastComponent = false;
  std::vector<uint64_t> forwardData_i = {};
  std::vector<uint64_t> forwardData_o = {};
  std::vector<uint64_t> backwardData_i = {};
  std::vector<uint64_t> backwardData_o = {};

  // -- SST handlers
  TimeConverter* timeConverter;
  SST::Clock::HandlerBase* clockHandler;

  // internals
  bool driveForwardPass = false;
  bool driveBackwardPass = false;
  bool driveMonitor = false;

};  //class NNLayer

class NNInputLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNInputLayer,   // Class name
        "neuralnet",    // Library name
        "NNInputLayer",   // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name

  NNInputLayer(ComponentId_t id, Params& params) : NNSubComponentAPI(id,params) {};
  ~NNInputLayer() {};

  virtual void forward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o) final;
  virtual void backward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o) final;

}; //class NNInputLayer

} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_H_

// EOF
