//
// _batch_controller_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_BATCH_CONTROLLER_H_
#define _SST_BATCH_CONTROLLER_H_

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

// -- Neural Net headers
#include "dataset.h"

// -- SubComponent API

// clang-format on

namespace SST::NeuralNet{

// -------------------------------------------------------
// NeuralNetEvent
// -------------------------------------------------------
class NeuralNetEvent : public SST::Event{
public:
  /// NeuralNetEvent : standard constructor
  NeuralNetEvent() : SST::Event() {}

  /// NeuralNetEvent: constructor
  NeuralNetEvent(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// NeuralNetEvent: destructor
  virtual ~NeuralNetEvent() {}

  /// NeuralNetEvent: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< NeuralNetEvent: data payload

  /// NeuralNetEvent: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data);
  }

  /// NeuralNetEvent: serialization implementor
  ImplementSerializable(SST::NeuralNet::NeuralNetEvent);

};  // class NeuralNetEvent

// -------------------------------------------------------
// NNBatchController
// -------------------------------------------------------
class NNBatchController : public SST::Component{
public:
  /// NNBatchController: top-level SST component constructor
  NNBatchController( SST::ComponentId_t id, const SST::Params& params );

  /// NNBatchController: top-level SST component destructor
  ~NNBatchController();

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
  // NNBatchController Component Registration Data
  // -------------------------------------------------------
  /// NNBatchController: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( NNBatchController,     // component class
                              "neuralnet",           // component library
                              "NNBatchController",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "NNBatchController SST Component",
                              COMPONENT_CATEGORY_UNCATEGORIZED )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" },
    {"trainingData",    "Directory containing training data", NULL},
    {"testData",        "Directory containing test data", NULL},
    {"evalData",        "Directory containing evaluation data", NULL},
    {"epochs",          "Training iterations", "0"}
  )

  // -------------------------------------------------------
  // NNBatchController Component Port Data
  // -------------------------------------------------------
  // SST_ELI_DOCUMENT_PORTS(
  //   {"port%(num_ports)d",
  //     "Ports which connect to endpoints.",
  //     {"chkpnt.NeuralNetEvent", ""}
  //   }
  // )

  // -------------------------------------------------------
  // NNBatchController SubComponent Parameter Data
  // -------------------------------------------------------
  // SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
  //   { "CPTSubComp",
  //     "Expansion slot for more checkpoint type checking",
  //     "SST::CPTSubComp::CPTSubCompAPI"
  //   }
  // )

  // -------------------------------------------------------
  // NNBatchController Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // NNBatchController Component Checkpoint Methods
  // -------------------------------------------------------
  /// NNBatchController: serialization constructor
  NNBatchController() : SST::Component() {}

  /// NNBatchController: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

  /// NNBatchController: serialization implementations
  ImplementSerializable(SST::NeuralNet::NNBatchController)

private:
  // -- SST handlers
  SST::Output    output;                          ///< SST output handler
  TimeConverter* timeConverter;                   ///< SST time conversion handler
  SST::Clock::HandlerBase* clockHandler;          ///< Clock Handler

  // -- Component Parameters   
  std::string trainingImagesStr = {};             ///< path to directory containing training images
  std::string testImagesStr = {};                 ///< path to directory containing test images
  std::string evalImageStr = {};                  ///< path to a single evaluation image file
  uint64_t epochs = 0;                            ///< training epochs

  // -- private methods
  /// event handler
  void handleEvent(SST::Event *ev);
  /// sends data to adjacent links
  void sendData();

  // -- private members
  Dataset trainingImages = {};
  Dataset testImages = {};
  EigenImage evalImage = {};

};  //class NNBatchController
}   //namespace SST::NeuralNet

#endif  // _SST_BATCH_CONTROLLER_H_

// EOF
