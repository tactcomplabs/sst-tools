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
  evalImagesStr = params.find<std::string>("evalImages");
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

  if (!enableTraining() && !enableValidation() && !enableEvaluation())
    output.fatal(CALL_INFO, -1, "Nothing to do. Please set --trainingImages, --testImages, and/or --evalImages");

  if (enableTraining()) {
    trainingImages.load(trainingImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
    if (output.getVerboseLevel() >= 2 ) {
      std::cout << "X" << util.shapestr(trainingImages.data) << "=\n" << HEAD(trainingImages.data) << std::endl;
      std::cout << "y" << util.shapestr(trainingImages.classes) << "=\n" << HEAD(trainingImages.classes.transpose()) << std::endl;
    }
    fsmState = MODE::TRAINING;
  }

  if (enableValidation()) {
    testImages.load(testImagesStr, Dataset::DTYPE::IMAGE, classImageLimit, true);
    if (output.getVerboseLevel() >= 2 ) {
      std::cout << "X_test"   << util.shapestr(testImages.data) << "=\n" << HEAD(testImages.data) << std::endl;
      std::cout << "y_test.T" << util.shapestr(testImages.classes) << ".T=\n" << HEAD(testImages.classes.transpose()) << std::endl;
    }
    if (fsmState==MODE::INVALID) fsmState = MODE::VALIDATION;
  }

  if (enableEvaluation()) {
    evalImages.load_eval_images(evalImagesStr.c_str(), EigenImage::TRANSFORM::INVERT, EigenImage::TRANSFORM::LINEARIZE, true);
    if (output.getVerboseLevel() >= 2 ) {
      std::cout << "X_eval"   << util.shapestr(evalImages.data) << "=\n" << HEAD(evalImages.data) << std::endl;
    }
    if (fsmState==MODE::INVALID) fsmState = MODE::EVALUATION;
  }
 
}

void NNBatchController::complete( unsigned int phase ){}

void NNBatchController::finish(){}

void NNBatchController::emergencyShutdown(){}

void NNBatchController::printStatus( Output& out ){}

void NNBatchController::forward_o_snd(MODE mode)
{
    output.verbose(CALL_INFO, 2, 0,
                   "%s sending %s forward pass data\n",
                   getName().c_str(), mode2str.at(mode).c_str());
    NNEvent *nnev = new NNEvent({mode, batch_X, batch_y});
    linkHandlers.at(PortTypes::forward_o)->send(nnev);
}

void NNBatchController::backward_i_rcv(SST::Event *ev) {
  // check the backward data
  NNEvent* nnev = static_cast<NNEvent*>(ev);
  payload_t payload = nnev->payload();
  assert(payload.mode == MODE::TRAINING);

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

  // Get and print epoch loss and accuracy
  accumulatedSums.count++;
  accumulatedSums.accuracy += payload.accuracy;
  accumulatedSums.loss.data_loss += payload.losses.data_loss;
  accumulatedSums.loss.regularization_loss += payload.losses.regularization_loss; 
  accumulatedSums.current_learning_rate = opt.current_learning_rate;
  
  // Signal to send the next batch
  readyToSend = true;
  reregisterClock(timeConverter, clockHandler);
  delete(ev);
}

void NNBatchController::monitor_rcv(SST::Event *ev) {
  NNEvent* nnev = static_cast<NNEvent*>(ev);
  monitor_payload = nnev->payload();
  output.verbose(CALL_INFO,2,0, "Monitor Data Received: %s\n", monitor_payload.str().c_str());

  // Signal to send the next batch
  MODE mode = monitor_payload.mode;
  if (mode == MODE::VALIDATION) {
    assert(mode == MODE::EVALUATION || mode == MODE::VALIDATION);
    
    accumulatedSums.count++;
    accumulatedSums.accuracy += monitor_payload.accuracy;
    accumulatedSums.loss.data_loss += monitor_payload.losses.data_loss;
    accumulatedSums.loss.regularization_loss += monitor_payload.losses.regularization_loss; 
    accumulatedSums.current_learning_rate = monitor_payload.optimizer_data.current_learning_rate;
  
    readyToSend = true;
    reregisterClock(timeConverter, clockHandler);
  } else if (mode == MODE::EVALUATION) {
    // std::cout << "predictions=" << monitor_payload.predictions.transpose() << std::endl;
    readyToSend = true;
    reregisterClock(timeConverter, clockHandler);
  } else {
    assert(mode == MODE::TRAINING);
  }
  delete(ev);
}

bool NNBatchController::initTraining()
{
  output.verbose(CALL_INFO, 0, 0, "Starting training phase\n");
  
  // Initialize epoch counter
  epoch = 0;
  accumulatedSums = {};
  step = 0;

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

  // First training step
  return launchTrainingStep();
}

