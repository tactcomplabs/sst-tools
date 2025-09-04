//
// _nn_batch_controller_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <assert.h>
#include "nn_batch_controller.h"
#include "tcldbg.h"

namespace SST::NeuralNet{

//------------------------------------------
// NNBatchController
//------------------------------------------
NNBatchController::NNBatchController(SST::ComponentId_t id, const SST::Params& params ) :
  NNLayerBase( id )
{
  tcldbg::spinner("NNSPINNER");
  
  // parameters
  batch_size = params.find<unsigned>("batchSize", 128);
  classImageLimit = params.find<unsigned>("classImageLimit", 100000);
  epochs = params.find<unsigned>("epochs", 0);
  evalImageStr = params.find<std::string>("evalImage");
  print_every = params.find<unsigned>("printEvery", 100);
  testImagesStr = params.find<std::string>("testImages");
  trainingImagesStr = params.find<std::string>("trainingImages");
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );

  // SST output init
  output.init(
    "NNBatchController[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );

  // clocking 
  const std::string systemClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<NNBatchController,&NNBatchController::clockTick>(this);
  timeConverter = registerClock(systemClock, clockHandler);

  // Configure Links
  linkHandlers[PortTypes::forward_i] = 
    configureLink(PortNames.at(PortTypes::forward_i),
              new Event::Handler2<NNBatchController, &NNBatchController::forward_i_rcv>(this));
  linkHandlers[PortTypes::forward_o] = 
    configureLink(PortNames.at(PortTypes::forward_o),
              new Event::Handler2<NNBatchController, &NNBatchController::forward_o_rcv>(this));
  linkHandlers[PortTypes::backward_i] = 
    configureLink(PortNames.at(PortTypes::backward_i),
              new Event::Handler2<NNBatchController, &NNBatchController::backward_i_rcv>(this));
  linkHandlers[PortTypes::backward_o] = 
    configureLink(PortNames.at(PortTypes::backward_o),
              new Event::Handler2<NNBatchController, &NNBatchController::backward_o_rcv>(this));
  linkHandlers[PortTypes::monitor] = 
    configureLink(PortNames.at(PortTypes::monitor),
              new Event::Handler2<NNBatchController, &NNBatchController::monitor_rcv>(this));

  // Complete construction
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

NNBatchController::~NNBatchController(){
}

void NNBatchController::init( unsigned int phase ){

}

void NNBatchController::setup(){

  if (trainingImagesStr.size()>0) {
    trainingImages.load(trainingImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
    mode_sequence.push(MODE::TRAINING);
  }

  if (testImagesStr.size()>0) {
    testImages.load(testImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
    mode_sequence.push(MODE::VALIDATION);
  }

  if (evalImageStr.size()>0) {
    evalImage.load(evalImageStr.c_str(), 
      EigenImage::TRANSFORM::INVERT, 
      EigenImage::TRANSFORM::LINEARIZE, 
      true);
    mode_sequence.push(MODE::EVALUATION);
  }

  if (mode_sequence.empty())
    output.fatal(CALL_INFO, -1, "Nothing to do. Please set --trainingImages, --testImages, and/or --evalImage");

  // TODO use SST debug and mask features
  if (output.getVerboseLevel() >= 2 ) {
    std::cout << "X" << util.shapestr(trainingImages.data) << "=\n" << HEAD(trainingImages.data) << std::endl;
    std::cout << "y" << util.shapestr(trainingImages.classes) << "=\n" << HEAD(trainingImages.classes.transpose()) << std::endl;
    std::cout << "X_test"   << util.shapestr(testImages.data) << "=\n" << HEAD(testImages.data) << std::endl;
    std::cout << "y_test.T" << util.shapestr(testImages.classes) << ".T=\n" << HEAD(testImages.classes.transpose()) << std::endl;
  }
 
}

void NNBatchController::complete( unsigned int phase ){
}

void NNBatchController::finish(){
}

void NNBatchController::emergencyShutdown(){
}

void NNBatchController::printStatus( Output& out ){
}

void NNBatchController::forward_o_snd(MODE mode){
  output.verbose(CALL_INFO,2,0, 
    "%s sending %s forward pass data\n",
    getName().c_str(), mode2str.at(mode).c_str());
  NNEvent *nnev = new NNEvent({mode, batch_X, batch_y});
  linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNBatchController::backward_i_rcv(SST::Event *ev) {
  // check the backward data
  NNEvent* nnev = static_cast<NNEvent*>(ev);
  payload_t payload = nnev->payload();
  // double sum = payload.data.array().sum();
  // output.verbose(CALL_INFO,2,0, "%s step completed. checksum=%f\n", 
  //                 getName().c_str(), sum);
  optimizer_data_t opt = payload.optimizer_data;
  
  // Print a summary
  if ( (step % print_every) == 0 || step == (train_steps-1) ) {
    std::cout  << "epoch: " << epoch << ", step: " << step 
      << std::fixed << std::setprecision(3)
      << ", acc: "   << payload.accuracy
      << ", loss: "  << payload.losses.total_loss()
      << " (data_loss: "  << payload.losses.data_loss
      << ", reg_loss: "  << payload.losses.regularization_loss
      << std::setprecision(10)
      << ") ,lr: "    << opt.current_learning_rate
      << std::endl;
  }
       
  // Signal to send the next batch
  readyToSend = true;
  reregisterClock(timeConverter, clockHandler);
  delete(ev);
}

void NNBatchController::monitor_rcv(SST::Event *ev) {
  // output.verbose(CALL_INFO,2,0, "%s receiving monitor data\n", getName().c_str());
  // NNEvent* nnev = static_cast<NNEvent*>(ev);
  // double sum = nnev->payload().data.array().sum();
  // output.verbose(CALL_INFO,0,0, "Forward Pass Result=%f\n",sum);
  delete(ev);
}

bool NNBatchController::initTraining()
{
  output.verbose(CALL_INFO, 0, 0, "Starting training phase\n");
  
  // Initialize epoch counter
  epoch = 0;

  // Calculate number of steps
  unsigned rows = (unsigned) trainingImages.data.rows();
  if (batch_size > 0) {
    train_steps = rows / batch_size;
    // Dividing rounds down. If there are some remaining
    // data but not a full batch, this won't include it
    // Add `1` to include this not full batch
    if (train_steps * batch_size < rows)
      train_steps += 1;
  }

  output.verbose(CALL_INFO, 1, 0, "### Training setup\n");
  output.verbose(CALL_INFO, 1, 0, "epochs=%" PRId32 "\n", epochs);
  output.verbose(CALL_INFO, 1, 0, "X.rows()=%" PRId32 "\n", rows);
  output.verbose(CALL_INFO, 1, 0, "batch_size=%" PRId32 "\n", batch_size);
  output.verbose(CALL_INFO, 1, 0, "train_steps=%" PRId32 "\n", train_steps);

  assert(epochs > 0);

  // first training step
  // std::cout << "epoch: " << epoch << std::endl;
  return launchTrainingStep();
}

bool NNBatchController::stepTraining() {
  assert(step < train_steps);
  if (++step == train_steps) {
    // Done with steps. Check epoch
    output.verbose(CALL_INFO, 2,0, "Finished epoch\n");
    assert(epoch < epochs);
    if (++epoch == epochs) {
      output.verbose(CALL_INFO, 2, 0, "Completed training\n");
      // Release controller so it can go to next instruction
      busy=false;   // release controller
      return false; // keep clocking
    }
    // Next epoch
    // std::cout << "epoch: " << epoch << std::endl;
    // Reset accumulated values in loss and accuracy objects
    accumulatedAccuracy = {};
    accumulatedLoss = {};
    // Reset step counter
    step = 0;
  }
  // next step
  return launchTrainingStep();
  
}

bool NNBatchController::launchTrainingStep() {
  // If batch size is not set, train using one step and full dataset
  if (batch_size==0) {
      batch_X = trainingImages.data;
      batch_y = trainingImages.classes;
  } else { 
      // Otherwise slice a batch
      unsigned p = step*batch_size;
      assert(trainingImages.data.rows()==trainingImages.data.rows());
      unsigned r = std::min((unsigned)trainingImages.data.rows()-p, batch_size);
      batch_X = trainingImages.data.block(p, 0, r, trainingImages.data.cols());
      batch_y = trainingImages.classes.block(p, 0, r, trainingImages.classes.cols());
  }

  // std::cout << "batch_X" << util.shapestr(batch_X) << "=\n" << HEAD(batch_X) << std::endl;
  // std::cout << "batch_y" << util.shapestr(batch_y) << "=\n" << HEAD(batch_y) << std::endl;
  output.verbose(CALL_INFO, 2, 0, "batch_X %s\n", util.shapestr(batch_X).c_str());
  output.verbose(CALL_INFO, 2, 0, "batch_y %s\n", util.shapestr(batch_y).c_str());

  // Initiate the forward pass (backward pass included)
  output.verbose(CALL_INFO, 2, 0, "epoch:%" PRId32 " step:%" PRId32 "\n", epoch, step);
  forward_o_snd(MODE::TRAINING);
  busy = true;  // lock controller
  return true;  // disable controller clock
}

bool NNBatchController::initValidation()
{
  output.verbose(CALL_INFO, 2, 0, "Starting validation phase\n");

  // Calculate number of steps
  unsigned rows = (unsigned) testImages.data.rows();
  if (batch_size > 0) {
    validation_steps = (unsigned int) rows / batch_size;
    // Dividing rounds down. If there are some remaining
    // data but nor full batch, this won't include it
    // Add `1` to include this not full batch
    if (validation_steps * batch_size < rows)
      validation_steps += 1;
  }

  output.verbose(CALL_INFO, 1, 0, "### Validation setup\n");
  output.verbose(CALL_INFO, 1, 0, "X_val.rows()=%" PRId32 "\n", rows);
  output.verbose(CALL_INFO, 1, 0, "batch_size=%" PRId32 "\n", batch_size);
  output.verbose(CALL_INFO, 1, 0, "validation_steps=%" PRId32 "\n", validation_steps);
  return false; // TODO return true
}

bool NNBatchController::stepValidation()
{
  return false;
}

bool NNBatchController::initEvaluation()
{
  output.verbose(CALL_INFO, 0, 0, "Starting evaluation phase\n");
  return false;
}

bool NNBatchController::stepEvaluation()
{ 
  return false;
}


bool NNBatchController::clockTick( SST::Cycle_t currentCycle ) {
  // Clocking control should ensure we have something to do.
  assert( !busy || readyToSend);

  if (mode_sequence.empty()) {
    output.verbose(CALL_INFO, 2, 0,
                  "%s has nothing else to do. Ending simulation.\n",
                  getName().c_str());
    primaryComponentOKToEndSim();
    return true;
  }

  if (!busy) {
    assert(readyToSend==false); 
    // not busy so get next task
    current_mode = mode_sequence.front();
    mode_sequence.pop();

    switch (current_mode) {
      case MODE::TRAINING:
        return initTraining();
      case MODE::VALIDATION:
        return initValidation();
      case MODE::EVALUATION:
        return initEvaluation();
      default:
        assert(false);
    }
    assert(false);
    return false;
  }

  // OK we are busy so must be sending something.
  assert(readyToSend==true);
  readyToSend = false;

  switch (current_mode) {
    case MODE::TRAINING:
      return stepTraining();
    case MODE::VALIDATION:
      return stepValidation();
    case MODE::EVALUATION:
      return stepEvaluation();
    default:
      assert(false);
  }

  return false;
}

} // namespace SST::NNBatchController

// EOF
