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

#include "SST.h"
#include "nn_event.h"
// clang-format on

namespace SST::NeuralNet{

// -------------------------------------------------------
// NNLayerBase ( not registered )
// -------------------------------------------------------
class NNLayerBase : public SST::Component{

public:
  SST_ELI_REGISTER_COMPONENT_BASE(SST::NeuralNet::NNLayerBase)
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" },
    {"lastComponent",   "Indicate component is last layer",     "0" }
  )
  SST_ELI_DOCUMENT_PORTS(
    { "forward_i",  "forward pass input port",   {"neuralnet.NNevent"} },
    { "forward_o",  "forward pass output port",  {"neuralnet.NNevent"} },
    { "backward_i", "backward pass input port",  {"neuralnet.NNevent"} },
    { "backward_o", "backward pass output port", {"neuralnet.NNevent"} },
    { "monitor",     "monitoring port",          {"neuralnet.NNevent"} }
  )

  explicit NNLayerBase(ComponentId_t id) : SST::Component(id) {}
  NNLayerBase() : SST::Component() {}
  ~NNLayerBase() {}

}; // class NNLayerBase

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

protected:
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
  std::vector<unsigned> forwardData = {};
  std::vector<unsigned> backwardData = {};
  
private:
  // -- SST handlers
  TimeConverter* timeConverter;
  SST::Clock::HandlerBase* clockHandler;
  // internals
  bool driveForwardPass = false;
  bool driveBackwardPass = false;
  bool driveMonitor = false;

};  //class NNLayer

} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_H_

// EOF
