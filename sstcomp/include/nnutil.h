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

static unsigned current_seed = 2;
static const string rand_normal_100k(string(getenv("HOME")).append("/work/ws/cppnnfs/dat/rand_normal_100k.dat"));

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

#endif //#define _NNUTIL_H_
