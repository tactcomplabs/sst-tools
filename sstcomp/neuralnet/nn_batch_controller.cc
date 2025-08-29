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
  tcldbg::spinner("NNBatchController_SPINNER");
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "NNBatchController[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  const std::string cpuClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<NNBatchController,&NNBatchController::clockTick>(this);
  timeConverter = registerClock(cpuClock, clockHandler);

  // read the rest of the parameters
  epochs = params.find<uint64_t>("epochs", 10000);

  // // Load optional subcomponent in the cpt_check slot
  // CPTSubComp = loadUserSubComponent<CPTSubComp::CPTSubCompAPI>("CPTSubComp");
  // if (checkSlot && !CPTSubComp)
  //   output.fatal(CALL_INFO, -1, "SubComponent did not load properly\n");
  
  // Complete construction
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

NNBatchController::~NNBatchController(){
}

void NNBatchController::setup(){
  assert(epochs>0);
}

void NNBatchController::finish(){
}

void NNBatchController::init( unsigned int phase ){
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
