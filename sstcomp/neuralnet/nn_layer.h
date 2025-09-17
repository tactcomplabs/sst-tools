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
// NNLayer
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
  payload_t monitorData_o = {};

  // -- SST handlers
  TimeConverter* timeConverter;
  SST::Clock::HandlerBase* clockHandler;

  // internals
  bool driveForwardPass = false;
  bool driveBackwardPass = false;
  bool driveMonitor = false;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNLayer() : NNLayerBase() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNLayer)

};  //class NNLayer

// -------------------------------------------------------
// NNInputLayer
// -------------------------------------------------------
class NNInputLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNInputLayer,   // Class name
        "neuralnet",    // Library name
        "NNInputLayer",   // Subcomponet name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  NNInputLayer(ComponentId_t id, Params& params) : NNSubComponentAPI(id,params) {};
  ~NNInputLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNInputLayer() : NNSubComponentAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNInputLayer)
}; //class NNInputLayer

// -------------------------------------------------------
// NNDenseLayer
// -------------------------------------------------------
class NNDenseLayer : public NNSubComponentAPI {
public:
  friend class NNAdamOptimizer;

  SST_ELI_REGISTER_SUBCOMPONENT(
    NNDenseLayer,   // Class name
    "neuralnet",    // Library name
    "NNDenseLayer",   // Subcomponent name
    SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
    "Neural network input layer.",     // Description
    SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  SST_ELI_DOCUMENT_PARAMS(
    {"biasRegularizerL1",    "L1 optimizer for biases",  "0" },
    {"biasRegularizerL2",    "L2 optimizer for biases",  "0" },
    {"initialWeightScaling", "scaling factor for random weights", "0.1"},
    {"nInputs",              "number of inputs",    "4" },
    {"nNeurons",             "number of neurons",   "128" },
    {"weightRegularizerL1",  "L1 optimizer for weights", "0" },
    {"weightRegularizerL2",  "L2 optimizer for weights", "0" },
  )

  NNDenseLayer(ComponentId_t id, Params& params);
  ~NNDenseLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
  void enable_weight_cache();
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
  double initial_weight_scaling = 0.1;
  Eigen::MatrixXd weights_ = {};
  Eigen::RowVectorXd biases_ = {};
  Eigen::MatrixXd predictions_ = {};
  // optimizer support  
  Eigen::MatrixXd weight_momentums = {};    // like weights
  Eigen::MatrixXd weight_cache = {};        // like weights
  Eigen::RowVectorXd bias_momentums = {};   // like biases
  Eigen::RowVectorXd bias_cache = {};       // like biases
  bool has_weight_cache = false;
  // derivatives
  Eigen::MatrixXd dweights_ = {};
  Eigen::RowVectorXd dbiases_ = {};

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNDenseLayer() : NNSubComponentAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNDenseLayer)

}; //class NNDenseLayer

// -------------------------------------------------------
// NNActivationReLULayer
// -------------------------------------------------------
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
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNActivationReLULayer() : NNSubComponentAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNActivationReLULayer)
}; //class NNActivationReLULayer

// -------------------------------------------------------
// NNActivationSoftmaxLayer
// -------------------------------------------------------
class NNActivationSoftmaxLayer : public NNSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
    NNActivationSoftmaxLayer,          // Class name
    "neuralnet",                       // Library name
    "NNActivationSoftmaxLayer",        // Subcomponent name
    SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
    "Neural network input layer.",     // Description
    SST::NeuralNet::NNSubComponentAPI) // Fully qualified API name
  SST_ELI_DOCUMENT_PARAMS({ 
    "lossType",
    "Next layer loss type. 0:MeanSquaredError 1:BinaryCrossEntroy 2:CategoricalCrossEntropy [2]",
    "2" })

  NNActivationSoftmaxLayer(ComponentId_t id, Params& params);
  ~NNActivationSoftmaxLayer() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;

  LOSS_TYPE loss_type() { return loss_type_; }

private:
  LOSS_TYPE loss_type_ = LOSS_TYPE::CATEGORICAL_CROSS_ENTROPY;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNActivationSoftmaxLayer() : NNSubComponentAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNActivationSoftmaxLayer)

}; //class NNActivationSoftmaxLayer

// -------------------------------------------------------
// NNLossLayer
// -------------------------------------------------------
class NNLoss_CategoricalCrossEntropy : public NNLossLayerAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        NNLoss_CategoricalCrossEntropy,    // Class name
        "neuralnet",                       // Library name
        "NNLoss_CategoricalCrossEntropy",  // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),    // A version number
        "Neural network input layer.",     // Description
        SST::NeuralNet::NNLossLayerAPI)    // Fully qualified API name
  NNLoss_CategoricalCrossEntropy(ComponentId_t id, Params& params) : NNLossLayerAPI(id,params) {};
  ~NNLoss_CategoricalCrossEntropy() {};
  virtual void forward(const payload_t& in, payload_t& o) final;
  virtual void backward(const payload_t& in, payload_t& o) final;
private:
  Eigen::MatrixXd negative_log_likelihoods_ = {}; // TODO remove
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNLoss_CategoricalCrossEntropy() : NNLossLayerAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNLoss_CategoricalCrossEntropy)
}; //class NNLossLayer

// -------------------------------------------------------
// NNAccuracyCategorical
// -------------------------------------------------------
class NNAccuracyCategorical : public NNAccuracyAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
    NNAccuracyCategorical,   // Class name
    "neuralnet",             // Library name
    "NNAccuracyCategorical", // Subcomponet name
    SST_ELI_ELEMENT_VERSION(1,0,0),
    "Neural network accuracy subcomponent.",
    SST::NeuralNet::NNAccuracyCategorical
  )

  NNAccuracyCategorical(ComponentId_t id, Params& params) : NNAccuracyAPI(id,params) {};
  ~NNAccuracyCategorical() {};
  Eigen::MatrixX<bool>& compare(const Eigen::MatrixXd& predictions, const Eigen::MatrixXd& y) final;

private:
  const bool binary_=false; //TODO input parameter
  const bool scalar_=false; //TODO input parameter
  Eigen::MatrixX<bool> result_ = {};

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNAccuracyCategorical() : NNAccuracyAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNAccuracyCategorical)

}; //class NNAccuracyCategorical

// -------------------------------------------------------
// NNAdamOptimizer
// -------------------------------------------------------
class NNAdamOptimizer : public NNOptimizerAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
    NNAdamOptimizer,   // Class name
    "neuralnet",       // Library name
    "NNAdamOptimizer", // Subcomponet name
    SST_ELI_ELEMENT_VERSION(1,0,0),
    "Neural network accuracy subcomponent.",
    SST::NeuralNet::NNAdamOptimizer
  )
  SST_ELI_DOCUMENT_PARAMS(
    {"decay",   "",  "0.0" },
    {"epsilon", "",  "1e-7" },
    {"beta_1",  "",  "0.9" },
    {"beta_2",  "",  "0.999" },
  )
        
  NNAdamOptimizer(ComponentId_t id, Params& params);
  ~NNAdamOptimizer() {};

  void pre_update_params() final;
  void update_params(NNDenseLayer* layer, const optimizer_data_t& opt) final;
  void post_update_params() final;

private:
  double decay_= 0.;
  double epsilon_ = 1e-7; 
  double beta_1_ = 0.9;
  double beta_2_ = 0.999;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNAdamOptimizer() : NNOptimizerAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNAdamOptimizer)
}; //class NNAccuracyCategorical


} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_H_

// EOF
