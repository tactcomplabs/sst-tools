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
#include "nn_layer_base.h"

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
  if (lastComponent) {
    loss_function = loadUserSubComponent<NNLossLayerAPI>("loss_function");
    assert(loss_function);
    accuracy_function = loadUserSubComponent<NNAccuracyAPI>("accuracy_function");
    assert(accuracy_function);
  }
  transfer_function = loadUserSubComponent<NNSubComponentAPI>("transfer_function");
  assert(transfer_function);
  // optimizer associated with layers with weights only
  optimizer = loadUserSubComponent<NNOptimizerAPI>("optimizer");

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
    // Loss calculation at end of first pass
    assert(lastComponent);
    assert(loss_function);
    payload_t sampleLosses = {};
    loss_function->forward(forwardData_i, sampleLosses);
    // sampleLosses.X_batch is sample_losses
    // sampleLosses.y_batch is y_true
    Losses losses = loss_function->calculate(sampleLosses.data);
    // Predictions and accuracy
    Eigen::MatrixXd predictions = loss_function->predictions(forwardData_i.data);
    double accuracy = accuracy_function->calculate(predictions, forwardData_i.classes);

    if (sstout.getVerboseLevel() >= 2 ) {
      std::cout << "### Forward pass result ###" << std::endl;
      std::cout << std::fixed << std::setprecision(3) 
        << "acc: " << accuracy
        << ", loss: "  << losses.total_loss()
        << " (data_loss: "  << losses.data_loss
        << ", reg_loss: "  << losses.regularization_loss << ")" << std::endl;
    }

    /// TODO send loss/accuracy info for printing at end of step
    monitorData_o = forwardData_i;  // garbage
    monitor_snd();
    driveMonitor=false;
    // set up backward pass.
    backwardData_i = forwardData_i;
    backwardData_i.optimizer_data.optimizerState = OPTIMIZER_STATE::PRE_UPDATE;
    backwardData_i.accuracy = accuracy;
    backwardData_i.losses = losses;
    assert(driveBackwardPass);
  }
  if (driveBackwardPass) {
    // backward pass transfer function
    transfer_function->backward(backwardData_i, backwardData_o);
    // Optimizer for layers with weights
    if (optimizer) {
        // optimizer
        if (backwardData_o.optimizer_data.optimizerState == OPTIMIZER_STATE::PRE_UPDATE) {
          optimizer->pre_update_params();
          backwardData_o.optimizer_data = { 
            OPTIMIZER_STATE::ACTIVE, 
            optimizer->current_learning_rate(),
            optimizer->iterations() };
          optimizer->post_update_params();
        } else {
          assert(backwardData_o.optimizer_data.optimizerState == OPTIMIZER_STATE::ACTIVE);
          optimizer->update_params(static_cast<NNDenseLayer*>(transfer_function), backwardData_o.optimizer_data );
        }
    }
    // drive output
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
  o = in;
}

void NNInputLayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

//
// NNSubComponentAPI
//
NNSubComponentAPI::NNSubComponentAPI(ComponentId_t id, Params &params) : SubComponent(id)
{
  // parameters
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  sstout.init(
    "NNSubComponentAPI[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
}
//
// Dense Layer
//

NNDenseLayer::NNDenseLayer(ComponentId_t id, Params &params) : NNSubComponentAPI(id,params)
{ 
  // Configuration
  n_inputs = params.find<unsigned>("nInputs", "4");
  n_neurons = params.find<unsigned>("nNeurons", "128");
  initial_weight_scaling = params.find<double>("initialWeightScaling", 0.1);

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### DenseLayer ###" << std::endl;
    std::cout << "n_inputs=" << n_inputs << std::endl;
    std::cout << "n_neurons=" << n_neurons << std::endl;
    std::cout << "initial_weight_scaling=" << initial_weight_scaling << std::endl;
  }

  // Regularization parameters
  weight_regularizer_l1_ = params.find<double>("weightRegularizerL1", "0");
  weight_regularizer_l2_ = params.find<double>("weightRegularizerL2", "0");
  bias_regularizer_l1_   = params.find<double>("biasRegularizerL1", "0");
  bias_regularizer_l2_   = params.find<double>("biasRegularizerL2", "0");

  // Initialize weights and biases
  bool normaldist = true;
  if (normaldist) {
      util.rand0to1normal(weights_, n_inputs, n_neurons, false);
      weights_ =  weights_ * initial_weight_scaling;
  } else {
      util.rand0to1flat(weights_, n_inputs, n_neurons);
      weights_ = weights_ * initial_weight_scaling;
  }
  biases_ = Eigen::RowVectorXd(n_neurons) = Eigen::RowVectorXd::Zero(n_neurons);
}

