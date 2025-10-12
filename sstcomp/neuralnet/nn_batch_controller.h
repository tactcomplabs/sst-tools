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
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "dataset.h"
#include "nn_event.h"
#include "nn_layer_base.h"
// clang-format on

namespace SST::NeuralNet{

struct AccumulatedSums_t {
  double accuracy = 0;
  Losses loss = {};
  unsigned count = 0;
  double current_learning_rate = 0;
};

// -------------------------------------------------------
// NNBatchController
// -------------------------------------------------------
class NNBatchController : public NNLayerBase{
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
    {"batchSize",       "Number of images per batch", "128"},
    {"classImageLimit", "Maximum images per class to load [100000]", "100000"},
    {"epochs",          "Training iterations", "1"},
    {"evalImages",      "Path to directory containing evaluation images", NULL},
    {"printEvery",      "Epochs between printed summary information", "100"},
    {"testImages",      "Directory containing test images in class subdirs", NULL},
    {"trainingImages",  "Directory containing training images in class subdirs", NULL},
  )

private:
  // -- SST handlers
  SST::Output    output;                          ///< SST output handler
  TimeConverter* timeConverter;                   ///< SST time conversion handler
  SST::Clock::HandlerBase* clockHandler;          ///< Clock Handler

  // -- Component Parameters  
  unsigned batch_size = 128;                      ///< number of images per batch
  const unsigned eval_batch_size = 1;             ///< Predictions ship 1 image at time (for now?)
  unsigned classImageLimit = 100000;              ///< maximum images to load for each classification set
  unsigned epochs = 1;                            ///< training epochs
  unsigned print_every = 100;                     ///< epochs between printing summary information
  std::string evalImagesStr = {};                 ///< path to directory containing evaluation images
  std::string testImagesStr = {};                 ///< path to directory containing test images in class subdirs
  std::string trainingImagesStr = {};             ///< path to directory containing training images in class subdirs
  
  // -- Interactive debug console helpers for synchronized checkpoint
  bool dbgPauseBeforeEvaluation = false;
  bool dbgReloadEvaluationImages = false;
  void loadEvaluationImages();

  // -- Internal State
  bool enableTraining()   { return (trainingImagesStr.size()>0 && !trainingComplete); }
  bool enableValidation() { return (testImagesStr.size()>0 && !validationComplete); }
  bool enableEvaluation() { return (evalImagesStr.size()>0 && !evaluationComplete); }
  MODE fsmState_ = MODE::INVALID;
  bool trainingComplete = false;
  bool validationComplete = false;
  bool evaluationComplete = false;

  unsigned epoch = 0;                             ///< training interations counter
  unsigned step = 0;                              ///< step counter
  unsigned train_steps = 1;                       ///< total steps per training epoch
  unsigned validation_steps = 1;                  ///< total steps for validation run
  unsigned prediction_steps = 1;                  ///< total steps for evaluation run

  payload_t monitor_payload = {};                 ///< last data received on monitor port

  // -- Communication
  std::map<SST::NeuralNet::PortTypes,SST::Link*> linkHandlers = {};
  void forward_i_snd() { assert(false); }
  void forward_i_rcv(SST::Event *ev) {assert(false);}
  void forward_o_snd(MODE mode);
  void forward_o_rcv(SST::Event *ev) { assert(false);}
  void backward_i_snd() { assert(false); }
  void backward_i_rcv(SST::Event *ev);
  void backward_o_snd() { assert(false); }
  void backward_o_rcv(SST::Event *ev) { assert(false); }
  void monitor_rcv(SST::Event *ev);
  void monitor_snd() { assert(false); }

  //-- Payload - Do not serialize
  Eigen::MatrixXd batch_X = {};
  Eigen::MatrixXi batch_y = {};

  //-- Flow Control
  bool readyToSend=false;
  bool busy=false;

  //-- Image Management - Do not serialize
  //-- training and validation load everything by class directory
  Dataset trainingImages = {};
  Dataset testImages = {};
  //-- evaluation/prediction load images from a single directory (no class info)
  Dataset evalImages = {};
  
  //-- FSM support ( call from clocktick )
  bool initTraining();  // returns true to disable clocking
  bool stepTraining();
  bool continueTraining(); // resume after validation step
  bool initValidation();
  bool stepValidation();
  bool preCheckEvaluation();
  bool initEvaluation();
  bool stepEvaluation();
  bool complete();

  //-- Sequences
  bool launchTrainingStep();
  bool launchValidationStep();
  bool launchEvaluationStep();

  //-- Loss/Accuracy
  AccumulatedSums_t accumulatedSums = {};

  //-- Helpers
  Eutils util = {};

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNBatchController() : NNLayerBase() {};
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNBatchController)
};  //class NNBatchController
}   //namespace SST::NeuralNet

#endif  // _SST_NN_BATCH_CONTROLLER_H_

// EOF
