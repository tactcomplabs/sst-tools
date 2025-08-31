//
// _nn_batch_controller_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <assert.h>
#include "nn_batch_controller.h"
#include "tcldbg.h"

namespace SST::NeuralNet{

//------------------------------------------
// NNBatchController
//------------------------------------------
NNBatchController::NNBatchController(SST::ComponentId_t id, const SST::Params& params ) :
  NNLayerBase( id )
{
  tcldbg::spinner("NNSPINNER");
  // verbosity
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "NNBatchController[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  
  // clocking 
  const std::string systemClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<NNBatchController,&NNBatchController::clockTick>(this);
  timeConverter = registerClock(systemClock, clockHandler);

  // parameters
  trainingImagesStr = params.find<std::string>("trainingImages");
  testImagesStr = params.find<std::string>("testImages");
  evalImageStr = params.find<std::string>("evalImage");
  epochs = params.find<unsigned>("epochs", 0);
  classImageLimit = params.find<unsigned>("classImageLimit", 100000);

  // Configure Links
  linkHandlers[PortTypes::forward_i] = 
    configureLink(PortNames.at(PortTypes::forward_i),
              new Event::Handler2<NNBatchController, &NNBatchController::forward_i_rcv>(this));
  linkHandlers[PortTypes::forward_o] = 
    configureLink(PortNames.at(PortTypes::forward_o),
              new Event::Handler2<NNBatchController, &NNBatchController::forward_o_rcv>(this));
  linkHandlers[PortTypes::backward_i] = 
    configureLink(PortNames.at(PortTypes::backward_i),
              new Event::Handler2<NNBatchController, &NNBatchController::backward_i_rcv>(this));
  linkHandlers[PortTypes::backward_o] = 
    configureLink(PortNames.at(PortTypes::backward_o),
              new Event::Handler2<NNBatchController, &NNBatchController::backward_o_rcv>(this));
  linkHandlers[PortTypes::monitor] = 
    configureLink(PortNames.at(PortTypes::monitor),
              new Event::Handler2<NNBatchController, &NNBatchController::monitor_rcv>(this));

  // Complete construction
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

NNBatchController::~NNBatchController(){
}

void NNBatchController::init( unsigned int phase ){

}

void NNBatchController::setup(){

  if (trainingImagesStr.size()>0) {
    trainingImages.load(trainingImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
    mode_sequence.push(MODE::TRAINING);
  }

  if (testImagesStr.size()>0) {
    testImages.load(testImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
    mode_sequence.push(MODE::VALIDATION);
  }

  if (evalImageStr.size()>0) {
    evalImage.load(evalImageStr.c_str(), 
      EigenImage::TRANSFORM::INVERT, 
      EigenImage::TRANSFORM::LINEARIZE, 
      true);
    mode_sequence.push(MODE::EVALUATION);
  }

  if (mode_sequence.empty())
    output.fatal(CALL_INFO, -1, "Nothing to do. Please set --trainingImages, --testImages, and/or --evalImage");
 
}

void NNBatchController::complete( unsigned int phase ){
}

void NNBatchController::finish(){
}

void NNBatchController::emergencyShutdown(){
}

void NNBatchController::printStatus( Output& out ){
}

void NNBatchController::forward_o_snd(){
  output.verbose(CALL_INFO,2,0, "%s sending forward pass data\n", getName().c_str());
  NNEvent *nnev = new NNEvent({1,2,3});
  linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNBatchController::backward_i_rcv(SST::Event *ev) {
  NNEvent* nnev = static_cast<NNEvent*>(ev);
  auto data = nnev->getData();
  uint64_t sum = 0;
  for (auto d : data) {
    sum += d;
  }
  output.verbose(CALL_INFO,0,0, "%s Epoch completed. Result=%" PRId64 "\n", 
                  getName().c_str(), sum);
  readyToSend = true;
  //reregister clock for next batch
  reregisterClock(timeConverter, clockHandler);
  delete(ev);
}

void NNBatchController::monitor_rcv(SST::Event *ev) {
  output.verbose(CALL_INFO,2,0, "%s receiving monitor data\n", getName().c_str());
  NNEvent* nnev = static_cast<NNEvent*>(ev);
  auto data = nnev->getData();
  uint64_t sum=0;
  for ( auto d : data) {
    sum += d;
  }
  output.verbose(CALL_INFO,0,0, "Forward Pass Result=%" PRIu64 "\n",sum);
  // don't reregister clock here.
  delete(ev);
}

bool NNBatchController::clockTick( SST::Cycle_t currentCycle ) {
  // Clocking control should ensure we have something to do.
  assert( !busy || readyToSend);

  if (mode_sequence.empty()) {
    output.verbose(CALL_INFO, 2, 0,
                  "%s has nothing else to do. Ending simulation.\n",
                  getName().c_str());
    primaryComponentOKToEndSim();
    return true;
  }

  if (!busy) {
    assert(readyToSend==false); 
    // not busy so get next task
    current_mode = mode_sequence.front();
    mode_sequence.pop();

    switch (current_mode) {
      case MODE::TRAINING:
        output.verbose(CALL_INFO, 0, 0, 
          "Starting training phase. epochs=%" PRId32 " batch_size=%" PRId32 " steps=%" PRId32 "\n",
          epochs, batch_size, train_steps);
        readyToSend = true;
        busy = true;
        break;
      case MODE::VALIDATION:
        output.verbose(CALL_INFO, 0, 0, 
          "Starting validation phase. steps=%" PRId32 "\n",
          validation_steps);
        break;
      case MODE::EVALUATION:
        output.verbose(CALL_INFO, 0, 0, "Starting evaluation phase\n");
        break;
      default:
        assert(false);
    }
    return false;
  }

  // OK we are busy so must be sending something.
  assert(readyToSend==true);
  readyToSend = false;

  switch (current_mode) {
    case MODE::TRAINING:
    {
      assert(epoch <= epochs);
      if (epoch++ == epochs) {
        output.verbose(CALL_INFO, 2, 0,
                    "%s completed training\n",
                    getName().c_str());
        busy=false;
      } else {
        output.verbose(CALL_INFO, 0, 0,
                   "%s initiating epoch %" PRId32 "\n",
                   getName().c_str(), epoch);
        // Launch batch and disable clocks
        forward_o_snd();
        return true;
      }
      break;
    }
    case MODE::VALIDATION:
      break;
    case MODE::EVALUATION:
      break;
    default:
      assert(false);
  }

  return false;
}

} // namespace SST::NNBatchController

// EOF
