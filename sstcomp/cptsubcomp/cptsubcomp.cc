//
// _cptsubcomp_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "cptsubcomp.h"
#include "tcldbg.h"

using namespace SST;
using namespace SST::CPTSubComp;

// cptSubCompVecInt
CPTSubCompVecInt::CPTSubCompVecInt(ComponentId_t id, Params& params) : CPTSubCompAPI() {
    max = params.find<size_t>("max", 1024);
    seed = params.find<unsigned>("seed", 1223);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    for (size_t i=0; i<max; i++)
        tut.push_back(rng->generateNextInt32());
}

CPTSubCompVecInt::~CPTSubCompVecInt()
{
    if (rng) delete rng;
}

int CPTSubCompVecInt::check()
{
    return 0;
}

void CPTSubCompVecInt::update()
{
    
}

void CPTSubCompVecInt::serialize_order(SST::Core::Serialization::serializer &ser)
{
    SST_SER(max);
    SST_SER(seed);
    SST_SER(tut);
    SST_SER(rng);
}

// EOF
