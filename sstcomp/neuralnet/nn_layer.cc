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
  sstout.init(
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
  forwardData_i = nnev->payload();
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
  backwardData_i = nnev->payload();
  driveBackwardPass = true;

  reregisterClock(timeConverter, clockHandler);
  delete ev;
}

void NNLayer::backward_o_snd() {
  sstout.verbose(CALL_INFO,2,0, "%s sending backward pass data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(backwardData_o);
  linkHandlers.at(PortTypes::backward_o)->send(nnev);
}

void NNLayer::forward_o_snd(){
  sstout.verbose(CALL_INFO,2,0, "%s sending forward pass data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(forwardData_o);
  linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNLayer::monitor_snd() {
  sstout.verbose(CALL_INFO,2,0, "%s sending monitor data\n", getName().c_str());
  NNEvent* nnev = new NNEvent(forwardData_o);
  linkHandlers.at(PortTypes::monitor)->send(nnev);
}

// 
// Input Layer
// 
void NNInputLayer::forward(const payload_t& in, payload_t& o)
{
  output_ = in.X_batch;
  o = in;
}

void NNInputLayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

//
// Dense Layer
//

NNDenseLayer::NNDenseLayer(ComponentId_t id, Params &params) : NNSubComponentAPI(id,params)
{ 
  // Configuration
  n_inputs = params.find<unsigned>("nInputs", "4");
  n_neurons = params.find<unsigned>("nNeurons", "128");

  std::cout << "### DenseLayer ###" << std::endl;
  std::cout << "n_inputs=" << n_inputs << std::endl;
  std::cout << "n_neurons=" << n_neurons << std::endl;

  // Regularization parameters
  weight_regularizer_l1_ = params.find<double>("weightRegularizerL1", "0");
  weight_regularizer_l2_ = params.find<double>("weightRegularizerL2", "0");
  bias_regularizer_l1_ = params.find<double>("biasRegularizerL1", "0");
  bias_regularizer_l2_ = params.find<double>("biasRegularizerL2", "0");

  // Initialize weights and biases
  bool normaldist = true;
  if (normaldist) {
      util.rand0to1normal(weights_, n_inputs, n_neurons, false);
      weights_ =  weights_ * INITIAL_WEIGHT_SCALING;
  } else {
      util.rand0to1flat(weights_, n_inputs, n_neurons);
      weights_ = weights_ * INITIAL_WEIGHT_SCALING;
  }
  biases_ = Eigen::RowVectorXd(n_neurons) = Eigen::RowVectorXd::Zero(n_neurons);
}

void NNDenseLayer::forward(const payload_t& in, payload_t& o)
{
  // save for backpropagation
  inputs_ = in.X_batch;
  // Calculate output values from inputs, weights and biases
  // output_ = inputs_ * weights_;
  // output_ = output_.rowwise() + biases_;

  std::cout << "### Layer_Dense.forward ###"  << std::endl;
  std::cout << std::fixed << std::setprecision(7);
  std::cout << "inputs"  << util.shapestr(inputs_)  << "=\n" << HEAD(inputs_)  << std::endl;
  std::cout << "weights" << util.shapestr(weights_) << "=\n" << HEAD(weights_) << std::endl;
  std::cout << "biases"  << util.shapestr(biases_)  << "=\n" << HEAD(biases_)  << std::endl;
  // std::cout << "output"  << util.shapestr(output_)  << "=\n" << HEAD(output_)  << std::endl;

  // Copy for transferring.
  o.mode = in.mode;
  o.X_batch = output_;
  o.y_batch = in.y_batch;

}

void NNDenseLayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

// 
// ReLU Activation Layer
// 
void NNActivationReLULayer::forward(const payload_t& in, payload_t& o)
{
  o = in;
}

void NNActivationReLULayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

// 
// Softmax Activation Layer
// 
void NNActivationSoftmaxLayer::forward(const payload_t& in, payload_t& o)
{
  o = in;
}

void NNActivationSoftmaxLayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

// 
// Loss Layer
// 
void NNLossLayer::forward(const payload_t& in, payload_t& o)
{
  o = in;
}

void NNLossLayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

} // namespace SST::NNLayer

// EOF
