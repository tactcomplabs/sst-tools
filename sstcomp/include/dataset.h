#ifndef _DATASET_H_
#define _DATASET_H_

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "eigen_utils.h"
#include "nnglobals.h"
#include "OPENCV.h"

namespace fs = std::filesystem;

struct MNISTinfo {
  enum FASHION_MNIST_LABEL : int {
    TOP=0,
    TROUSER=1,
    PULLOVER=2,
    DRESS=3,
    COAT=4,
    SANDAL=5,
    SHIRT=6,
    SNEAKER=7,
    BAG=8,
    ANKLE_BOOT=9,
    UNKNOWN=10
  };
  std::string toString(int label) {
    switch(label) {
      case FASHION_MNIST_LABEL::TOP:        return "TOP";
      case FASHION_MNIST_LABEL::TROUSER:    return "TROUSER";
      case FASHION_MNIST_LABEL::PULLOVER:   return "PULLOVER";
      case FASHION_MNIST_LABEL::DRESS:      return "DRESS";
      case FASHION_MNIST_LABEL::COAT:       return "COAT";
      case FASHION_MNIST_LABEL::SANDAL:     return "SANDAL";
      case FASHION_MNIST_LABEL::SHIRT:      return "SHIRT";
      case FASHION_MNIST_LABEL::SNEAKER:    return "SNEAKER";
      case FASHION_MNIST_LABEL::BAG:        return "BAG";
      case FASHION_MNIST_LABEL::ANKLE_BOOT: return "ANKLE_BOOT";
      case FASHION_MNIST_LABEL::UNKNOWN:    return "UNKNOWN";
      default:
        return "UNKNOWN";
    }
  };
}; //class MNISTinfo

// Adjust image for use with predictor
struct EigenImage {
  enum TRANSFORM { NONE, INVERT, LINEARIZE };
  Eigen::MatrixXd image_matrix = {}; // 2D matrix
  Eigen::MatrixXd linear_image = {}; // single row with entire image.
  std::string filepath;
  EigenImage() {};
  void load(const char* filepath, TRANSFORM invert=NONE, TRANSFORM linearize=NONE, bool print=false) {
    // Load the image
    this->filepath = filepath;
    if (print) 
      std::cout << "Loading image from " << filepath << std::endl;
    cv::Mat original_image = cv::imread(this->filepath, cv::IMREAD_GRAYSCALE);
    if (original_image.empty()) {
      std::cerr << "error: could not read file from path. [" << filepath << "]" << std::endl;
      assert(false);
    }
    
    // std::cout << "Mat type: " << original_image.type() << std::endl;
    // std::cout << "Mat channels: " << original_image.channels() << std::endl;

    // Scale to 28x28 and covert to Eigen::MatrixXd
    cv::Mat image = {};
    if ( original_image.rows != 28  || original_image.cols !=28 ) {
      cv::Size size28(28,28);
      cv::resize(original_image, image, size28, 0, 0, cv::INTER_LINEAR);
    } else {
      image = original_image;
    }
    cv::cv2eigen(image, image_matrix);

    // Conditionally invert
    if (invert == INVERT) {
      image_matrix = 255 - image_matrix.array(); 
    }

    // Normalize the data for range -1 to 1
    image_matrix = (image_matrix.array() - 127.5) / 127.5;

    // Generate the 1D version
    if (linearize == LINEARIZE) {
      long irows = image_matrix.rows();
      long icols = image_matrix.cols();
      long new_cols = irows * icols;
      linear_image.resize(1, new_cols);
      for (long row=0; row<irows; row++) {
        for (long col=0; col<icols; col++) {
          linear_image(0, row*icols + col) = image_matrix(row,col);
        }
      }
    }
  };

}; //struct EigenImage

// Internal representation of image file for training
struct MNIST_image_t {
  Eigen::MatrixXd data = {};
  int label = -1;
  fs::path image_path = {};
};

class Dataset {
public:
  enum DTYPE { INVALID, SIMPLE_CLASSIFICATION, SCALAR, IMAGE };
  std::string imagePath(size_t n) { return mnist_images[n].image_path; }
private:
  DTYPE dtype_ = INVALID;
  int n_samples = 0;
  int n_classes = 0;
  double stddev_ = 0; // scalar only
  Eigen::VectorXd vecScalars = {};
  std::vector<MNIST_image_t> mnist_images = {};
public:
  Eigen::MatrixXd data;
  Eigen::MatrixXi classes;   // for classification mode (e.g. spiral)
  Eigen::MatrixXd scalars;   // for regression/scalar mode (e.g. sine)
  double stddev() { return stddev_; }

  // Training and validation data
  Dataset() {};
  void load(const std::string& pathstring, DTYPE dtype, uint64_t limitPerClass, bool print=false) 
  {
    dtype_ = dtype;
    const char* path = pathstring.c_str();
    if (print)
      std::cout << "Reading " << path << std::endl;
    FILE* f = fopen(path, "r");
    if (!f) {
      std::cerr << "Error opening " << path << std::endl;
      assert(false);
    }

    switch (dtype_) {
      case SIMPLE_CLASSIFICATION:
        load_class_data(f, print);
        break;
      case SCALAR:
        load_scalar_data(path, print);
        break;
      case IMAGE: 
        load_mnist_dataset(path, g_shuffle, limitPerClass, print);
        break;
      case INVALID:
        assert(false);
    }
  }
    
  bool valid() { return n_samples > 0;}
  
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