void NNDenseLayer::forward(const payload_t& in, payload_t& o)
{
  // save for back propagation
  inputs_ = in.data;
  // Calculate output values from inputs, weights and biases
  o.data = inputs_ * weights_;
  o.data = o.data.rowwise() + biases_;

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Layer_Dense.forward ###"  << std::endl;
    std::cout << std::fixed << std::setprecision(7);
    std::cout << "inputs"  << util.shapestr(inputs_)  << "=\n" << HEAD(inputs_)  << std::endl;
    std::cout << "weights" << util.shapestr(weights_) << "=\n" << HEAD(weights_) << std::endl;
    std::cout << "biases"  << util.shapestr(biases_)  << "=\n" << HEAD(biases_)  << std::endl;
    std::cout << "output"  << util.shapestr(o.data)  << "=\n" << HEAD(o.data)  << std::endl;
  }

  // Complete payload
  o.copyWithNoData(in);
}

void NNDenseLayer::backward(const payload_t& in, payload_t& o)
{
  Eigen::MatrixXd dvalues = in.data; // TODO remove deep copy

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Layer_Dense.backward ###" << std::endl;
    std::cout << std::fixed << std::setprecision(7);
    std::cout << "dvalues.shape=" <<  util.shapestr(dvalues) << std::endl;
    std::cout << "dvalues.h=\n" << HEAD(dvalues.array()) << std::endl;
    std::cout << "dvalues.t=\n" << TAIL(dvalues.array()) << std::endl;
  }

  // Gradients on parameters
  //# self.dweights = self.inputs.T @ dvalues
  dweights_ = inputs_.transpose() * dvalues;
  // # self.dbiases = np.sum(dvalues, axis=0, keepdims=True)
  dbiases_ = dvalues.colwise().sum(); // .reshaped(1, dvalues.cols());

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << std::scientific << std::setprecision(7)
              << "dweights(0)=\n" << HEAD(dweights_.array()) 
              << "\ndbiases(0)=\n" << HEAD(dbiases_.array()) << std::endl;
  }

  // Gradients on regularization
  // L1 on weights
  if (weight_regularizer_l1_ > 0) {
    //# dL1 = np.ones_like(self.weights)
    Eigen::MatrixXd dL1 = Eigen::MatrixXd::Ones(weights_.rows(), weights_.cols());
    //# dL1[self.weights < 0] = -1
    dL1 = (weights_.array() < 0).select(-1, dL1.array());
    //# self.dweights += self.weight_regularizer_l1 * dL1
    dweights_ = dweights_.array() + weight_regularizer_l1_ * dL1.array();

    if (sstout.getVerboseLevel() >= 2) {
      std::cout << "dL1=\n" << HEAD(dL1.array()) << std::endl;
      std::cout << "dweights(l1)=\n" << HEAD(dweights_.array()) << std::endl;
    }
  }
  // L2 on weights
  if (weight_regularizer_l2_ > 0) {
    //# self.dweights += 2 * self.weight_regularizer_l2 * self.weights
    dweights_ += 2 * weight_regularizer_l2_ * weights_;
  }
  // L1 on biases
  if (bias_regularizer_l1_ > 0) {
    //# dL1 = np.ones_like(self.biases)
    Eigen::MatrixXd dL1 = Eigen::MatrixXd::Ones(biases_.rows(), biases_.cols());
    //# dL1[self.biases < 0] = -1
    dL1 = (biases_.array() < 0).select(-1, dL1.array());
    //# self.dbiases += self.bias_regularizer_l1 * dL1
    dbiases_ += bias_regularizer_l1_ * dL1;
  }
  // L2 on biases
  if (bias_regularizer_l2_ > 0) {
    //# self.dbiases += 2 * self.bias_regularizer_l2 * self.biases
    dbiases_ += 2.0 * bias_regularizer_l2_ * biases_;
  }

  // Gradient on values
  //# self.dinputs = dvalues @ self.weights.T
  dinputs_ = dvalues * weights_.transpose();

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "weights=\n"  << HEAD(weights_.array())  << std::endl;
    std::cout << "biases=\n"   << HEAD(biases_.array())   << std::endl;
    std::cout << "dweights=\n" << HEAD(dweights_.array()) << std::endl;
    std::cout << "dbiases=\n"  << HEAD(dbiases_.array())  << std::endl;
    if (weight_regularizer_l1_>0) std::cout << "weight_regularizer_l1_=" << weight_regularizer_l1_ << std::endl;
    if (weight_regularizer_l2_>0) std::cout << "weight_regularizer_l2_=" << weight_regularizer_l2_ << std::endl;
    if (bias_regularizer_l1_>0) std::cout << "bias_regularizer_l1=" << bias_regularizer_l1_ << std::endl;
    if (bias_regularizer_l2_>0) std::cout << "bias_regularizer_l2=" << bias_regularizer_l2_ << std::endl;
    std::cout << "################################" << std::endl;
  }

  // complete payload
  o.data = dinputs_;
  o.copyWithNoData(in);
}

