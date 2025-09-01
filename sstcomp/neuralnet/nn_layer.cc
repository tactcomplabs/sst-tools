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
  // an event will wake up the clocking
  unregisterClock(timeConverter, clockHandler);

  // subcomponents
  transfer_function = loadUserSubComponent<NNSubComponentAPI>("transfer_function");
  assert(transfer_function);

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
  // Clocking control should ensure we always have something to do here
  assert(driveForwardPass || driveMonitor || driveBackwardPass);
  if (driveForwardPass) {
    transfer_function->forward(forwardData_i, forwardData_o);
    forward_o_snd();
    driveForwardPass=false;
  }
  if (driveMonitor) {
    transfer_function->forward(forwardData_i, forwardData_o);
    backwardData_i.resize(forwardData_o.size());
    backwardData_i = forwardData_o;
    monitor_snd();
    driveMonitor=false;
  }
  if (driveBackwardPass) {
    transfer_function->backward(backwardData_i, backwardData_o);
    backward_o_snd();
    driveBackwardPass=false;
  }
  return true;
}

void NNLayer::forward_i_rcv(SST::Event *ev){
  NNEvent *nnev = static_cast<NNEvent*>(ev);
  forwardData_i = nnev->getData();
  if (lastComponent) {
    driveMonitor = true;
    driveBackwardPass = true;
  }
  else
    driveForwardPass = true;

  reregisterClock(timeConverter, clockHandler);
  delete ev;
}

void NNLayer::backward_i_rcv(SST::Event *ev){
  NNEvent *nnev = static_cast<NNEvent*>(ev);
  backwardData_i = nnev->getData();
  driveBackwardPass = true;

  reregisterClock(timeConverter, clockHandler);
  delete ev;
}

void NNLayer::backward_o_snd() {
  output.verbose(CALL_INFO,2,0, "%s sending backward pass data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(backwardData_o);
  linkHandlers.at(PortTypes::backward_o)->send(nnev);
}

void NNLayer::forward_o_snd(){
  output.verbose(CALL_INFO,2,0, "%s sending forward pass data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(forwardData_o);
  linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNLayer::monitor_snd() {
  output.verbose(CALL_INFO,2,0, "%s sending monitor data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(forwardData_o);
  linkHandlers.at(PortTypes::monitor)->send(nnev);
}

// 
// Input Layer
// 
void NNInputLayer::forward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

void NNInputLayer::backward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

// 
// Dense Layer
// 
void NNDenseLayer::forward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

void NNDenseLayer::backward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

// 
// ReLU Activation Layer
// 
void NNActivationReLULayer::forward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

void NNActivationReLULayer::backward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

// 
// Softmax Activation Layer
// 
void NNActivationSoftmaxLayer::forward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

void NNActivationSoftmaxLayer::backward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

// 
// Loss Layer
// 
void NNLossLayer::forward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

void NNLossLayer::backward(const std::vector<uint64_t>& in, std::vector<uint64_t>& o)
{
  if (in.size() != o.size())
    o.resize(in.size());
  o = in;
}

} // namespace SST::NNLayer

// EOF