    std::ifstream inputFile(filepath);
    if (!inputFile.is_open()) {
      std::cerr << "Error: Could not open file: " << filepath << std::endl;
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
      std::cout << "n_samples=" << n_samples << std::endl;
      std::cout << "data=\n" << data << std::endl;
      std::cout << "values=\n" << scalars << std::endl;
    }

  };

  void load_mnist_dataset(const char* dir, bool shuffle, uint64_t classImageLimit, bool print=false) {
    
    assert(this->data.size()==0);
    // Scan all the directories and create a list of labels
    fs::path fullPath = fs::path(dir);
    std::vector<std::string> labels;
    try {
        for (const auto& entry : fs::directory_iterator(fullPath)) {
            labels.push_back(entry.path().filename().string());
            if (labels.size() >= classImageLimit)
              break;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        exit(1);
    }
    // Iterate over each label directory
    std::vector<fs::path> image_paths;
    Eigen::Index image_rows = 0;
    Eigen::Index image_cols = 0;
    // pull in all the image files for this directory using opencv
    for (const auto& label: labels){
      fs::path imgdir = fullPath / label;
      size_t icount = 0;
      try {
        for (const auto& entry : fs::directory_iterator(imgdir)) {
          MNIST_image_t mnist_image = {};
          // check limit
          if (icount++ >= classImageLimit) 
            break;
          // save image path
          std::string path = entry.path().string();
          image_paths.push_back(path);
          // Load the image
          EigenImage eigenImage;
          eigenImage.load(path.c_str());
          // And append it and label to the lists
          mnist_image.data = eigenImage.image_matrix;
          // consistency checking
          if (image_rows==0) {
            image_rows = mnist_image.data.rows();
            image_cols = mnist_image.data.cols();
          } else {
            assert(image_rows == mnist_image.data.rows());
            assert(image_cols == mnist_image.data.cols());
          }
          // update label vector
          mnist_image.label = std::stoi(label);
          mnist_images.emplace_back(mnist_image);
        }
      } catch (const fs::filesystem_error& e) {
          std::cerr << "Error accessing directory: " << e.what() << std::endl;
          exit(1);
      }
      catch (const std::out_of_range& e) {
            std::cerr << "label [" << label << "] is out of range" << std::endl;
            assert(false);
      }
    }

    // conditionally shuffle the vectors
    if (shuffle) {
      // Create a Mersenne Twister engine
      #if 0
      std::mt19937 gen(current_seed++);
      #else
      std::mt19937 gen(42);
      #endif 
      std::shuffle(mnist_images.begin(), mnist_images.end(), gen);

    }

    // convert mnist vector data and labels to a 2D arrays. 
    // For the data, each row will be an entire serialized image.
    unsigned total_rows = (unsigned) mnist_images.size();
    unsigned total_cols = (unsigned)(image_rows * image_cols);
    data.resize(total_rows, total_cols);
    classes.resize(total_rows,1);
    for ( unsigned mnist_index=0; mnist_index<total_rows; mnist_index++) {
      for (unsigned row=0; row<image_rows; row++) {
        classes(mnist_index) = (int) mnist_images[mnist_index].label;
        for (unsigned col=0; col<image_cols; col++) {
          data(mnist_index,row*image_cols + col) = mnist_images[mnist_index].data(row,col);
        }
      }
    }

    if (print)
      std::cout << "Loaded " << total_rows << " images" << std::endl;
  
  }

  // Evaluation data ( no class info )
  void load_eval_images(const std::string& pathstring, EigenImage::TRANSFORM invert, EigenImage::TRANSFORM linearize, bool print=false) 
  {
    dtype_ = DTYPE::IMAGE;
    const char* path = pathstring.c_str();
    if (print)
      std::cout << "Reading " << path << std::endl;
    FILE* f = fopen(path, "r");
    if (!f) {
      std::cerr << "Error opening " << path << std::endl;
      assert(false);
    }
    assert(this->data.size()==0);
    fs::path imgdir = pathstring;
    Eigen::Index image_rows = 0;
    Eigen::Index image_cols = 0;
    try {
      for (const auto& entry : fs::directory_iterator(imgdir)) {
        MNIST_image_t mnist_image = {};
        mnist_image.image_path = entry;
        std::string path = entry.path().string();
        // Load the image
        EigenImage eigenImage;
        eigenImage.load(path.c_str(), invert, linearize, true);
        // And append it and label to the lists
        mnist_image.data = eigenImage.image_matrix;
        // consistency checking
        if (image_rows==0) {
          image_rows = mnist_image.data.rows();
          image_cols = mnist_image.data.cols();
        } else {
          assert(image_rows == mnist_image.data.rows());
          assert(image_cols == mnist_image.data.cols());
        }
        // push image
        mnist_images.emplace_back(mnist_image);
      }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        exit(1);
    }
  
    // convert mnist vector data to a 2D array where each row 
    // is an entire serialized image.
    unsigned total_rows = (unsigned) mnist_images.size();
    unsigned total_cols = (unsigned)(image_rows * image_cols);
    data.resize(total_rows, total_cols);
    for ( unsigned mnist_index=0; mnist_index<total_rows; mnist_index++) {
      for (unsigned row=0; row<image_rows; row++) {
        for (unsigned col=0; col<image_cols; col++) {
          data(mnist_index,row*image_cols + col) = mnist_images[mnist_index].data(row,col);
        }
      }
    }

    if (print)
      std::cout << "Loaded " << total_rows << " images" << std::endl;
    
  }

}; //struct dataset


#endif //_DATASET_H_
