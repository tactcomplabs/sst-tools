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
#include "tcldbg.h"

namespace SST::NeuralNet{

//------------------------------------------
// NNLayer
//------------------------------------------
NNLayer::NNLayer(SST::ComponentId_t id, const SST::Params& params ) :
  NNLayerBase( id )
{

  // parameters
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "NNLayer[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  lastComponent = params.find<bool>("lastComponent", false);

  // clocking 
  const std::string systemClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<NNLayer,&NNLayer::clockTick>(this);
  timeConverter = registerClock(systemClock, clockHandler);

  // Configure Links
  linkHandlers[PortTypes::forward_i] = 
    configureLink(PortNames.at(PortTypes::forward_i),
              new Event::Handler2<NNLayer, &NNLayer::forward_i_rcv>(this));
  linkHandlers[PortTypes::forward_o] = 
    configureLink(PortNames.at(PortTypes::forward_o),
              new Event::Handler2<NNLayer, &NNLayer::forward_o_rcv>(this));
  linkHandlers[PortTypes::backward_i] = 
    configureLink(PortNames.at(PortTypes::backward_i),
              new Event::Handler2<NNLayer, &NNLayer::backward_i_rcv>(this));
  linkHandlers[PortTypes::backward_o] = 
    configureLink(PortNames.at(PortTypes::backward_o),
              new Event::Handler2<NNLayer, &NNLayer::backward_o_rcv>(this));
  linkHandlers[PortTypes::monitor] = 
    configureLink(PortNames.at(PortTypes::monitor),
    new Event::Handler2<NNLayer, &NNLayer::monitorEvent>(this));
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

bool NNLayer::clockTick( SST::Cycle_t currentCycle ) {
  if (driveForwardPass) {
    forward_o_snd();
    driveForwardPass=false;
  }
  if (driveMonitor) {
    tcldbg::spinner("SPINNER");
    monitor_snd();
    driveMonitor=false;
  }
  return false;
}

void NNLayer::forward_i_rcv(SST::Event *ev){
  NNEvent *nnev = static_cast<NNEvent*>(ev);
  auto data = nnev->getData();

  output.verbose(CALL_INFO,2,0, "%s: doubling %zu values from forward pass data\n",
    getName().c_str(),
    data.size());

  out.clear();
  for ( size_t i=0;i<data.size();i++) {
    out.emplace_back(2 * data[i]);
  }

  if (lastComponent)
    driveMonitor = true;
  else
    driveForwardPass = true;

  delete ev;
}

void NNLayer::backward_i_rcv(SST::Event *ev){
  assert(false);
  delete ev;
}

void NNLayer::forward_o_snd(){
  output.verbose(CALL_INFO,2,0, "%s sending forward pass data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(out);
  linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNLayer::monitor_snd() {
  output.verbose(CALL_INFO,0,0, "%s sending monitor data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(out);
  linkHandlers.at(PortTypes::monitor)->send(nnev);
}

} // namespace SST::NNLayer

// EOF
