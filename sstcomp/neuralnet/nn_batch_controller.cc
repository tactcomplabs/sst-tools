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

void NNBatchController::forward_o_snd(){
  output.verbose(CALL_INFO,2,0, "%s sending forward pass data\n", getName().c_str());
  NNEvent *nnev = new NNEvent({1,2,3});
  linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNBatchController::backward_i_rcv(SST::Event *ev) {
  output.verbose(CALL_INFO,2,0, "%s receiving backward pass data\n", getName().c_str());
  delete(ev);
}

void NNBatchController::monitor_rcv(SST::Event *ev) {
  output.verbose(CALL_INFO,0,0, "%s receiving monitor data\n", getName().c_str());
  NNEvent* nnev = static_cast<NNEvent*>(ev);
  auto data = nnev->getData();
  uint64_t sum=0;
  for ( auto d : data) {
    sum += d;
  }
  output.verbose(CALL_INFO,0,0, "Result=%" PRIu64 "\n",sum);
  readyToSend = true;
  delete(ev);
}

bool NNBatchController::clockTick( SST::Cycle_t currentCycle ) {

  if (!readyToSend)
    return false;

  readyToSend = false;

  assert(epoch <= epochs);
  if (epoch++ == epochs) {
    output.verbose(CALL_INFO, 1, 0,
                   "%s ready to end simulation\n",
                   getName().c_str());
    primaryComponentOKToEndSim();
    return true;
  }

  output.verbose(CALL_INFO, 1, 0,
                   "%s initiating epoch %" PRId64 "\n",
                   getName().c_str(), epoch);
  forward_o_snd();
  return false;
}

} // namespace SST::NNBatchController

// EOF
