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
    Losses losses = loss_function->calculate(sampleLosses.X_batch);
    // Predictions and accuracy
    Eigen::MatrixXd predictions = loss_function->predictions(forwardData_i.X_batch);
    double accuracy = accuracy_function->calculate(predictions, forwardData_i.y_batch);

    std::cout << "### Forward pass result ###" << std::endl;
    std::cout << std::fixed << std::setprecision(3) 
      << "acc: " << accuracy
      << ", loss: "  << losses.total_loss()
      << " (data_loss: "  << losses.data_loss
      << ", reg_loss: "  << losses.regularization_loss << ")" << std::endl;

    /// TODO send loss/accuracy info for printing at end of step
    monitorData_o = forwardData_i;  // garbage
    monitor_snd();
    driveMonitor=false;
    // set up backward pass.
    assert(driveBackwardPass);
    backwardData_i = forwardData_i; // start backward pass.
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
  initial_weight_scaling = params.find<double>("initialWeightScaling", 0.1);

  std::cout << "### DenseLayer ###" << std::endl;
  std::cout << "n_inputs=" << n_inputs << std::endl;
  std::cout << "n_neurons=" << n_neurons << std::endl;
  std::cout << "initial_weight_scaling=" << initial_weight_scaling << std::endl;

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
  // save for backpropagation
  inputs_ = in.X_batch;
  // Calculate output values from inputs, weights and biases
  o.X_batch = inputs_ * weights_;
  o.X_batch = o.X_batch.rowwise() + biases_;

  std::cout << "### Layer_Dense.forward ###"  << std::endl;
  std::cout << std::fixed << std::setprecision(7);
  std::cout << "inputs"  << util.shapestr(inputs_)  << "=\n" << HEAD(inputs_)  << std::endl;
  std::cout << "weights" << util.shapestr(weights_) << "=\n" << HEAD(weights_) << std::endl;
  std::cout << "biases"  << util.shapestr(biases_)  << "=\n" << HEAD(biases_)  << std::endl;
  std::cout << "output"  << util.shapestr(o.X_batch)  << "=\n" << HEAD(o.X_batch)  << std::endl;

  //Complete paylod
  o.mode = in.mode;
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
  // save for backpropagation
  inputs_ = in.X_batch;
  // Calculation output values from input
  //# self.output = np.maximum(0,inputs)
  o.X_batch = in.X_batch.cwiseMax(0.0);

  std::cout << "### ReLU.forward ###" << std::endl;
  std::cout << "inputs=\n" << HEAD(in.X_batch) << std::endl;
  std::cout << "output=\n" << HEAD(o.X_batch) << std::endl;
  std::cout << "################################" << std::endl;

  // complete payload
  o.mode = in.mode;
  o.y_batch = in.y_batch;
}

void NNActivationReLULayer::backward(const payload_t& in, payload_t& o)
{
  o = in;
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
  inputs_ = in.X_batch;

  // Get unnormalized probabilities
  //# exp_values = np.exp(inputs - np.max(inputs, axis=1, keepdims=True))
  Eigen::VectorXd row_max = (in.X_batch.rowwise().maxCoeff()).reshaped(in.X_batch.rows(),1); 
  Eigen::MatrixXd exp_values = (in.X_batch.colwise() - row_max).array().exp();

  // Normalize them for each sample
  //# probabilities = exp_values / np.sum(exp_values, axis=1, keepdims=True)
  Eigen::VectorXd row_sum = (exp_values.rowwise().sum()).reshaped(exp_values.rows(), 1);
  Eigen::MatrixXd probabilities = exp_values.array().colwise() / row_sum.array();

  // Save result
  o.X_batch = probabilities;

  std::cout << "### Softmax.forward ###" << std::endl;
  std::cout << std::fixed << std::setprecision(7);
  std::cout << "inputs=\n" << HEAD(inputs_) << std::endl;
  // std::cout << "rowmax=\n" << HEAD(row_max) << std::endl;
  std::cout << "exp_values=\n" << HEAD(exp_values) << std::endl;
  // std::cout << "row_sum=\n" << HEAD(row_sum) << std::endl;
  std::cout << "output=\n" << HEAD(o.X_batch) << std::endl;
  std::cout << "################################" << std::endl;

  // complete payload
  o.mode = in.mode;
  o.y_batch = in.y_batch;
}

void NNActivationSoftmaxLayer::backward(const payload_t& in, payload_t& o)
{
  assert(loss_type_==LOSS_TYPE::CATEGORICAL_CROSS_ENTROPY);
  // Using optimized combined loss and software backward pass function

  Eigen::MatrixXd dvalues = in.X_batch; // TODO remove these deep copies.
  Eigen::MatrixXi y_true = in.y_batch;

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
}

//
// Loss Layer API
//

NNLossLayerAPI::NNLossLayerAPI(ComponentId_t id, Params &params) : NNSubComponentAPI(id,params)
{
  unsigned prediction_type = params.find<unsigned>("weightRegularizerL1", "2");
  assert(prediction_type <= 2);
  std::cout << "prediction_type=" << prediction_type << std::endl;
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

  std::cout << "### Loss.calculate ###" << std::endl;
  // std::cout << "output=\n" << HEAD(output) << std::endl;
  // std::cout << "y=\n" << HEAD(y) << std::endl;
  std::cout << "sample_losses=\n" << HEAD(sample_losses) << std::endl;
  std::cout << "data_loss=" << data_loss << std::endl;
  std::cout << "accumulated_sum=" << accumulated_sum_ << std::endl;
  std::cout << "accumulated_count=" << accumulated_count_ << std::endl;
  std::cout << "################################" << std::endl;

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
  // readability (temporary hopefully)
  #define y_pred in.X_batch
  #define y_true in.y_batch

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

  // Debug
  std::cout << "### Loss_CategoricalCrossentropy.forward ###" << std::endl;
  std::cout << "samples=" << samples << std::endl;
  std::cout << "y_pred=\n" << HEAD(y_pred) << std::endl;
  std::cout << "y_pred_clipped=\n" << HEAD(y_pred) << std::endl;
  std::cout << "y_true=\n" << HEAD(y_true)  << std::endl;
  std::cout << "correct_confidences=\n" << HEAD(correct_confidences) << std::endl;
  // std::cout << "correct_confidences.size()=" << correct_confidences.size() << std::endl;
  std::cout << "negative_log_likelihoods=\n" << HEAD(negative_log_likelihoods_) << std::endl;
  std::cout << "################################" << std::endl;

  // output payload
  o.mode = in.mode;
  o.X_batch = negative_log_likelihoods_;  // sample_losses
  o.y_batch = in.y_batch; // y_true
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

  std::cout << "### Accuracy.calculate" << std::endl;
  std::cout << "predictions=\n" << HEAD(predictions) << std::endl;
  std::cout << "comparisons=\n" << HEAD(comparisons) << std::endl;
  std::cout << "accuracy=" << accuracy << std::endl;
  std::cout << "accumulated_sum=" << accumulated_sum_ << std::endl;
  std::cout << "accumulated_count=" << accumulated_count_ << std::endl;

  // return accuracy
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

Eigen::MatrixX<bool>& NNAccuracyCategorical::compare(const Eigen::MatrixXd &predictions, const Eigen::MatrixXd &y)
{
  if (!binary_ && scalar_) {
    assert(false);
    // result_ = (predictions.array() == argmax(y).array());
  } else {
    result_ =  (predictions.array() == y.array());
  }
  return result_;
}

} // namespace SST::NNLayer

// EOF
