#ifndef _NNUTIL_H_
#define _NNUTIL_H_

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>

#include "EIGEN.h"

using namespace std;

#ifndef INITIAL_WEIGHT_SCALING
#define INITIAL_WEIGHT_SCALING 0.1
#endif

static const bool g_debug = false;
// static const bool g_debug = true;

static unsigned current_seed = 2;

static const string rand_normal_100k(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/rand_normal_100k.dat"));

static const string spiral_100_2_0(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/spiral_100_2_0.dat"));
static const string spiral_100_2_1(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/spiral_100_2_1.dat"));
static const string spiral_100_3_0(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/spiral_100_3_0.dat"));
static const string spiral_100_3_1(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/spiral_100_3_1.dat"));
static const string spiral_1000_3_0(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/spiral_1000_3_0.dat"));

static const string sine(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/sine.dat"));

#define CLIP(A, MIN, MAX) A.max(MIN).min(MAX)
#define ISVECTOR(A)  ((A.rows()==1)||A.cols()==1)
#define HEADM(A,N,M) A.block(0,0,std::min(N,(int)A.rows()),std::min(M,(int)A.cols()))
#define HEAD(A) HEADM(A,5,4)

Eigen::MatrixXd rand0to1flat(unsigned rows, unsigned cols,  bool readFromFile=false) {
  if (!readFromFile) {
    // - Add 1 to every element and divide by 2
    Eigen::MatrixXd mat = Eigen::MatrixXd().Random(rows, cols);
    Eigen::MatrixXd ones = Eigen::MatrixXd().Constant(rows, cols, 1.0);
    // std::cout << "mat=\n" << mat << std::endl;
    // std::cout << "ones=\n" << ones << std::endl;
    mat = mat + ones;
    // std::cout << "mat=\n" << mat << std::endl;
    mat = mat / 2.0;
    // std::cout << "mat/2=\n" << mat << std::endl;
    return mat;
  } else {
    assert(false);
  }
}

Eigen::MatrixXd rand0to1normal(unsigned rows, unsigned cols, bool readFromFile=false) {

  Eigen::MatrixXd mat = Eigen::MatrixXd(rows, cols);
  if (!readFromFile) {
    // std::random_device rd;
    std::mt19937 gen(current_seed++);
    std::normal_distribution<> dist(0.0, 1.0); // Mean = 0, StdDev = 1
    for (int i=0; i<mat.rows(); i++) {
      for (int j=0; j<mat.cols(); j++) {
        mat(i,j) = dist(gen);
      }
    }
    return mat;
  }

  vector<double> values;
  const char* filepath = rand_normal_100k.c_str();
  ifstream inputFile(filepath);
  if (!inputFile.is_open()) {
    cerr << "Error: Could not open file: " << filepath << endl;
    assert(false);
  }
  double v;
  while (inputFile >> v ) {
    values.push_back(v);
  }
  inputFile.close();
  // cout << "Found " << values.size() << " random numbers from " << filepath << endl;

  size_t index = 0;
  for (int i=0; i<mat.rows(); i++) {
    for (int j=0; j<mat.cols(); j++) {
      mat(i,j) = values[index];
      index = (index + 1 ) % values.size();
    }
  }
  return mat;

}

// p = probability of 1 (also scaling factor)
// q = 1-p = probably of 0
Eigen::MatrixXd rand_binomial_scaled_mask(double p, long rows, long cols,  bool readFromFile=false) {

  if (!readFromFile) {
    // Create a Mersenne Twister engine
    std::mt19937 gen(current_seed++);

    // Create a Bernoulli distribution with the probability of success
    std::bernoulli_distribution distribution(p); 

    // Create the result matrix and initialize to 1/p
    Eigen::MatrixXd mat(rows,cols);
    mat.setConstant(1.0/p);

    // drop elements where result is 0 
    for (unsigned row=0; row < mat.rows(); row++) {
      for (unsigned col=0; col< mat.cols(); col++) {
        if (distribution(gen) == 0)
          mat(row,col) = 0.0;
      }
    }

    return mat;
  } else {
    assert(false);
  }
}

Eigen::RowVectorXi argmax(Eigen::MatrixXd in) {
  Eigen::RowVectorXi rowvec(in.rows()); 
  for (int i = 0; i < in.rows(); ++i) {
      // Find the max value and its column index in the current row
      long index;
      in.row(i).maxCoeff(&index);
      rowvec(i) = (int)index;
  }
  return rowvec;
}

class Dataset {
private:
  bool scalar;
  int n_samples = 0;
  int n_classes = 0;
  double stddev_ = 0; // scalar only
  Eigen::VectorXd vecScalars = {};
public:
  Eigen::MatrixXd data;
  Eigen::MatrixXi classes;   // for classification mode (e.g. spiral)
  Eigen::MatrixXd scalars;   // for regression/scalar mode (e.g. sine)
  double stddev() { return stddev_; }

  Dataset(const string& filestring, bool _scalar=false, bool print=false)  : scalar(_scalar) {
    const char* filepath = filestring.c_str();
    if (print)
      cout << "Reading " << filepath << endl;
    FILE* f = fopen(filepath, "r");
    if (!f) {
      std::cerr << "Error opening " << filepath << std::endl;
      exit(1);
    }

    if (!scalar) {
      load_class_data(f, print);
    } else {
      load_scalar_data(filepath, print);
    }
  }
  bool valid() { return n_samples > 0;}

  private:

  void load_class_data(FILE *f, bool print) {
    int rc = fscanf(f, "%d %d\n", &n_samples, &n_classes); assert(rc);
    int n_total = n_samples * n_classes;
    // std::cout << "spiral_data " << n_samples << " samples, " << n_classes << " classes, " << n_total << " entries" << std::endl;
    data.resize(n_total, 2);
    classes.resize(n_total, 1);

    double x,y;
    rc = fscanf(f, "[[%lf %lf]\n", &x, &y); assert(rc);
    data(0,0) = x; data(0,1) = y;
    for (int n=1;n<n_total-1; n++) {
      rc = fscanf(f, " [%lf %lf]\n", &x, &y); assert(rc);
      data(n,0) = x; data(n,1) = y;
    }
    rc = fscanf(f, " [%lf %lf]]\n", &x, &y); assert(rc);
    data(n_total-1,0) = x; data(n_total-1,1) = y;

    int d;
    rc = fscanf(f, "[%d", &d); assert(rc);
    classes(0,0) = d;
    for (int n=1;n<n_total-1; n++) {
      rc = fscanf(f, "%d", &d); assert(rc);
      classes(n,0) = d;
    }
    rc = fscanf(f, "%d]", &d); assert(rc);
    classes(n_total-1,0) = d;

    fclose(f);

    if (print) {
      for (int n=0; n<n_total; n++) {
        std::cout << n << ": " << classes(n,0) << " ";
        std::cout << "[" <<  data(n,0) << "," << data(n,1) << "]" << std::endl;
      }
    }
  
  }

  void load_scalar_data(const char* filepath, bool print) {

    ifstream inputFile(filepath);
    if (!inputFile.is_open()) {
      cerr << "Error: Could not open file: " << filepath << endl;
      assert(false);
    }
    inputFile >> n_samples;
    assert(n_samples>0);

    data.resize(n_samples,1);
    scalars.resize(n_samples,1);
    vecScalars.resize(n_samples);
    double v;
    for (int i=0;i<n_samples; i++) {
      inputFile >> v;
      data(i,0) = v;
    }
    for (int i=0;i<n_samples;i++) {
      inputFile >> v;
      scalars(i,0) = v;
      vecScalars[i] = v;
    }

    stddev_ = sqrt((vecScalars.array() - vecScalars.mean()).square().sum() / ((double)vecScalars.size() - 1.));

    if (print) {
      cout << "n_samples=" << n_samples << endl;
      cout << "data=\n" << data << endl;
      cout << "values=\n" << scalars << endl;
    }

  };

}; //struct dataset

class Layer_Dense  {

public:
  friend class Optimizer_Adam;
  friend class Loss;

private:
  unsigned n_inputs;
  unsigned n_neurons;
  // states
  Eigen::MatrixXd output_ = {};
  Eigen::MatrixXd inputs_ = {};
  Eigen::MatrixXd weights_ = {};
  Eigen::RowVectorXd biases_ = {};
  // derivatves
  Eigen::MatrixXd dinputs_ = {};
  Eigen::MatrixXd dweights_ = {};
  Eigen::RowVectorXd dbiases_ = {};
  // regularization
  double weight_regularizer_l1_ = 0;
  double weight_regularizer_l2_ = 0;
  double bias_regularizer_l1_ = 0;
  double bias_regularizer_l2_ = 0;
  // optimizer support  
  Eigen::MatrixXd weight_momentums = {};    // like weights
  Eigen::MatrixXd weight_cache = {};        // like weights
  Eigen::RowVectorXd bias_momentums = {};   // like biases
  Eigen::RowVectorXd bias_cache = {};       // like biases
  bool has_weight_cache = false;
  void enable_weight_cache() {
    weight_momentums = Eigen::MatrixXd::Zero(weights_.rows(), weights_.cols());
    weight_cache = Eigen::MatrixXd::Zero(weights_.rows(), weights_.cols());
    bias_momentums = Eigen::RowVectorXd::Zero(biases_.rows(), biases_.cols());
    bias_cache = Eigen::RowVectorXd::Zero(biases_.rows(), biases_.cols());
    has_weight_cache = true;
  }

public:
  // Getters
  Eigen::MatrixXd& output() { return output_; }
  Eigen::MatrixXd& dinputs() { return dinputs_; }
  Eigen::MatrixXd& dweights() { return dweights_; }
  Eigen::RowVectorXd& dbiases() { return dbiases_; }

  // Construction
  Layer_Dense(const unsigned _inputs, const unsigned _neurons,
    double weight_regularizer_l1=0, double weight_regularizer_l2=0,
    double bias_regularizer_l1=0, double bias_regularizer_l2=0) 
    : n_inputs(_inputs), n_neurons(_neurons),
      weight_regularizer_l1_(weight_regularizer_l1),
      weight_regularizer_l2_(weight_regularizer_l2),
      bias_regularizer_l1_(bias_regularizer_l1),
      bias_regularizer_l2_(bias_regularizer_l2)
    {
    // Initialize weights and biases
    //# self.weights = 0.01 * np.random.randn(n_inputs, n_neurons)
    bool normaldist = true;
    bool readFromFile = true;
    if (normaldist) 
      weights_ = rand0to1normal(n_inputs, n_neurons, readFromFile) * INITIAL_WEIGHT_SCALING;
    else
      weights_ = rand0to1flat(n_inputs, n_neurons) * INITIAL_WEIGHT_SCALING;
    //# self.biases = np.zeros((1, n_neurons))
    biases_ = Eigen::RowVectorXd(n_neurons) = Eigen::RowVectorXd::Zero(n_neurons);
  };

  virtual ~Layer_Dense() {}

  // Forward pass
  void forward( const Eigen::MatrixXd& inputs, bool debug=false) {
    // save for backpropagation
    inputs_ = inputs;
    // Calculate output values from inputs, weights and biases
    //# self.output = (inputs @ self.weights) + self.biases
    output_ = inputs * weights_;
    output_ = output_.rowwise() + biases_;

    if (debug | g_debug) {
      cout << "### Layer_Dense.forward ###"  << endl;
      cout << "inputs=\n"  << HEAD(inputs)   << endl;
      cout << "weights=\n" << HEAD(weights_) << endl;
      cout << "biases=\n"  << HEAD(biases_)  << endl;
      cout << "output_=\n" << HEAD(output_)  << endl;
      cout << "################################" << endl;
    }
  }

  // Backward pass
  void backward( Eigen::MatrixXd& dvalues, bool debug=false) {
    if (debug | g_debug) {
      cout << "### Layer_Dense.backward ###" << endl;
      cout << "dvalues=\n" << HEAD(dvalues.array()) << endl;
    }

    // Gradients on parameters
    //# self.dweights = self.inputs.T @ dvalues
    dweights_ = inputs_.transpose() * dvalues;
    // # self.dbiases = np.sum(dvalues, axis=0, keepdims=True)
    dbiases_ = dvalues.colwise().sum().reshaped(1, dvalues.cols());
    
    if (debug | g_debug)
      cout << std::scientific << std::setprecision(7)
        << "dweights(0)=\n" << HEAD(dweights_.array()) 
        << "\ndbiases(0)=\n" << HEAD(dbiases_.array()) << endl;

    // Gradients on regularization
    // L1 on weights
    if (weight_regularizer_l1_ > 0) {
      //# dL1 = np.ones_like(self.weights)
      Eigen::MatrixXd dL1 = Eigen::MatrixXd::Ones(weights_.rows(), weights_.cols());
      //# dL1[self.weights < 0] = -1
      dL1 = (weights_.array() < 0).select(-1, dL1.array());
      //# self.dweights += self.weight_regularizer_l1 * dL1
      dweights_ = dweights_.array() + weight_regularizer_l1_ * dL1.array();
      if (debug | g_debug) {
        cout << "dL1=\n" << HEAD(dL1.array()) << endl;
        cout << "dweights(l1)=\n" << HEAD(dweights_.array()) << endl;
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

    if (debug | g_debug) {
      cout << "weights=\n"  << HEAD(weights_.array())  << endl;
      cout << "biases=\n"   << HEAD(biases_.array())   << endl;
      cout << "dweights=\n" << HEAD(dweights_.array()) << endl;
      cout << "dbiases=\n"  << HEAD(dbiases_.array())  << endl;
      if (weight_regularizer_l1_>0) cout << "weight_regularizer_l1_=" << weight_regularizer_l1_ << endl;
      if (weight_regularizer_l2_>0) cout << "weight_regularizer_l2_=" << weight_regularizer_l2_ << endl;
      if (bias_regularizer_l1_>0) cout << "bias_regularizer_l1=" << bias_regularizer_l1_ << endl;
      if (bias_regularizer_l2_>0) cout << "bias_regularizer_l2=" << bias_regularizer_l2_ << endl;
      cout << "################################" << endl;
    }
    
  }
  
}; //class Layer_Dense

class Layer_Dropout {
  private:
    double rate_; // success rate ( 1 - dropout_rate)
    Eigen::MatrixXd inputs_ = {};
    Eigen::MatrixXd dinputs_ = {};
    Eigen::MatrixXd binary_mask_ = {};
    Eigen::MatrixXd output_ = {};
  public:
    // Getters
    Eigen::MatrixXd& output() { return output_; }
    Eigen::MatrixXd& dinputs() { return dinputs_; }
    // Construction
    Layer_Dropout(double dropout_rate) : rate_(1-dropout_rate) {}

    // Forward Pass
    void forward(Eigen::MatrixXd& inputs) {
      // Save input values
      inputs_ = inputs; // Eigen deep copy default
      // Generate and save scaled mask
      binary_mask_ = rand_binomial_scaled_mask(rate_, inputs_.rows(), inputs_.cols());
      // Apply mask to output values
      output_ = inputs_.array() * binary_mask_.array();
    }

    // Backward pass
    void backward(Eigen::MatrixXd& dvalues) {
      // Gradient on values
      dinputs_ = dvalues.array() * binary_mask_.array();
    }

}; //class Layer_Dropout

class Activation_ReLU {
private:
  Eigen::MatrixXd output_ = {};
  Eigen::MatrixXd inputs_ = {};
  Eigen::MatrixXd dinputs_ = {};
public:
  // Getters
  Eigen::MatrixXd& output() { return output_; }
  Eigen::MatrixXd& dinputs() { return dinputs_; }

  // Forward pass
  void forward(const Eigen::MatrixXd& inputs) {
    // Save for backpropagations
    inputs_ = inputs;
    // Calculation output values from input
    //# self.output = np.maximum(0,inputs)
    output_ = inputs.cwiseMax(0.0);
  }

  // Backward pass
  void backward( const Eigen::MatrixXd& dvalues, bool debug=false) {
    // Since we need to modify the original variable,
    // let's make a copy of the values first
    //#self.dinputs = dvalues.copy()
    dinputs_ = dvalues; // Eigen deep copy default
    // Zero gradient where input values were negative
    //# self.dinputs[self.inputs <= 0] = 0
    Eigen::Array<bool, Eigen::Dynamic, Eigen::Dynamic> mask = (inputs_.array() <= 0).cast<bool>();
    Eigen::MatrixXd zeros(inputs_.rows(), inputs_.cols());
    zeros.setZero();
    dinputs_ = mask.select(zeros.array(), dinputs_);

    if (debug | g_debug) {
      cout << "### ReLU.backward ###" << endl;
      cout << std::scientific << std::setprecision(7)
           << "inputs=\n"  << HEAD(inputs_)
           << "\ndvalues=\n"  << HEAD(dvalues)
           << "\ndinputs=\n" << HEAD(dinputs_)
           << "\n################################" << endl;
    }
  }


};

// Softmax activation
class Activation_Softmax {

private:
  Eigen::MatrixXd inputs_  = {};   // saved inputs
  Eigen::MatrixXd output_  = {};   // forward pass state
  Eigen::MatrixXd dinputs_ = {};   // input gradients

public:
  // Getters
  Eigen::MatrixXd& output() { return output_; }

  // Constructor
  Activation_Softmax() {}

  // Forward pass
  void forward( const Eigen::MatrixXd& inputs, bool debug=false) {

    // Remember input values
    inputs_ = inputs;

    // Get unnormalized probabilities
    //# exp_values = np.exp(inputs - np.max(inputs, axis=1, keepdims=True))
    Eigen::VectorXd row_max = (inputs.rowwise().maxCoeff()).reshaped(inputs.rows(),1); 
    Eigen::MatrixXd exp_values = (inputs.colwise() - row_max).array().exp();

    // Normalize them for each sample
    //# probabilities = exp_values / np.sum(exp_values, axis=1, keepdims=True)
    Eigen::VectorXd row_sum = (exp_values.rowwise().sum()).reshaped(exp_values.rows(), 1);
    Eigen::MatrixXd probabilities = exp_values.array().colwise() / row_sum.array();
    
    // Save result
    output_ = probabilities;

    if (debug | g_debug) {
      cout << "### Softmax.forward ###" << endl;
      std::cout << "inputs=\n" << inputs << std::endl;
      std::cout << "rowmax=\n" << row_max << std::endl;
      std::cout << "exp_values=\n" << exp_values << std::endl;
      std::cout << "row_sum=\n" << row_sum << std::endl;
      std::cout << "output_=\n" << output_ << std::endl;
      cout << "################################" << endl;
    }
  }

  void backward( const Eigen::MatrixXd& dvalues) {

    // Create uninitialized array
    //# self.dinputs = np.empty_like(dvalues)
    dinputs_.resize(dvalues.rows(), dvalues.cols());
    dinputs_.setZero();

    // Enumerate outputs and gradients
    assert(false);

  }

}; //class Activation_Softmax

// Sigmoid activation
class Activation_Sigmoid {
private:
  Eigen::MatrixXd inputs_={};
  Eigen::MatrixXd output_={};
  Eigen::MatrixXd dinputs_={};
public:
  // Getters
  Eigen::MatrixXd& output() { return output_; }
  Eigen::MatrixXd& dinputs() { return dinputs_; }

  // Construction
  Activation_Sigmoid() {};

  // Forward pass
  void forward(Eigen::MatrixXd& inputs, bool debug=false) {
    // Save input and calculate/save output of the sigmoid function
    inputs_ = inputs; // Eigen deep copy
    //# self.output = 1 / (1 + np.exp(-inputs))
    output_ = 1.0 / ( 1.0 + (-1. * inputs.array()).exp());
    if (debug | g_debug) {
      cout << "### Activation_Sigmoid.forward ###" << endl;
      cout << "inputs=\n" << HEAD(inputs) << endl;
      cout << "output=\n" << HEAD(output_) << endl;
      cout << "################################" << endl;
    }
  }

  // Backward pass
  void backward(Eigen::MatrixXd& dvalues, bool debug=false) {
    // Derivative - calculates from output of the sigmoid function
    //# self.dinputs = dvalues * (1 - self.output) * self.output
    dinputs_ = dvalues.array() * (1.0 - output_.array()) * output_.array();
    if (debug | g_debug) {
      cout << "### Activation_Sigmoid.backward ###" << endl;
      cout << "dvalues=\n" << HEAD(dvalues) << endl;
      cout << "dinputs=\n" << HEAD(dinputs_) << endl;
      cout << "################################" << endl;
    }
  }
}; //class Activation_Sigmoid

// Linear activation
class Activation_Linear {
private:
  Eigen::MatrixXd inputs_ = {};
  Eigen::MatrixXd dinputs_ = {};
  Eigen::MatrixXd output_ = {};
public:
  // Getters
  Eigen::MatrixXd& inputs() { return inputs_; }
  Eigen::MatrixXd& dinputs() { return dinputs_; }
  Eigen::MatrixXd& output() { return output_; }

  // Forward pass
  void forward(Eigen::MatrixXd inputs) {
      // Just remember values
      inputs_ = inputs; // deep copies
      output_ = inputs; 
  }

  // Backward pass
  void backward(Eigen::MatrixXd dvalues) {
      // derivative is 1, 1 * dvalues = dvalues - the chain rule
      dinputs_ = dvalues;
  }

}; //class Activation_Linear


// Adam optimizer
class Optimizer_Adam {

private:
  double learning_rate_, current_learning_rate_;
  double decay_, epsilon_;
  double beta_1_, beta_2_;
  int iterations_ = 0;
 
public:
  // Getters
  double current_learning_rate() { return current_learning_rate_; }
  
  // Construction
  Optimizer_Adam(
    double learning_rate=0.001, double decay=0., double epsilon=1e-7,
    double beta_1=0.9, double beta_2=0.999) :
    learning_rate_(learning_rate), current_learning_rate_(learning_rate),
    decay_(decay), epsilon_(epsilon),
    beta_1_(beta_1), beta_2_(beta_2)
  {}

  // Call once before any parameter updates
  void pre_update_params() {
    if (decay_ != 0.0) {
      current_learning_rate_ = learning_rate_ * \
      ( 1. / (1. + decay_ * iterations_ ));
    }
  }

  // Update parameters
  void update_params(Layer_Dense& layer) {

    // If layer does not contain cache arrays,
    // create them filled with zeros
    if (! layer.has_weight_cache)
      layer.enable_weight_cache();

    // Update momentum  with current gradients
    layer.weight_momentums = beta_1_ * layer.weight_momentums.array()
                    + ( 1. - beta_1_) * layer.dweights().array();
    layer.bias_momentums = beta_1_ * layer.bias_momentums.array()
                    + ( 1. - beta_1_) * layer.dbiases().array();

    // Get corrected momentum
    // iteration is 0 at first pass and we need to start with 1 here
    Eigen::MatrixXd weight_momentums_corrected = 
      layer.weight_momentums.array() / ( 1. - pow(beta_1_ ,(iterations_ + 1.)) );
    Eigen::MatrixXd bias_momentums_corrected = 
      layer.bias_momentums.array() / ( 1. - pow(beta_1_ ,(iterations_ + 1.)) );

    // Update cache with squared current gradients
    layer.weight_cache = beta_2_ * layer.weight_cache.array() + 
        (1 - beta_2_) * layer.dweights_.array().pow(2);
    layer.bias_cache = beta_2_ * layer.bias_cache.array() + 
        (1 - beta_2_) * layer.dbiases_.array().pow(2);
  
    // Get correct cache
    Eigen::MatrixXd weight_cache_corrected = layer.weight_cache /
            (1 - pow(beta_2_, (iterations_ + 1)));
    Eigen::MatrixXd bias_cache_corrected = layer.bias_cache /
            (1 - pow(beta_2_, (iterations_ + 1)));


    // Vanilla SGD parameter update + normalization
    // with square rooted cache
    layer.weights_ = layer.weights_.array() - current_learning_rate_ *
                      weight_momentums_corrected.array() /
                      (weight_cache_corrected.array().sqrt() + epsilon_);
    layer.biases_ = layer.biases_.array() - current_learning_rate_ *
                      bias_momentums_corrected.array() /
                      (bias_cache_corrected.array().sqrt()+
                      epsilon_);

  }

  // Call once after any parameter updates
  void post_update_params() {
    iterations_++;
  }

}; //class Optimizer_Adam

// Common loss class
class Loss {

public:
  // Construction
  Loss() {};

  // Forward pass - classification
  virtual Eigen::MatrixXd& forward( Eigen::MatrixXd& output, const Eigen::MatrixXi& y, bool debug=false ) { assert(false); }
  // Forward pass - regression
  virtual Eigen::MatrixXd& forward( Eigen::MatrixXd& output, const Eigen::MatrixXd& y, bool debug=false ) { assert(false); }

  // Regularization loss calculation
  double regularization_loss(Layer_Dense& layer, bool debug=false) {

    if (debug | g_debug) {
      cout << "### Loss.regularization_loss ###" << endl;
      cout << "layer.weights=\n" << HEAD(layer.weights_) << endl;
      cout << "layer.biases=\n" << HEAD(layer.biases_) << endl;
      cout << "layer.weight_regularizer_l1=" << layer.weight_regularizer_l1_ << endl;
      cout << "layer.weight_regularizer_l2=" << layer.weight_regularizer_l2_ << endl;
      cout << "layer.bias_regularizer_l1=" << layer.bias_regularizer_l1_ << endl;
      cout << "layer.bias_regularizer_l2=" << layer.bias_regularizer_l2_ << endl;
    }

    // 0 by default
    double regularization_loss = 0;

    // L1 regularization - weights
    // calculation only when factor greater than 0
    if ( layer.weight_regularizer_l1_ > 0) {
      //# regularization_loss += layer.weight_regularizer_l1 * np.sum(np.abs(layer.weights))
      regularization_loss += layer.weight_regularizer_l1_ * layer.weights_.array().abs().sum();
      if (debug | g_debug)
        cout << "regularization_loss(wl1)=" << regularization_loss << endl;
    }

    // L2 regularization - weights
    if ( layer.weight_regularizer_l2_ > 0) {
      //# regularization_loss += layer.weight_regularizer_l2 * np.sum(layer.weights * layer.weights)
      regularization_loss += layer.weight_regularizer_l2_ * ((layer.weights_.array() * layer.weights_.array())).sum();
      if (debug | g_debug)
        cout << "regularization_loss(wl2)=" << regularization_loss << endl;
    }

    // L1 regularization - biases
    // calculation only when factor greater than 0
    if ( layer.bias_regularizer_l1_ > 0) {
      //# regularization_loss += layer.bias_regularizer_l1 * np.sum(np.abs(layer.biases))
      regularization_loss += layer.bias_regularizer_l1_ * layer.biases_.array().abs().sum();
      if (debug | g_debug)
        cout << "regularization_loss(bl1)=" << regularization_loss << endl;
    }

    // L2 regularization - biases
    if ( layer.bias_regularizer_l2_ > 0) {
      //# regularization_loss += layer.bias_regularizer_l2 * np.sum(layer.biases * layer.biases)
      regularization_loss += layer.bias_regularizer_l2_ * (layer.biases_.array() * layer.biases_.array()).sum();
      if (debug | g_debug)
        cout << "regularization_loss(bl2)=" << regularization_loss << endl;
    }

    if (debug | g_debug) {
      cout << "regularization_loss=" << regularization_loss << endl;
      cout << "################################" << endl;
    }

    return regularization_loss;
  }

  // Calculates the data and regularization losses - classification
  // given model output and ground truth values
  double calculate_i( Eigen::MatrixXd& output, const Eigen::MatrixXi& y, bool debug=false ) {
    
    // Calculate the sample losses
    Eigen::MatrixXd sample_losses = this->forward(output, y, debug);
    
    // Calculate the mean loss
    double data_loss = sample_losses.mean();

    if (debug | g_debug) {
      cout << "### Loss.calculate ###" << endl;
      cout << "output=\n" << HEAD(output) << endl;
      cout << "y=\n" << HEAD(y) << endl;
      cout << "sample_losses=\n" << HEAD(sample_losses) << endl;
      cout << "data_loss=" << data_loss << endl;
      cout << "################################" << endl;
    }
      
    // Return loss
    return data_loss;
  }

  // Calculates the data and regularization losses - regression
  // given model output and ground truth values
  double calculate_d( Eigen::MatrixXd& output, const Eigen::MatrixXd& y, bool debug=false ) {
    
    // Calculate the sample losses
    Eigen::MatrixXd sample_losses = this->forward(output, y, debug);
    
    // Calculate the mean loss
    double data_loss = sample_losses.mean();

    if (debug | g_debug) {
      cout << "### Loss.calculate ###" << endl;
      cout << "output=\n" << HEAD(output) << endl;
      cout << "y=\n" << HEAD(y) << endl;
      cout << "sample_losses=\n" << HEAD(sample_losses) << endl;
      cout << "data_loss=" << data_loss << endl;
      cout << "################################" << endl;
    }
      
    // Return loss
    return data_loss;
  }

}; //class loss

// Cross-entropy loss
class Loss_CategoricalCrossEntropy final : public Loss {

private:
  Eigen::MatrixXd negative_log_likelihoods_ = {};

public:
  // Construction
  Loss_CategoricalCrossEntropy() : Loss() {};
  
  Eigen::MatrixXd& forward( Eigen::MatrixXd& y_pred, const Eigen::MatrixXi& y_true, bool debug ) override {
    
    // Number of samples in a batch
    auto samples = y_pred.rows();
    // cout << "y_pred=\n" << HEAD(y_pred) << endl;

    // Clip data to prevent division by 0
    // Clip both sides to not drag mean towards any value
    //# y_pred_clipped = np.clip(y_pred, 1e-7, 1 - 1e-7)
    Eigen::MatrixXd y_pred_clipped = CLIP(y_pred.array(), 1e-7, 1-1e-7);
    // cout << "y_pred_clipped=\n" << HEAD(y_pred_clipped) << endl;

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
        // cout << "y_true[" << i << "]=" << y_true[i] << endl;
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
    // cout << "correct_confidences=\n" << HEAD(correct_confidences) << endl;

    // Losses
    //# negative_log_likelihoods = -np.log(correct_confidences)
    negative_log_likelihoods_ = -correct_confidences.array().log();

    // Debug
    if (debug | g_debug) {
      cout << "### Loss_CategoricalCrossentropy.forward ###" << endl;
      cout << "Loss_CategoricalCrossentropy" << endl;
      cout << "y_pred=" << HEAD(y_pred) << endl;
      cout << "y_true=" << HEAD(y_true)  << endl;
      cout << "samples=" << samples << endl;
      cout << "ISVECTOR(y_true)=" << ISVECTOR(y_true) << endl;
      cout << "correct_confidences=" << HEAD(correct_confidences) << endl;
      cout << "correct_confidences.size()=" << correct_confidences.size() << endl;
      cout << "negative_log_likelihoods=" << HEAD(negative_log_likelihoods_) << endl;
      // cout << "correct_confidences=" << correct_confidences << endl;
      // cout << "negative_log_likelihoods=" << negative_log_likelihoods_ << endl;
      // cout << "negative_log_likelihoods.size()=" << negative_log_likelihoods_.size() << endl;
      cout << "################################" << endl;
    }

    return negative_log_likelihoods_;
  }

}; //class Loss_CategoricalCrossEntropy

// Softmax classifier - combined Softmax activation
// and cross-entropy loss for faster backward step
class Activation_Softmax_Loss_CategoricalCrossentropy {

private:
  std::unique_ptr<Activation_Softmax> activation_ = nullptr;
  std::shared_ptr<Loss_CategoricalCrossEntropy> loss_ = nullptr;
  Eigen::MatrixXd output_ = {};
  Eigen::MatrixXd dinputs_ = {};

public:
  // Getters
  Eigen::MatrixXd& output() { return output_; }
  Eigen::MatrixXd& dinputs() { return dinputs_; }
  std::shared_ptr<Loss_CategoricalCrossEntropy> loss() { assert(loss_); return loss_; }

  // Construction
  Activation_Softmax_Loss_CategoricalCrossentropy() {
    activation_ = make_unique<Activation_Softmax>();
    loss_ = make_shared<Loss_CategoricalCrossEntropy>();
  }
  virtual ~Activation_Softmax_Loss_CategoricalCrossentropy() {};

  // Forward pass
  // returns loss calculation - classification
  double forward( const Eigen::MatrixXd& inputs, const Eigen::VectorXi& y_true) {
    // Output layer's activation function
    activation_->forward(inputs);
    // Set the output
    output_ = activation_->output();
    // Calculation and return loss value
    return loss_->calculate_i(output_, y_true);
  }

   // Forward pass
  // returns loss calculation - regression
  double forward( const Eigen::MatrixXd& inputs, const Eigen::VectorXd& y_true) {
    // Output layer's activation function
    activation_->forward(inputs);
    // Set the output
    output_ = activation_->output();
    // Calculation and return loss value
    return loss_->calculate_d(output_, y_true);
  }

  // Backward pass
  void backward(const Eigen::MatrixXd& dvalues, Eigen::VectorXi& y_true) {

    // Number of samples
    auto samples = dvalues.rows();

    // If labels are one-hot encoded,
    // turn them into discrete values
    if (!ISVECTOR(y_true)) {
      //# y_true = np.argmax(y_true, axis=1)
      assert(false);
    }

    // Copy so we can safely modify
    dinputs_ = dvalues;  // Eigen default deep copy
    // Calculate gradient
    //# self.dinputs[range(samples), y_true] -= 1
    for (int i=0; i<samples; i++) {
      auto v = dinputs_(i, y_true[i]);
      dinputs_(i, y_true[i]) = v - 1;
    }

    // Normalize gradient
    //# self.dinputs = self.dinputs / samples
    dinputs_ = dinputs_.array() / samples;

  }


}; //class Activation_Softmax_Loss_CategoricalCrossentropy

// Binary cross-entropy loss
class Loss_BinaryCrossentropy : public Loss {
private:
  Eigen::MatrixXd sample_losses_ = {};
  Eigen::MatrixXd dinputs_ = {};
public:
  // Getters
  Eigen::MatrixXd& dinputs() { return dinputs_; }

  // Construction
  Loss_BinaryCrossentropy() : Loss() {};

  // Forward pass
  Eigen::MatrixXd& forward( Eigen::MatrixXd& y_pred, const Eigen::MatrixXi& y_true, bool debug ) override {
    // Clip data to prevent division by 0
    // Clip both sides to not drag mean towards any value
    //# y_pred_clipped = np.clip(y_pred, 1e-7, 1 - 1e-7)
    Eigen::MatrixXd y_pred_clipped = CLIP(y_pred.array(), 1e-7, 1 - 1e-7);

    // Calculate sample-wise loss
    //# sample_losses = -(y_true * np.log(y_pred_clipped) +
    //#                   (1 - y_true) * np.log(1 - y_pred_clipped))
    // Eigen::MatrixXd term1 = -1.0 * (y_true.array().cast<double>() * y_pred_clipped.array().log() +
    //                     ( 1.0 - y_true.array().cast<double>()) * (1.0 - y_pred_clipped.array().log()));
    Eigen::MatrixXd term1 = y_true.array().cast<double>() * y_pred_clipped.array().log();
    Eigen::MatrixXd term2 = ( 1.0 - y_true.array().cast<double>()) * (1.0 - y_pred_clipped.array()).log();
    Eigen::MatrixXd term3 = -1.0 * (term1.array() + term2.array());

    //# sample_losses = np.mean(sample_losses, axis=-1)
    sample_losses_ = term3.rowwise().mean();

    if (debug | g_debug) {
      cout << "### Loss_BinaryCrossentropy.forward ###" << endl;
      cout << "y_pred=\n" << HEAD(y_pred) << endl;
      cout << "y_pred_clipped=\n" << HEAD(y_pred_clipped) << endl;
      cout << "y_true=\n" << HEAD(y_true) << endl;
      cout << "term1=\n" << HEAD(term1) << endl;
      cout << "term2=\n" << HEAD(term2) << endl;
      cout << "term3=\n" << HEAD(term3) << endl;
      cout << "sample_losses=\n" << HEAD(sample_losses_) << endl;
      cout << "################################" << endl;
    }
    
    // Return losses
    return sample_losses_;
  }

  // Backward pass
  void backward(const Eigen::MatrixXd& dvalues, Eigen::MatrixXi& y_true, bool debug=false) {

    // Number of samples
    auto samples = dvalues.rows();
    // Number of outputs in every sample
    // We'll use the first sample to count them
    //# outputs = len(dvalues[0])
    auto outputs = 1; // TODO Temporary since we only have 1 output for this example

    // Clip data to prevent division by 0
    // Clip both side to not drage mean towards any value
    //# clipped_dvalues = np.clip(dvalues, 1e-7, 1 - 1e-7)
    Eigen::MatrixXd clipped_dvalues = CLIP(dvalues.array(), 1e-7, 1 - 1e-7);

    // Calculate gradient
    //# self.dinputs = -(y_true / clipped_dvalues -
    //#                  (1 - y_true) / (1 - clipped_dvalues)) / outputs
    dinputs_ = -1 * ( y_true.array().cast<double>() / clipped_dvalues.array() -
                  ( 1.0 - y_true.array().cast<double>() ) / ( 1.0 - clipped_dvalues.array())) / outputs;
    // Normalize gradient
    dinputs_ = dinputs_.array() / samples;

    if (debug | g_debug) {
      cout << "### Loss_BinaryCrossentropy.backward ###" << endl;
      cout << "dvalues=\n" << HEAD(dvalues) << endl;
      cout << "y_true=\n" << HEAD(y_true) << endl;
      cout << "dinputs=\n" << HEAD(dinputs_) << endl;
      cout << "################################" << endl;
    }

  }

}; //class Loss_BinaryCrossentropy 

// Mean Squared Error loss
class Loss_MeanSquaredError : public Loss {
private:
  Eigen::MatrixXd sample_losses_ = {};
  Eigen::MatrixXd dinputs_ = {};
public:
  // Getters
  Eigen::MatrixXd& dinputs() { return dinputs_; }

  // Construction
  Loss_MeanSquaredError() : Loss() {};

  // Forward pass
  Eigen::MatrixXd& forward( Eigen::MatrixXd& y_pred, const Eigen::MatrixXd& y_true, bool debug ) override {
    // Calculate loss
    //# sample_losses = np.mean((y_true - y_pred)**2, axis=-1)
    Eigen::MatrixXd d = y_true - y_pred;
    sample_losses_ = (d.array() * d.array()).rowwise().mean();

    if (debug | g_debug) {
      cout << "### Loss_MeanSquaredError.forward ###" << endl;
      cout << "y_pred=\n" << HEAD(y_pred) << endl;
      cout << "y_true=\n" << HEAD(y_true) << endl;
      cout << "sample_losses=\n" << HEAD(sample_losses_) << endl;
      cout << "################################" << endl;
    }
    
    // Return losses
    return sample_losses_;
  }

  // Backward pass - classification
  void backward(const Eigen::MatrixXd& dvalues, Eigen::MatrixXi& y_true, bool debug=false) {

    // Number of samples
    auto samples = dvalues.rows();
    // Number of outputs in every sample
    // We'll use the first sample to count them
    //# outputs = len(dvalues[0])
    auto outputs = 1; // TODO Temporary since we only have 1 output for this example

    // Clip data to prevent division by 0
    // Clip both side to not drage mean towards any value
    //# clipped_dvalues = np.clip(dvalues, 1e-7, 1 - 1e-7)
    Eigen::MatrixXd clipped_dvalues = CLIP(dvalues.array(), 1e-7, 1 - 1e-7);

    // Calculate gradient
    //# self.dinputs = -(y_true / clipped_dvalues -
    //#                  (1 - y_true) / (1 - clipped_dvalues)) / outputs
    dinputs_ = -1 * ( y_true.array().cast<double>() / clipped_dvalues.array() -
                  ( 1.0 - y_true.array().cast<double>() ) / ( 1.0 - clipped_dvalues.array())) / outputs;
    // Normalize gradient
    dinputs_ = dinputs_.array() / samples;

    if (debug | g_debug) {
      cout << "### Loss_MeanSquaredError.backward(I) ###" << endl;
      cout << "dvalues=\n" << HEAD(dvalues) << endl;
      cout << "y_true=\n" << HEAD(y_true) << endl;
      cout << "dinputs=\n" << HEAD(dinputs_) << endl;
      cout << "################################" << endl;
    }

  }

  // Backward pass - regression
  void backward(const Eigen::MatrixXd& dvalues, Eigen::MatrixXd& y_true, bool debug=false) {

    // Number of samples
    auto samples = dvalues.rows();
    // Number of outputs in every sample
    // We'll use the first sample to count them
    //# outputs = len(dvalues[0])
    auto outputs = 1; // TODO Temporary since we only have 1 output for this example

    // Gradient on values
    dinputs_ = -2.0 * ( y_true - dvalues ) / outputs;

    // Normalize gradient
    dinputs_ = dinputs_ / samples;

    if (debug | g_debug) {
      cout << "### Loss_MeanSquaredError.backward(D) ###" << endl;
      cout << "samples=" << samples << endl;
      cout << "outputs=" << outputs << endl;
      cout << std::scientific << std::setprecision(7);
      cout << "dvalues=\n" << HEAD(dvalues) << endl;
      cout << "y_true=\n" << HEAD(y_true) << endl;
      cout << "dinputs=\n" << HEAD(dinputs_) << endl;
      cout << "################################" << endl;
    }

  }

}; //class Loss_MeanSquaredError 

#endif //#define _NNUTIL_H_
