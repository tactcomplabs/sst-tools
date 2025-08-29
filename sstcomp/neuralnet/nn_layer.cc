//
// _nn_layer_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <assert.h>
#include "nn_layer.h"

namespace SST::NeuralNet{

//------------------------------------------
// NNLayer
//------------------------------------------
NNLayer::NNLayer(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id )
{

  // verbosity
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "NNLayer[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  
  // clocking 
  const std::string systemClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<NNLayer,&NNLayer::clockTick>(this);
  timeConverter = registerClock(systemClock, clockHandler);

  // parameters


  // Configure Links
  linkHandlers[PortTypes::forward] = 
    configureLink(PortNames.at(PortTypes::forward),
              new Event::Handler2<NNLayer, &NNLayer::handleEvent>(this));


  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

NNLayer::~NNLayer(){
}

void NNLayer::init( unsigned int phase ){

}

void NNLayer::setup(){
}

void NNLayer::complete( unsigned int phase ){
}

void NNLayer::finish(){
}

void NNLayer::emergencyShutdown(){
}

void NNLayer::printStatus( Output& out ){
}

void NNLayer::serialize_order(SST::Core::Serialization::serializer& ser){
  SST::Component::serialize_order(ser);
}

void NNLayer::handleEvent(SST::Event *ev){
  NNEvent *nnev = static_cast<NNEvent*>(ev);
  auto data = nnev->getData();

  std::cout << "layer receiving data" << std::endl;
  output.verbose(CALL_INFO,0,0, "%s: doubling %zu values\n",
    getName().c_str(),
    data.size());

  out.clear();
  for ( size_t i=0;i<data.size();i++) {
    out.emplace_back(2 * data[i]);
  }
  readyToSend = true; // TODO enable clock handler
  delete ev;
}

void NNLayer::sendData(){
  std::cout << "layer returning data" << std::endl;
  NNEvent* nnev = new NNEvent(out);
  linkHandlers.at(PortTypes::forward)->send(nnev);

}

bool NNLayer::clockTick( SST::Cycle_t currentCycle ) {
  if (readyToSend) {
    sendData();
    readyToSend=false; // TODO disable clock handler
  }
  return false;
}

} // namespace SST::NNLayer

// EOF