void NNDenseLayer::enable_weight_cache() {
    weight_momentums = Eigen::MatrixXd::Zero(weights_.rows(), weights_.cols());
    weight_cache = Eigen::MatrixXd::Zero(weights_.rows(), weights_.cols());
    bias_momentums = Eigen::RowVectorXd::Zero(biases_.rows(), biases_.cols());
    bias_cache = Eigen::RowVectorXd::Zero(biases_.rows(), biases_.cols());
    has_weight_cache = true;
}

// 
// ReLU Activation Layer
// 
void NNActivationReLULayer::forward(const payload_t& in, payload_t& o)
{
  // save for backpropagation
  inputs_ = in.data;
  // Calculation output values from input
  //# self.output = np.maximum(0,inputs)
  o.data = in.data.cwiseMax(0.0);

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### ReLU.forward ###" << std::endl;
    std::cout << "inputs=\n" << HEAD(in.data) << std::endl;
    std::cout << "output=\n" << HEAD(o.data) << std::endl;
    std::cout << "################################" << std::endl;
  }

  // complete payload
  o.copyWithNoData(in);
}

void NNActivationReLULayer::backward(const payload_t& in, payload_t& o)
{
  // Internal copy
  dinputs_ = in.data;
  // Zero gradient where input values were negative
  //# self.dinputs[self.inputs <= 0] = 0
  Eigen::Array<bool, Eigen::Dynamic, Eigen::Dynamic> mask = (inputs_.array() <= 0).cast<bool>();
  Eigen::MatrixXd zeros(inputs_.rows(), inputs_.cols());
  zeros.setZero();
  dinputs_ = mask.select(zeros.array(), dinputs_);

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### ReLU.backward ###" << std::endl;
    std::cout << std::scientific << std::setprecision(7)
      << "inputs=\n"  << HEAD(inputs_)
      << "\ndvalues=\n"  << HEAD(in.data)
      << "\ndinputs=\n" << HEAD(dinputs_)
      << "\n################################" << std::endl;
  }

  // Complete payload
  o.data = dinputs_;
  o.copyWithNoData(in);
}

// 
// Softmax Activation Layer
// 

