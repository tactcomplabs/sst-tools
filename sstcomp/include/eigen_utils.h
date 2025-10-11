#ifndef _EIGEN_UTILS_
#define _EIGEN_UTILS_

#include <fstream>
#include <random>
#include <vector>

#include "EIGEN.h"

#define CLIP(A, MIN, MAX) A.max(MIN).min(MAX)
#define ISVECTOR(A)  ((A.rows()==1)||A.cols()==1)

#define HEADM(A,N,M) A.block(0,0,std::min(N,(int)A.rows()),std::min(M,(int)A.cols()))
#define HEAD(A) HEADM(A,5,5)
#define HEAD0(A) HEADM(A,5,(int)A.cols())
#define HEAD1(A) HEADM(A,(int)A.rows(),5)

// #define TAILM(A,N,M) A.block(A.rows()-std::min(N,(int)A.rows())+1,A.cols()-std::min(M,(int)A.cols())+1, A.rows()-1, A.cols()-1)
#define TAILM(A,N,M) A.block(A.rows()-std::min(N, (int)A.rows()), A.cols()-std::min(M, (int)A.cols()), std::min(N, (int)A.rows()), std::min(M, (int)A.cols()))
#define TAIL(A) TAILM(A,5,5)

static unsigned current_seed = 2;
static const std::string rand_normal_100k(std::string(getenv("HOME")).append("/work/ws/cppnnfs/dat/rand_normal_100k.dat"));

class Eutils {
public:

  std::string shapestr(const Eigen::MatrixXd& A) {
    std::stringstream s;
    s << "(" << std::dec << A.rows() << ", " << A.cols() << ")";
    std::string ret = s.str();
    return ret;
  }

  std::string shapestr(const Eigen::MatrixXi& A) {
    std::stringstream s;
    s << "(" << std::dec << A.rows() << ", " << A.cols() << ")";
    std::string ret = s.str();
    return ret;
  }

  std::string shapestr(const Eigen::RowVectorXd& A) {
    std::stringstream s;
    s << "(1, " << std::dec << A.cols() << ")";
    std::string ret = s.str();
    return ret;
  }

  void rand0to1flat(Eigen::MatrixXd& result, unsigned rows, unsigned cols,  bool readFromFile=false) {
    if (!readFromFile) {
      // - Add 1 to every element and divide by 2
      result =  Eigen::MatrixXd().Random(rows, cols);
      Eigen::MatrixXd ones = Eigen::MatrixXd().Constant(rows, cols, 1.0);
      // std::cout << "mat=\n" << mat << std::endl;
      // std::cout << "ones=\n" << ones << std::endl;
      result = result + ones;
      // std::cout << "result=\n" << result << std::endl;
      result = result / 2.0;
      // std::cout << "result/2=\n" << result << std::endl;
    } else {
      assert(false);
    }
  }

  void rand0to1normal(Eigen::MatrixXd& result, unsigned rows, unsigned cols, bool readFromFile=false) {
    result = Eigen::MatrixXd(rows, cols);
    if (!readFromFile) {
      // std::random_device rd;
      // std::mt19937 gen(current_seed++);
      std::mt19937 gen(42);
      std::normal_distribution<> dist(0.0, 1.0); // Mean = 0, StdDev = 1
      for (int i=0; i<result.rows(); i++) {
        for (int j=0; j<result.cols(); j++) {
          result(i,j) = dist(gen);
        }
      }
      return;
    }

    std::vector<double> values;
    const char* filepath = rand_normal_100k.c_str();
    std::ifstream inputFile(filepath);
    if (!inputFile.is_open()) {
      std::cerr << "Error: Could not open file: " << filepath << std::endl;
      assert(false);
    }
    double v;
    while (inputFile >> v ) {
      values.push_back(v);
    }
    inputFile.close();
    // cout << "Found " << values.size() << " random numbers from " << filepath << std::endl;

    size_t index = 0;
    for (int i=0; i<result.rows(); i++) {
      for (int j=0; j<result.cols(); j++) {
        result(i,j) = values[index];
        index = (index + 1 ) % values.size();
      }
    }
    return;
}

  // p = probability of 1 (also scaling factor)
  // q = 1-p = probably of 0
  void rand_binomial_scaled_mask(Eigen::MatrixXd& result, double p, long rows, long cols,  bool readFromFile=false) {

    if (!readFromFile) {
      // Create a Mersenne Twister engine
      std::mt19937 gen(current_seed++);

      // Create a Bernoulli distribution with the probability of success
      std::bernoulli_distribution distribution(p); 

      // Create the result matrix and initialize to 1/p
      result.resize(rows,cols);
      result.setConstant(1.0/p);

      // drop elements where result is 0 
      for (unsigned row=0; row < result.rows(); row++) {
        for (unsigned col=0; col< result.cols(); col++) {
          if (distribution(gen) == 0)
            result(row,col) = 0.0;
        }
      }
    } else {
      assert(false);
    }
  }

  void argmax(Eigen::MatrixXd& result, const Eigen::MatrixXd& in) {
    result.resize(in.rows(),1);
    for (int i = 0; i < in.rows(); ++i) {
        // Find the max value and its column index in the current row
        long index;
        in.row(i).maxCoeff(&index);
        result(i,0) = (int)index;
    }
  }

}; // class Eutils

#endif //_EIGEN_UTILS_