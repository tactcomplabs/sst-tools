//
// _nn_eigen_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_NN_EIGEN_H
#define _SST_NN_EIGEN_H

// clang-format off
#include "EIGEN.h"
#include "SST.h"
// clang-format on

namespace SST::NeuralNet {

// -------------------------------------------------------
// NNEigen - wrapper class for serializable Eigen Objects
// -------------------------------------------------------

class NNMatrixXd : public SST::Core::Serialization::serializable {

public:
    // default constructor required for serialization
    NNMatrixXd() {};
    // Serialization
    void serialize_order(SST::Core::Serialization::serializer& ser) override {
        rows_ = mat_.rows();
        cols_ = mat_.cols();
        switch ( ser.mode() ) {
            case SST::Core::Serialization::serializer::SIZER:
            case SST::Core::Serialization::serializer::PACK:
            {
                // Get the data size during SIZER
                // and  pack the data into the serialization stream during PACK
                SST_SER(rows_);
                SST_SER(cols_);
                for (Eigen::Index row=0; row<rows_; row++) {
                    for (Eigen::Index col=0; col<cols_; col++)
                        SST_SER(mat_(row,col));
                }
                break;
            }
        case SST::Core::Serialization::serializer::UNPACK:
        {   
            // Extract the serialized data from the stream
            SST_SER(rows_);
            SST_SER(cols_);
            for (Eigen::Index row=0; row<rows_; row++) {
                for (Eigen::Index col=0; col<cols_; col++)
                    SST_SER(mat_(row,col));
            }
            break;
        }
        case SST::Core::Serialization::serializer::MAP:
        {
            // TODO
            break;
        }
        }

    }
    ImplementSerializable(SST::NeuralNet::NNMatrixXd)

    private:
     Eigen::MatrixXd mat_ = {};
     Eigen::Index rows_ = 0;
     Eigen::Index cols_ = 0;

};// class NNMatrixXd

}

#endif  // _SST_NN_EIGEN_H