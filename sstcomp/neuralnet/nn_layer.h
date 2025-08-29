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
#include "nn_event.h"

// -- Neural Net headers

// -- SubComponent API

// clang-format on

namespace SST::NeuralNet{

// -------------------------------------------------------
// NNLayer
// -------------------------------------------------------
class NNLayer : public SST::Component{
public:
  /// NNLayer: top-level SST component constructor
  NNLayer( SST::ComponentId_t id, const SST::Params& params );

  /// NNLayer: top-level SST component destructor
  ~NNLayer();

  //
  // Component Lifecycle
  //
  void init( unsigned int phase ) override;     // post-construction, polled events
  void setup() override;                        // pre-simulation, called once per component
  void complete( unsigned int phase ) override; // post-simulation, polled events
  void finish() override;                       // pre-destruction, called once per component
  void emergencyShutdown() override;            // SIGINT, SIGTERM
  void printStatus(Output& out) override;       // SIGUSR2

  // return true if the clock should be disabled
  bool clockTick( SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // NNLayer Component Registration Data
  // -------------------------------------------------------
  /// NNLayer: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( NNLayer,     // component class
                              "neuralnet",           // component library
                              "NNLayer",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "NNLayer SST Component",
                              COMPONENT_CATEGORY_UNCATEGORIZED )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" },
  )

  // -------------------------------------------------------
  // NNLayer Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    { "forward", // PortNames.at(PortTypes::forward),
      "forward port",
      {"neuralnet.NNevent"}
    }
  )

  // -------------------------------------------------------
  // NNLayer Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // NNLayer Component Checkpoint Methods
  // -------------------------------------------------------
  /// NNLayer: serialization constructor
  NNLayer() : SST::Component() {}

  /// NNLayer: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

  /// NNLayer: serialization implementations
  ImplementSerializable(SST::NeuralNet::NNLayer)

private:
  // -- SST handlers
  SST::Output    output;                          ///< SST output handler
  TimeConverter* timeConverter;                   ///< SST time conversion handler
  SST::Clock::HandlerBase* clockHandler;          ///< Clock Handler

  // -- Component Parameters   

  // -- private methods
  /// event handling
  std::map<SST::NeuralNet::PortTypes,SST::Link*> linkHandlers = {};
  void handleEvent(SST::Event *ev);
  void sendData();
  bool readyToSend = false;

  // -- private members
  std::vector<unsigned> out = {};

};  //class NNLayer
} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_H_

// EOF
