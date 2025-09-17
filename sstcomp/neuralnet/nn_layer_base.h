//
// _nn_layer_base_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_NN_LAYER_BASE_H_
#define _SST_NN_LAYER_BASE_H_

#include <vector>
#include "nn_event.h"
#include "eigen_utils.h"
#include "SST.h"

namespace SST::NeuralNet{

  enum REGULARIZATION { NONE, INCL };

  enum class ACTIVATION_TYPE : unsigned  {
    RELU = 0,
    SIGMOID = 1,
    SOFTMAX = 2
  };

  enum class LOSS_TYPE : unsigned {
    MEAN_SQUARED_ERROR = 0,
    BINARY_CROSS_ENTROPY = 1,
    CATEGORICAL_CROSS_ENTROPY = 2
  };

  enum class OPTIMIZER_TYPE : unsigned {
    INVALID = 0,
    ADAM = 1,
  };

// -------------------------------------------------------
// NNSubComponentAPI (not registered)
// -------------------------------------------------------
class NNSubComponentAPI : public SST::SubComponent 
{
public:
    // Tell SST that this class is a SubComponent API
    SST_ELI_REGISTER_SUBCOMPONENT_API(SST::NeuralNet::NNSubComponentAPI)
    SST_ELI_DOCUMENT_PARAMS(
      {"verbose",         "Sets the verbosity level of output",   "0" }
    )
    NNSubComponentAPI(ComponentId_t id, Params& params);
    virtual ~NNSubComponentAPI() {}

    virtual void forward(const payload_t& in, payload_t& o) = 0;
    virtual void backward(const payload_t& in, payload_t& o) = 0;

protected:
  // SST Handlers
  SST::Output sstout;
  // Flopped forward pass inputs
  Eigen::MatrixXd inputs_ = {};
  // Flopped backwas pass outputs (derivatives)
  Eigen::MatrixXd dinputs_ = {};

  //-- Helpers
  Eutils util = {};

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNSubComponentAPI() : SubComponent() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
};

// -------------------------------------------------------
// NNLossLayerAPI (not registered)
// -------------------------------------------------------
class NNLossLayerAPI : public NNSubComponentAPI 
{
public:
    // Tell SST that this class is a SubComponent API
    SST_ELI_REGISTER_SUBCOMPONENT_API(SST::NeuralNet::NNLossLayerAPI)
    SST_ELI_DOCUMENT_PARAMS(
      {"predictionType",    "Type of previous layer. 0:relu, 1:sigmoid, 2:softmax [default:2]", "2" }
    )

    NNLossLayerAPI(ComponentId_t id, Params& params);
    virtual ~NNLossLayerAPI() {}

    // Calculates the data and regularization losses - classification
    const Losses& calculate( Eigen::MatrixXd& sample_losses, REGULARIZATION include_regularization=NONE);
    // Calculates accumulated loss
    const Losses& calculated_accumulated(REGULARIZATION include_regularization=NONE);
    // Activation type of previous layer determines prediction calculation
    ACTIVATION_TYPE prediction_type() { return prediction_type_; }
    // Perform predications
    const Eigen::MatrixXd& predictions(const Eigen::MatrixXd& outputs);

protected:
    ACTIVATION_TYPE prediction_type_ = ACTIVATION_TYPE::SOFTMAX;
    Losses losses_ = {};
    Losses accumulated_losses_ = {};
    Eigen::MatrixXd predictions_ = {};
    double accumulated_sum_ = 0;
    int accumulated_count_ = 0;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNLossLayerAPI() : NNSubComponentAPI() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
};

// -------------------------------------------------------
// NNAccuracyAPI (not registered)
// -------------------------------------------------------
class NNAccuracyAPI : public SST::SubComponent { //TODO create common API for debug/output support
public:
  // Tell SST that this class is a SubComponent API
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::NeuralNet::NNAccuracyAPI)
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" }
  )
  NNAccuracyAPI(ComponentId_t id, Params& params);
  virtual ~NNAccuracyAPI() {}
  virtual Eigen::MatrixX<bool>& compare(const Eigen::MatrixXd& predictions, const Eigen::MatrixXd& y) = 0;
  double calculate(const Eigen::MatrixXd& predictions, const Eigen::MatrixXi& y);
  double calculate_accumulated();
  void new_pass();
protected:
  // SST Handlers
  SST::Output sstout;
private:
  double accumulated_sum_ = 0;
  double accumulated_count_ = 0;
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNAccuracyAPI() : SubComponent() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
};

// -------------------------------------------------------
// NNOptimizerAPI (not registered)
// -------------------------------------------------------
class NNDenseLayer;

class NNOptimizerAPI : public SST::SubComponent {
public:
  // Tell SST that this class is a SubComponent API
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::NeuralNet::NNOptimizerAPI)
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" }
  )

  NNOptimizerAPI(ComponentId_t id, Params& params);
  virtual ~NNOptimizerAPI() {}

  // Call once before any parameter updates
  virtual void pre_update_params() = 0;
  // Update parameters
  virtual void update_params(NNDenseLayer* layer, const optimizer_data_t& opt) = 0;
  // Call once after any parameter updates
  virtual void post_update_params() = 0;
  // Getters
  double current_learning_rate() { return current_learning_rate_; }
  unsigned iterations() { return iterations_; }
protected:
  SST::Output sstout;
  double learning_rate_ = 0.001;
  double current_learning_rate_ = 0.001;
  unsigned iterations_ = 0;
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNOptimizerAPI() : SubComponent() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

};

// -------------------------------------------------------
// NNLayerBase ( not registered )
// -------------------------------------------------------
class NNLayerBase : public SST::Component{

public:
  SST_ELI_REGISTER_COMPONENT_BASE(SST::NeuralNet::NNLayerBase)
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" },
    {"lastComponent",   "Indicate component is last layer",     "0" }
  )
  SST_ELI_DOCUMENT_PORTS(
    { "forward_i",  "forward pass input port",   {"neuralnet.NNevent"} },
    { "forward_o",  "forward pass output port",  {"neuralnet.NNevent"} },
    { "backward_i", "backward pass input port",  {"neuralnet.NNevent"} },
    { "backward_o", "backward pass output port", {"neuralnet.NNevent"} },
    { "monitor",    "monitoring port",           {"neuralnet.NNevent"} }
  )
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    { "transfer_function", 
      "Primary forward and backward pass operations",
      "SST::NeuralNet::NNSubComponentAPI" },
    // TODO LossLayerBase class?
    { "loss_function",
      "Loss calculations for final layer in forward pass",
      "SST::NeuralNet::NNLossLayerAPI" },
    { "accuracy_function",
      "Accuracy functions for final layer in forward pass",
      "SST::NeuralNet::NNAccuracyAPI" },
    // TODO DenseLayerBase class?
    { "optimizer",
      "Optimizer initial",
      "SST::NeuralNet::NNOptimizerAPI" }
  )

  explicit NNLayerBase(ComponentId_t id) : SST::Component(id) {}
  ~NNLayerBase() {}

  protected:
  // Subcomponent pointers
  NNSubComponentAPI* transfer_function = nullptr;
  NNLossLayerAPI* loss_function = nullptr;
  NNAccuracyAPI* accuracy_function = nullptr;
  NNOptimizerAPI* optimizer = nullptr;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNLayerBase() : SST::Component() {}
  // Serialization function 
  void serialize_order(SST::Core::Serialization::serializer& ser) override {
    Component::serialize_order(ser); 
  }

}; //class NNLayerBase


} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_BASE_H_

// EOF