NNActivationSoftmaxLayer::NNActivationSoftmaxLayer(ComponentId_t id, Params& params)
  : NNSubComponentAPI(id,params) 
{
  unsigned loss_type = params.find<unsigned>("lossType", 2);
  assert(loss_type<=2);
  loss_type_ = static_cast<LOSS_TYPE>(loss_type);

  // only the combined software/crossentropy backward pass support for now
  assert(loss_type_==LOSS_TYPE::CATEGORICAL_CROSS_ENTROPY);
};

void NNActivationSoftmaxLayer::forward(const payload_t& in, payload_t& o)
{

  // Remember input values
  inputs_ = in.data;

  // Get unnormalized probabilities
  //# exp_values = np.exp(inputs - np.max(inputs, axis=1, keepdims=True))
  Eigen::VectorXd row_max = (in.data.rowwise().maxCoeff()).reshaped(in.data.rows(),1); 
  Eigen::MatrixXd exp_values = (in.data.colwise() - row_max).array().exp();

  // Normalize them for each sample
  //# probabilities = exp_values / np.sum(exp_values, axis=1, keepdims=True)
  Eigen::VectorXd row_sum = (exp_values.rowwise().sum()).reshaped(exp_values.rows(), 1);
  Eigen::MatrixXd probabilities = exp_values.array().colwise() / row_sum.array();

  // Save result
  o.data = probabilities;

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Softmax.forward ###" << std::endl;
    std::cout << std::fixed << std::setprecision(7);
    std::cout << "inputs=\n" << HEAD(inputs_) << std::endl;
    // std::cout << "rowmax=\n" << HEAD(row_max) << std::endl;
    std::cout << "exp_values=\n" << HEAD(exp_values) << std::endl;
    // std::cout << "row_sum=\n" << HEAD(row_sum) << std::endl;
    std::cout << "output=\n" << HEAD(o.data) << std::endl;
    std::cout << "################################" << std::endl;
  }

  // Complete payload
  o.copyWithNoData(in);
}

void NNActivationSoftmaxLayer::backward(const payload_t& in, payload_t& o)
{
  assert(loss_type_==LOSS_TYPE::CATEGORICAL_CROSS_ENTROPY);
  // Using optimized combined loss and software backward pass function

  Eigen::MatrixXd dvalues = in.data; // TODO remove these deep copies.
  Eigen::MatrixXi y_true = in.classes;

  // Number of samples
  auto samples = dvalues.rows();

  // If labels are one-hot encoded, turn them into discrete values
  if (!ISVECTOR(y_true)){
      // # y_true = np.argmax(y_true, axis=1)
      assert(false);
  }

  // Copy so we can safely modify
  dinputs_ = dvalues; // Eigen default deep copy
  // Calculate gradient
  // # self.dinputs[range(samples), y_true] -= 1
  for (int i = 0; i < samples; i++)
  {
      double v = dinputs_(i, y_true(i));
      dinputs_(i, y_true(i)) = v - 1;
  }

  // Normalize gradient
  // # self.dinputs = self.dinputs / samples
  dinputs_ = dinputs_.array() / samples;

  // Complete payload
  o.data = dinputs_;
  o.copyWithNoData(in);
}

//
// Loss Layer API
//

NNLossLayerAPI::NNLossLayerAPI(ComponentId_t id, Params &params) : NNSubComponentAPI(id,params)
{
  unsigned prediction_type = params.find<unsigned>("weightRegularizerL1", "2");
  assert(prediction_type <= 2);
  // std::cout << "prediction_type=" << prediction_type << std::endl;
  prediction_type_ = static_cast<ACTIVATION_TYPE>(prediction_type);
}

