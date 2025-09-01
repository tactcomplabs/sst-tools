//
// _nn_layer_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_NN_LAYER_H_
#define _SST_NN_LAYER_H_

// clang-format off
#include <map>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "nn_layer_base.h"
#include "nn_event.h"
// clang-format on

namespace SST::NeuralNet{

// -------------------------------------------------------
// NNLayer (registered)
// -------------------------------------------------------
class NNLayer : public NNLayerBase {
public:
  NNLayer( SST::ComponentId_t id, const SST::Params& params );
  ~NNLayer();

  // Component Lifecycle
  void init( unsigned int phase ) override;     // post-construction, polled events
  void setup() override;                        // pre-simulation, called once per component
  void complete( unsigned int phase ) override; // post-simulation, polled events
  void finish() override;                       // pre-destruction, called once per component
  void emergencyShutdown() override;            // SIGINT, SIGTERM
  void printStatus(Output& out) override;       // SIGUSR2

  // Clocking
  bool clockTick( SST::Cycle_t currentCycle );  // return true if clock should be disabled

  SST_ELI_REGISTER_COMPONENT( NNLayer,     // component class
                              "neuralnet", // component library
                              "NNLayer",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "NNLayer SST Component",
                              COMPONENT_CATEGORY_UNCATEGORIZED )

private:
  // SST handlers
  SST::Output    sstout; 
  // event handling
  std::map<SST::NeuralNet::PortTypes,SST::Link*> linkHandlers = {};
  void forward_i_snd() { assert(false); }
  void forward_i_rcv(SST::Event *ev);
  void forward_o_snd();
  void forward_o_rcv(SST::Event *ev) { assert(false);}
  void backward_i_snd() { assert(false); }
  void backward_i_rcv(SST::Event *ev);
  void backward_o_snd();
  void backward_o_rcv(SST::Event *ev) { assert(false); }
  void monitorEvent(SST::Event *ev) { assert(false); }
  void monitor_rcv(SST::Event *ev) {assert(false); };
  void monitor_snd();
  // Internals
  bool lastComponent = false;
  payload_t forwardData_i = {};
  payload_t forwardData_o = {};
  payload_t backwardData_i = {};
  payload_t backwardData_o = {};

  // -- SST handlers
  TimeConverter* timeConverter;
  SST::Clock::HandlerBase* clockHandler;

  // internals
  bool driveForwardPass = false;
  bool driveBackwardPass = false;
  bool driveMonitor = false;

};  //class NNLayer

class NNInputLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNInputLayer,   // Class name
        "neuralnet",    // Library name
        "NNInputLayer",   // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  NNInputLayer(ComponentId_t id, Params& params) : NNSubComponentAPI(id,params) {};
  ~NNInputLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
}; //class NNInputLayer

class NNDenseLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNDenseLayer,   // Class name
        "neuralnet",    // Library name
        "NNDenseLayer",   // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  SST_ELI_DOCUMENT_PARAMS(
    {"nInputs",             "number of inputs",    "4" },
    {"nNeurons",            "number of neurons",   "128" },
    {"weightRegularizerL1", "L1 optimizer for weights", "0" },
    {"weightRegularizerL2", "L2 optimizer for weights", "0" },
    {"biasRegularizerL1",   "L1 optimizer for biases",  "0" },
    {"biasRegularizerL2",   "L2 optimizer for biases",  "0" })
    
  NNDenseLayer(ComponentId_t id, Params& params);
  ~NNDenseLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
private:
  // Configuration
  unsigned n_inputs = 4;
  unsigned n_neurons = 128;
  // regularization (trainable layers only)
  double weight_regularizer_l1_ = 0;
  double weight_regularizer_l2_ = 0;
  double bias_regularizer_l1_ = 0;
  double bias_regularizer_l2_ = 0;
  // Weights and Biases
  const double INITIAL_WEIGHT_SCALING = 0.1;
  Eigen::MatrixXd weights_ = {};
  Eigen::RowVectorXd biases_ = {};
  Eigen::MatrixXd predictions_ = {};
  // optimizer support  
  Eigen::MatrixXd weight_momentums = {};    // like weights
  Eigen::MatrixXd weight_cache = {};        // like weights
  Eigen::RowVectorXd bias_momentums = {};   // like biases
  Eigen::RowVectorXd bias_cache = {};       // like biases
  // derivatives
  Eigen::MatrixXd dweights_ = {};
  Eigen::RowVectorXd dbiases_ = {};
}; //class NNDenseLayer

class NNActivationReLULayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNActivationReLULayer,   // Class name
        "neuralnet",    // Library name
        "NNActivationReLULayer",   // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  NNActivationReLULayer(ComponentId_t id, Params& params) : NNSubComponentAPI(id,params) {};
  ~NNActivationReLULayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
}; //class NNActivationReLULayer

class NNActivationSoftmaxLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNActivationSoftmaxLayer,   // Class name
        "neuralnet",    // Library name
        "NNActivationSoftmaxLayer",   // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  NNActivationSoftmaxLayer(ComponentId_t id, Params& params) : NNSubComponentAPI(id,params) {};
  ~NNActivationSoftmaxLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
}; //class NNActivationSoftmaxLayer

class NNLossLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNLossLayer,   // Class name
        "neuralnet",    // Library name
        "NNLossLayer",   // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  NNLossLayer(ComponentId_t id, Params& params) : NNSubComponentAPI(id,params) {};
  ~NNLossLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
}; //class NNLossLayer

} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_H_

// EOF
