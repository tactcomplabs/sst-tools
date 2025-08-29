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

namespace SST::NeuralNet{

//------------------------------------------
// NNBatchController
//------------------------------------------
NNBatchController::NNBatchController(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id )
{

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
  classImageLimit = params.find<uint64_t>("classImageLimit", 100000);

  // Configure Links
  linkHandlers[PortTypes::forward] = 
    configureLink(PortNames.at(PortTypes::forward),
              new Event::Handler2<NNBatchController, &NNBatchController::handleEvent>(this));

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
    trainingImages.load(trainingImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
  }

  if (testImagesStr.size()>0) {
    testImages.load(testImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);;
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

void NNBatchController::handleEvent(SST::Event *ev){
  std::cout << "batchController receiving data" << std::endl;
  NNEvent *nnev = static_cast<NNEvent*>(ev);
  auto data = nnev->getData();
  output.verbose(CALL_INFO,0,0, "%s: received %zu values\n",
    getName().c_str(),
    data.size());
  for ( auto d : data ) {
    output.verbose(CALL_INFO,0,0, "%d\n", d );
  }
  epoch++;
  readyToSend=true;
  delete ev;
}

void NNBatchController::sendData(){
  std::cout << "batchController sending data" << std::endl;
  NNEvent *nnev = new NNEvent({1,2,3});
  linkHandlers.at(PortTypes::forward)->send(nnev);
}

bool NNBatchController::clockTick( SST::Cycle_t currentCycle ) {

  // check to see if we've reached the completion state
  assert(epoch <= epochs);
  if( epoch==epochs ){
    output.verbose(CALL_INFO, 1, 0,
                   "%s ready to end simulation\n",
                   getName().c_str());
    primaryComponentOKToEndSim();
    return true;
  }

  if (readyToSend) {
    readyToSend=false;
    sendData();
  }

  return false;
}

} // namespace SST::NNBatchController

// EOF