const Losses& NNLossLayerAPI::calculate(Eigen::MatrixXd& sample_losses, REGULARIZATION include_regularization)
{

  // Calculate the sample losses (called by parent)
  // Eigen::MatrixXd sample_losses = forward(output, y, debug);

  // Calculate the mean loss
  double data_loss = sample_losses.mean();

  // Add accumulated sum of losses and sample count
  accumulated_sum_ += sample_losses.sum();
  accumulated_count_ += (int) sample_losses.rows();

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Loss.calculate ###" << std::endl;
    // std::cout << "output=\n" << HEAD(output) << std::endl;
    // std::cout << "y=\n" << HEAD(y) << std::endl;
    std::cout << "sample_losses=\n" << HEAD(sample_losses) << std::endl;
    std::cout << "data_loss=" << data_loss << std::endl;
    std::cout << "accumulated_sum=" << accumulated_sum_ << std::endl;
    std::cout << "accumulated_count=" << accumulated_count_ << std::endl;
    std::cout << "################################" << std::endl;
  }

  double regularization_loss = 0;
  if (include_regularization) {
    assert(false); //TODO
    // regularization_loss = this->regularization_loss();
  }

  // Return loss
  losses_ = {data_loss, regularization_loss};
  return losses_;
}

const Losses& NNLossLayerAPI::calculated_accumulated(REGULARIZATION include_regularization)
{
  return losses_;
}

const Eigen::MatrixXd &NNLossLayerAPI::predictions(const Eigen::MatrixXd &outputs)
{
    assert(prediction_type_ == ACTIVATION_TYPE::SOFTMAX);
    util.argmax(predictions_, outputs);
    return predictions_;
}

// 
// NNLoss_CategoricalCrossEntropy
// 
void NNLoss_CategoricalCrossEntropy::forward(const payload_t& in, payload_t& o)
{
  // TODO remove these deep copies
  Eigen::MatrixXd y_pred = in.data;
  Eigen::MatrixXi y_true = in.classes;

  // Number of samples in a batch
  auto samples = y_pred.rows();
  // std::cout << "y_pred=\n" << HEAD(y_pred) << std::endl;

  // Clip data to prevent division by 0
  // Clip both sides to not drag mean towards any value
  //# y_pred_clipped = np.clip(y_pred, 1e-7, 1 - 1e-7)
  Eigen::MatrixXd y_pred_clipped = CLIP(y_pred.array(), 1e-7, 1-1e-7);
  // std::cout << "y_pred_clipped=\n" << HEAD(y_pred_clipped) << std::endl;

  // Probabilities for target values
  Eigen::MatrixXd correct_confidences(samples, 1);
  if (ISVECTOR(y_true)) {
      // only if categorical labels
      //# if len(y_true.shape) == 1:
      //#   correct_confidences = y_pred_clipped[
      //#       range(samples),
      //#       y_true
      //#   ]   
      for (int i=0; i<samples;i++) {
        // std::cout << "y_true[" << i << "]=" << y_true[i] << std::endl;
        correct_confidences(i,0) = y_pred_clipped(i,y_true(i,0));
      }
  } else {
      // Mask values - only for one-hot encoded labels
      //# elif len(y_true.shape) == 2:
      //#       correct_confidences = np.sum(
      //#           y_pred_clipped * y_true,
      //#           axis=1
      //#       )
      assert(false); // path not tested
  }
  // std::cout << "correct_confidences=\n" << HEAD(correct_confidences) << std::endl;

  // Losses
  //# negative_log_likelihoods = -np.log(correct_confidences)
  negative_log_likelihoods_ = -correct_confidences.array().log();

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Loss_CategoricalCrossentropy.forward ###" << std::endl;
    std::cout << "samples=" << samples << std::endl;
    std::cout << "y_pred=\n" << HEAD(y_pred) << std::endl;
    std::cout << "y_pred_clipped=\n" << HEAD(y_pred) << std::endl;
    std::cout << "y_true=\n" << HEAD(y_true)  << std::endl;
    std::cout << "correct_confidences=\n" << HEAD(correct_confidences) << std::endl;
    // std::cout << "correct_confidences.size()=" << correct_confidences.size() << std::endl;
    std::cout << "negative_log_likelihoods=\n" << HEAD(negative_log_likelihoods_) << std::endl;
    std::cout << "################################" << std::endl;
  }

  // output payload
  o.data = negative_log_likelihoods_;  // sample_losses
  o.copyWithNoData(in);
}

