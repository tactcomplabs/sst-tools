//
// _batch_controller_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <assert.h>
#include "neuralnet.h"
#include "tcldbg.h"

namespace SST::NeuralNet{

//------------------------------------------
// NNBatchController
//------------------------------------------
NNBatchController::NNBatchController(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id )
{
  // gdb debug support
  tcldbg::spinner("NNBATCHCONTROLLER_SPINNER");

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
  epochs = params.find<uint64_t>("epochs", 0);

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
  if (trainingImagesStr.size()==0 && testImagesStr.size()==0 && evalImageStr.size()==0) {
    output.fatal(CALL_INFO, -1, "Nothing to do. Please set --trainingImages, --testImages, and/or --evalImage");
  }

  if (trainingImagesStr.size()>0) {
    trainingImages.load(trainingImagesStr, Dataset::DTYPE::IMAGE, true);
  }

  if (testImagesStr.size()>0) {
    testImages.load(testImagesStr, Dataset::DTYPE::IMAGE, true);;
  }

  if (evalImageStr.size()>0) {
    evalImage.load(evalImageStr.c_str(), 
      EigenImage::TRANSFORM::INVERT, 
      EigenImage::TRANSFORM::LINEARIZE, 
      true);
  }
}

void NNBatchController::complete( unsigned int phase ){
}

void NNBatchController::finish(){
}

void NNBatchController::emergencyShutdown(){
}

void NNBatchController::printStatus( Output& out ){
}

void NNBatchController::serialize_order(SST::Core::Serialization::serializer& ser){
  SST::Component::serialize_order(ser);
  SST_SER(epochs);
}

void NNBatchController::handleEvent(SST::Event *ev){ }

void NNBatchController::sendData(){
}

bool NNBatchController::clockTick( SST::Cycle_t currentCycle ) {

    // check to see if we've reached the completion state
  if( (uint64_t)(currentCycle) >= epochs ){
    output.verbose(CALL_INFO, 1, 0,
                   "%s ready to end simulation\n",
                   getName().c_str());
    primaryComponentOKToEndSim();
    return true;
  }

  return false;
}

} // namespace SST::NNBatchController

// EOF
