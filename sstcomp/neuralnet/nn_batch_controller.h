//
// _nn_batch_controller_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_NN_BATCH_CONTROLLER_H_
#define _SST_NN_BATCH_CONTROLLER_H_

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
#include "nn_event.h"

// -- SubComponent API

// clang-format on

namespace SST::NeuralNet{

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
    {"epochs",          "Training iterations", "0"},
    {"classImageLimit", "Maximum images per class to load [100000]", "100000"}
  )

  // -------------------------------------------------------
  // NNBatchController Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    { "forward",
      "forward port",
      {"neuralnet.NNevent"}
    },
  )

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
  uint64_t epoch = 0;                             ///< iterations
  uint64_t classImageLimit = 100000;              ///< maximum images to load for each classification set

  // -- private methods
  std::map<SST::NeuralNet::PortTypes,SST::Link*> linkHandlers = {};
  void handleEvent(SST::Event *ev);
  void sendData();
  bool readyToSend=true;

  // -- private members
  Dataset trainingImages = {};
  Dataset testImages = {};
  EigenImage evalImage = {};

};  //class NNBatchController
}   //namespace SST::NeuralNet

#endif  // _SST_NN_BATCH_CONTROLLER_H_

// EOF