void NNLoss_CategoricalCrossEntropy::backward(const payload_t& in, payload_t& o)
{
  o = in;
}

// 
// NNAccuracyAPI
//

double NNAccuracyAPI::calculate(const Eigen::MatrixXd& predictions, const Eigen::MatrixXi& y) {
  // Get comparison results
  Eigen::MatrixXd yy = y.cast<double>();
  Eigen::MatrixX<bool> comparisons = this->compare(predictions, yy);
  // Calculate an accuracy
  double accuracy = comparisons.array().cast<double>().mean();
  // Add accumulated sum of matching values and sample count
  accumulated_sum_ += comparisons.cast<double>().sum();
  accumulated_count_ += (double) comparisons.rows(); //TODO change accumulated_count_ to long

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Accuracy.calculate" << std::endl;
    std::cout << "predictions=\n" << HEAD(predictions) << std::endl;
    std::cout << "comparisons=\n" << HEAD(comparisons) << std::endl;
    std::cout << "accuracy=" << accuracy << std::endl;
    std::cout << "accumulated_sum=" << accumulated_sum_ << std::endl;
    std::cout << "accumulated_count=" << accumulated_count_ << std::endl;
  }

  return accuracy;
}

double NNAccuracyAPI::calculate_accumulated() {
  double accuracy = accumulated_sum_ / accumulated_count_;
  return accuracy;
}

void NNAccuracyAPI::new_pass() {
  accumulated_sum_ = 0;
  accumulated_count_ = 0;
}

// 
// NNAccuracyCategorical
//

