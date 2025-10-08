#include "nn_event.h"

void SST::NeuralNet::NNEvent::serialize_order(SST::Core::Serialization::serializer &ser)
{
    Event::serialize_order(ser);
}