bool NNBatchController::stepTraining() {
  assert(step < train_steps);
  if (++step == train_steps) {
    // Finished all steps for epoch
    output.verbose(CALL_INFO, 2,0, "Finished epoch\n");
    assert(epoch < epochs);
    if (++epoch == epochs) {
      trainingComplete = true;
    }
    
    // At end of epoch. Print accumulated loss and accuracy
    Losses epoch_losses;
    epoch_losses.data_loss = accumulatedSums.loss.data_loss / accumulatedSums.count;
    epoch_losses.regularization_loss = accumulatedSums.loss.regularization_loss / accumulatedSums.count;
    double epoch_loss = epoch_losses.total_loss();
    double epoch_accuracy = accumulatedSums.accuracy / accumulatedSums.count;

    std::cout  << "training"
        << std::fixed << std::setprecision(3)
        << ", acc: "   << epoch_accuracy
        << ", loss: "  << epoch_loss
        << " (data_loss: "  << epoch_losses.data_loss
        << ", reg_loss: "  << epoch_losses.regularization_loss
        << std::setprecision(10)
        << ") ,lr: "    << accumulatedSums.current_learning_rate
        << std::endl;

    // Switch to validation mode if enabled before next training epoch.
    if (enableValidation()) {
      std::cout << "### Validating model" << std::endl;
      fsmState = MODE::VALIDATION;
      step=0;     // reset counter
      busy=false; // release controller
      return false;
    } 

    if (trainingComplete) {
      // only get here if no validation images provided.
      assert(false); //TODO remove this after confirming operation
      std::cout << "\nTraining done\n" << std::endl;
      busy=false;   // release controller
      return false; // keep clocking
    }

    // Next epoch
    return continueTraining();
  }

  // next step
  return launchTrainingStep();
}