NNAccuracyAPI::NNAccuracyAPI(ComponentId_t id, Params &params) : SubComponent(id)
{
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  sstout.init(
    "NNAccuracyAPI[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
}

Eigen::MatrixX<bool> &NNAccuracyCategorical::compare(const Eigen::MatrixXd &predictions, const Eigen::MatrixXd &y)
{
  if (!binary_ && scalar_) {
    assert(false);
    // result_ = (predictions.array() == argmax(y).array());
  } else {
    result_ =  (predictions.array() == y.array());
  }
  return result_;
}

//
// NNOptimizerAPI
//
NNOptimizerAPI::NNOptimizerAPI(ComponentId_t id, Params &params) 
  : SubComponent(id)
{
  // parameters
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  sstout.init(
    "NNOptimizerAPI[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );

  learning_rate_ = params.find<double>("learningRate", "0.001");
  assert(learning_rate_>0);
  current_learning_rate_ = learning_rate_;
}

//
// NNAdamOptimizer
//
NNAdamOptimizer::NNAdamOptimizer(ComponentId_t id, Params &params) : NNOptimizerAPI(id,params)
{
  decay_   = params.find<double>("decay", "0");
  epsilon_ = params.find<double>("epsilon", "1e-7");
  beta_1_  = params.find<double>("beta_1", "0.9");
  beta_2_  = params.find<double>("beta_2", "0.999");
}

void NNAdamOptimizer::pre_update_params() {
  if (decay_ != 0) {
    current_learning_rate_ = learning_rate_ * ( 1. / (1. + decay_ * iterations_ ));
  }
}

void NNAdamOptimizer::update_params(NNDenseLayer *layer, const optimizer_data_t& opt) {

  assert(layer);

  // Unlike the centralized optimizer in the functional model, 
  // the current learning rate and iterations must be passed in from the backward pass information
  current_learning_rate_ = opt.current_learning_rate;
  iterations_ = opt.iterations;
  assert(opt.optimizerState == OPTIMIZER_STATE::ACTIVE);

  // If layer does not contain cache arrays, create them filled with zeros
  if (! layer->has_weight_cache)
      layer->enable_weight_cache();

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "### Optimizer_Adam.update_params ###" << std::endl;
    std::cout << "beta_1=" << beta_1_ << std::endl;
    std::cout << "weight_momentums(i)=\n" << HEAD(layer->weight_momentums) << std::endl;
    std::cout << "bias_momentums(i)=\n" << HEAD(layer->bias_momentums) << std::endl;
  }

  // Update momentum  with current gradients
  layer->weight_momentums = beta_1_ * layer->weight_momentums.array()
                  + ( 1. - beta_1_) * layer->dweights_.array();
  layer->bias_momentums = beta_1_ * layer->bias_momentums.array()
                  + ( 1. - beta_1_) * layer->dbiases_.array();

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "weight_momentums(f)=\n" << HEAD(layer->weight_momentums) << std::endl;
    std::cout << "bias_momentums(f)=\n" << HEAD(layer->bias_momentums) << std::endl;
  }

  // Get corrected momentum
  // iteration is 0 at first pass and we need to start with 1 here
  Eigen::MatrixXd weight_momentums_corrected = 
      layer->weight_momentums.array() / ( 1. - pow(beta_1_ ,(iterations_ + 1.)) );
  Eigen::MatrixXd bias_momentums_corrected = 
      layer->bias_momentums.array() / ( 1. - pow(beta_1_ ,(iterations_ + 1.)) );

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "iterations=" << iterations_ << std::endl;
    std::cout << "weight_momentums_corrected=\n" << HEAD(weight_momentums_corrected) << std::endl;
    std::cout << "bias_momentums_corrected=\n" << HEAD(bias_momentums_corrected) << std::endl;
    std::cout << "beta_2=" << beta_2_ << std::endl;
    std::cout << "weight_cache(i)=\n" << HEAD(layer->weight_cache) << std::endl;
    std::cout << "bias_cache(i)=\n" << HEAD(layer->bias_cache) << std::endl;
  }

  // Update cache with squared current gradients
  layer->weight_cache = beta_2_ * layer->weight_cache.array() + 
      (1 - beta_2_) * layer->dweights_.array().pow(2);
  layer->bias_cache = beta_2_ * layer->bias_cache.array() + 
      (1 - beta_2_) * layer->dbiases_.array().pow(2);

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "weight_cache(f)=\n" << HEAD(layer->weight_cache) << std::endl;
    std::cout << "bias_cache(f)=\n" << HEAD(layer->bias_cache) << std::endl; 
  }  

  // Get correct cache
  Eigen::MatrixXd weight_cache_corrected = layer->weight_cache /
          (1 - pow(beta_2_, (iterations_ + 1)));
  Eigen::MatrixXd bias_cache_corrected = layer->bias_cache /
          (1 - pow(beta_2_, (iterations_ + 1)));

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "weight_cache_corrected=\n" << HEAD(weight_cache_corrected) << std::endl;
    std::cout << "bias_cache_corrected=\n" << HEAD(bias_cache_corrected) << std::endl;   
    std::cout << "current_learning_rate" << current_learning_rate_ << std::endl;
    std::cout << "weights_(i)=\n" << HEAD(layer->weights_) << std::endl;
    std::cout << "biases(i)=\n" << HEAD(layer->biases_) << std::endl; 
  }  

  // Vanilla SGD parameter update + normalization
  // with square rooted cache
  layer->weights_ = layer->weights_.array() - current_learning_rate_ *
                      weight_momentums_corrected.array() /
                      (weight_cache_corrected.array().sqrt() + epsilon_);
  layer->biases_ = layer->biases_.array() - current_learning_rate_ *
                      bias_momentums_corrected.array() /
                      (bias_cache_corrected.array().sqrt()+
                      epsilon_);

  if (sstout.getVerboseLevel() >= 2) {
    std::cout << "weights_(f)=\n" << HEAD(layer->weights_) << std::endl;
    std::cout << "biases(f)=\n" << HEAD(layer->biases_) << std::endl;
  }

}

void NNAdamOptimizer::post_update_params() {
  iterations_++;
}

} // namespace SST::NNLayer

// EOF
