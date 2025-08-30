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

#include "SST.h"

namespace SST::NeuralNet{

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

  explicit NNLayerBase(ComponentId_t id) : SST::Component(id) {}
  NNLayerBase() : SST::Component() {}
  ~NNLayerBase() {}

}; // class NNLayerBase

} //namespace SST::NeuralNet

#endif  // _SST_NN_LAYER_BASE_H_

// EOF