bool NNBatchController::continueTraining()
{
    fsmState = MODE::TRAINING;
    // Next epoch
    std::cout << "epoch: " << epoch << std::endl;
    // Reset accumulated values in loss and accuracy objects
    accumulatedSums = {};
    // Reset step counter
    step = 0;
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
      assert(r);
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

bool NNBatchController::launchValidationStep() {
  // If batch size is not set, train using one step and full dataset
  if (batch_size==0) {
      batch_X = testImages.data;
      batch_y = testImages.classes;
  } else { 
      // Otherwise slice a batch
      unsigned p = step*batch_size;
      assert(testImages.data.rows()==testImages.data.rows());
      unsigned r = std::min((unsigned)testImages.data.rows()-p, batch_size);
      assert(r);
      batch_X = testImages.data.block(p, 0, r, testImages.data.cols());
      batch_y = testImages.classes.block(p, 0, r, testImages.classes.cols());
  }

  // std::cout << "batch_X" << util.shapestr(batch_X) << "=\n" << HEAD(batch_X) << std::endl;
  // std::cout << "batch_y" << util.shapestr(batch_y) << "=\n" << HEAD(batch_y) << std::endl;
  output.verbose(CALL_INFO, 2, 0, "batch_X %s\n", util.shapestr(batch_X).c_str());
  output.verbose(CALL_INFO, 2, 0, "batch_y %s\n", util.shapestr(batch_y).c_str());

  // Initiate the forward pass (completed on monitor_rcv)
  output.verbose(CALL_INFO, 2, 0, "step:%" PRId32 "\n", step);
  forward_o_snd(MODE::VALIDATION);
  busy = true;  // lock controller
  return true;  // disable controller clock
}

bool NNBatchController::initValidation() {
  output.verbose(CALL_INFO, 2, 0, "Starting validation phase\n");
  fsmState = MODE::VALIDATION;
  accumulatedSums = {};
  step=0;

  // Calculate number of steps
  validation_steps = 1;
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
  
  // First validation step
  return launchValidationStep();
}

bool NNBatchController::stepValidation() {
  assert(step < validation_steps);
  if (++step == validation_steps) {
    // Finished all validation steps.
    validationComplete = true;
    output.verbose(CALL_INFO, 2, 0, "Completed validation\n");

    // Get and print validation loss and accuracy
    Losses validation_losses;
    validation_losses.data_loss = accumulatedSums.loss.data_loss / accumulatedSums.count;
    validation_losses.regularization_loss = accumulatedSums.loss.regularization_loss / accumulatedSums.count;
    double validation_accuracy = accumulatedSums.accuracy / accumulatedSums.count;

    std::cout << std::fixed << std::setprecision(3)
        << "validation, acc: "  << validation_accuracy 
        << " loss: " << validation_losses.total_loss() 
        << std::endl;

    // Release controller
    busy=false;   // release controller
    return false; // keep clocking
  }

  // next validaiton step
  return launchValidationStep();
}

bool NNBatchController::initEvaluation() {

  std::cout << "### Evaluating images" << std::endl;
  fsmState = MODE::EVALUATION;
  accumulatedSums = {};
  step=0;

  // Calculate number of steps
  prediction_steps = 1;
  unsigned rows = (unsigned) evalImages.data.rows();
  assert(rows>0);
  if (eval_batch_size > 0) {
    prediction_steps = (unsigned int) rows / eval_batch_size;
    // Dividing rounds down. If there are some remaining
    // data but nor full batch, this won't include it
    // Add `1` to include this not full batch
    if (prediction_steps * eval_batch_size < rows)
      prediction_steps += 1;
  } else {
    assert(false); //TODO
  }

  output.verbose(CALL_INFO, 1, 0, "### Evaluation setup\n");
  output.verbose(CALL_INFO, 1, 0, "X.rows()=%" PRId32 "\n", rows);
  output.verbose(CALL_INFO, 1, 0, "eval_batch_size=%" PRId32 "\n", eval_batch_size);
  output.verbose(CALL_INFO, 1, 0, "prediction_steps=%" PRId32 "\n", prediction_steps);
  
  // First validation step
  return launchEvaluationStep();
}

bool NNBatchController::launchEvaluationStep() {

  // If batch size is not set, train using one step and full dataset
  if (eval_batch_size==0) {
      assert(false);
  } else {
      // Otherwise slice a batch
      unsigned p = step*eval_batch_size;
      unsigned r = std::min((unsigned)evalImages.data.rows()-p, eval_batch_size);
      assert(r);
      batch_X = evalImages.data.block(p, 0, r, evalImages.data.cols());
  }

  // std::cout << "batch_X" << util.shapestr(batch_X) << "=\n" << HEAD(batch_X) << std::endl;
  output.verbose(CALL_INFO, 2, 0, "batch_X %s\n", util.shapestr(batch_X).c_str());

  // Initiate the forward pass (completed on monitor_rcv)
  output.verbose(CALL_INFO, 2, 0, "step: %" PRId32 "\n", step);
  forward_o_snd(MODE::EVALUATION);
  busy = true;  // lock controller
  return true;  // disable controller clock
}

void NNBatchController::serialize_order(SST::Core::Serialization::serializer &ser)
{
  NNLayerBase::serialize_order(ser);
  SST_SER(output);
}

bool NNBatchController::stepEvaluation() { 
  assert(step < prediction_steps);

  // Show prediction ( each step is 1 image )
  MNISTinfo info;
  std::cout << "Prediction for " << evalImages.imagePath(step) << " ... ";
  std::cout << "Survey says ### " << info.toString((int) monitor_payload.predictions(0)) << " ###" << std::endl;

  if (++step == prediction_steps) {
    // Finished all evaluation steps.
    evaluationComplete = true;
    output.verbose(CALL_INFO, 2, 0, "Evaluations completed\n");
    // Release controller
    busy=false;   // release controller
    return false; // keep clocking
  }

  // next evaluation step
  return launchEvaluationStep();
}

bool NNBatchController::complete()
{
  fsmState = MODE::COMPLETE;
  output.verbose(CALL_INFO, 2, 0,
                "%s has completed. Ending simulation.\n",
                getName().c_str());
  primaryComponentOKToEndSim();
  return true;
}

bool NNBatchController::clockTick( SST::Cycle_t currentCycle ) {
  // Clocking control should ensure we have something to do.
  assert( !busy || readyToSend);

  if (!busy) {
    assert(readyToSend==false); 
    // not busy so what's next
    switch (fsmState) {
      
      case MODE::TRAINING:
        if (enableTraining())
          return initTraining();
        else if (enableEvaluation())
          return initEvaluation();
        else
          return complete(); 

      case MODE::VALIDATION:
        if (enableValidation())
          return initValidation();
        else if (enableTraining()) {
          validationComplete = false; // validate after each training epoch
          return continueTraining();
        } else if (enableEvaluation())
          return initEvaluation();
        else
          return complete();

      case MODE::EVALUATION:
        if (enableEvaluation())
          return initEvaluation();
        else
          return complete();

      default:
        assert(false);
    }
    assert(false);
    return false;
  }

  // OK we are busy so must be sending something.
  assert(readyToSend==true);
  readyToSend = false;

  switch (fsmState) {
    case MODE::TRAINING:
      return stepTraining();
    case MODE::VALIDATION:
      return stepValidation();
    case MODE::EVALUATION:
      return stepEvaluation();
    // Never should see other states.
    default:
      output.fatal(CALL_INFO, -1, "FSM should not have entered %s state\n", mode2str.at(fsmState).c_str());
      break;
  }

  return false;
}

} // namespace SST::NNBatchController

// EOF
