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
#include "SST.h"

namespace SST::NeuralNet{

// -------------------------------------------------------
// NNSubComponentAPI
// -------------------------------------------------------
class NNSubComponentAPI : public SST::SubComponent 
{
public:
    // Tell SST that this class is a SubComponent API
    SST_ELI_REGISTER_SUBCOMPONENT_API(SST::NeuralNet::NNSubComponentAPI)
    NNSubComponentAPI(ComponentId_t id, Params& params) : SubComponent(id) {}
    virtual ~NNSubComponentAPI() {}

    virtual void forward(const std::vector<uint64_t>& in, std::vector<uint64_t>* o) = 0;
    virtual void backward(const std::vector<uint64_t>& in, std::vector<uint64_t>* o) = 0;
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
    { "monitor",     "monitoring port",          {"neuralnet.NNevent"} }
  )
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    { "transfer_function", 
    "Operations for forward and backward passes",
    "SST::NeuralNet::NNSubComponentAPI"}
  )

  explicit NNLayerBase(ComponentId_t id) : SST::Component(id) {}
  NNLayerBase() : SST::Component() {}
  ~NNLayerBase() {}

  protected:
  // -- Subcomponents
  NNSubComponentAPI* transfer_function = nullptr;

}; // class NNLayerBase

} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_BASE_H_

// EOF
