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


CPTSubCompAPI::CPTSubCompAPI(ComponentId_t id, Params& params) : SubComponent(id)
{
    tcldbg::spinner("CPTSUB_SPINNER");
}

CPTSubCompAPI::~CPTSubCompAPI()
{
    if (rng) delete rng;
}

void CPTSubCompAPI::serialize_order(SST::Core::Serialization::serializer &ser)
{
    SST::SubComponent::serialize_order(ser);
    SST_SER(output);
    SST_SER(clocks);
    SST_SER(max);
    SST_SER(seed);
    SST_SER(rng);
}

CPTSubCompVecInt::CPTSubCompVecInt(ComponentId_t id, Params& params) : CPTSubCompAPI(id, params) 
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompVecInt[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "max=%lx seed=%" PRIu32 "\n", max, seed);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    tut.resize(max);
    tutini.resize(max);
    for (size_t i=0; i<max; i++) {
        int32_t n = rng->generateNextInt32();
        tut[i] = n;
        tutini[i] = n;
    }
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}


void SST::CPTSubComp::CPTSubCompVecInt::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check 0x%x\n", clocks, tut[0]);
}

void CPTSubCompVecInt::finish()
{
    output.verbose(CALL_INFO, 2, 0, "finish() clocks %d check 0x%x\n", clocks, tut[0]);
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");
}

int CPTSubCompVecInt::check()
{
    assert(tut.size() == tutini.size());
    assert(tut.size() == max);
    for (size_t i=0;i<max; i++) {
        output.verbose(CALL_INFO, 3, 0, "Comparing %x %x\n", tut[i], tutini[i]);
        if (tut[i] != tutini[i] + (int32_t)clocks) 
            return 1;
    }
    return 0;
}

void CPTSubCompVecInt::update()
{
    clocks++;
    for (size_t i=0; i<max; i++) 
        tut[i]++;
}

void CPTSubCompVecInt::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
    SST_SER(tut);
    SST_SER(tutini);
    SST_SER(subcompEnd);
}

SST::CPTSubComp::CPTSubCompVecPairOfStructs::CPTSubCompVecPairOfStructs(ComponentId_t id, Params &params) : CPTSubCompAPI(id, params)
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompVecPairOfStructs[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "max=%lx seed=%" PRIu32 "\n", max, seed);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    tut.resize(max);
    tutini.resize(max);
    for (size_t i=0; i<max; i++) {
        uint64_t n = rng->generateNextUInt64();
        tut[i].first = struct_t{n};
        tut[i].second = struct_t{n*n};
        tutini[i].first = tut[i].first;
        tutini[i].second = tut[i].second;
    }
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}

void SST::CPTSubComp::CPTSubCompVecPairOfStructs::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check {%s} : {%s}\n", 
        clocks, tut[0].first.toString().c_str(), tut[0].second.toString().c_str());
}

void SST::CPTSubComp::CPTSubCompVecPairOfStructs::finish()
{
    output.verbose(CALL_INFO, 2, 0, 
        "finish() clocks %d check {%s} : {%s}\n", 
        clocks, tut[0].first.toString().c_str(), tut[0].second.toString().c_str());
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");  
}

int SST::CPTSubComp::CPTSubCompVecPairOfStructs::check()
{
    assert(tut.size() == tutini.size());
    assert(tut.size() == max);
    for (size_t i=0;i<tut.size(); i++) {
        output.verbose(CALL_INFO, 3, 0, 
            "Checking tut[%zu].first {%s} against tutini[%zu].first {%s} + %" PRId32 " clocks\n", 
            i, tut[i].first.toString().c_str(), i, tutini[i].first.toString().c_str(), clocks);
        if (tut[i].first != tutini[i].first + clocks)
            return 1;
        output.verbose(CALL_INFO, 3, 0, 
            "Checking tut[%zu].second {%s} against tutini[%zu].second {%s} + %" PRId32 " clocks\n", 
            i, tut[i].second.toString().c_str(), i, tutini[i].second.toString().c_str(), clocks);
        if (tut[i].second != tutini[i].second + clocks)
            return 1;
    }
    return 0;
}

void SST::CPTSubComp::CPTSubCompVecPairOfStructs::update()
{
    clocks++;
    for (size_t i=0; i<max; i++) {
        tut[i].first++;
        tut[i].second++;
    }
}

void SST::CPTSubComp::CPTSubCompVecPairOfStructs::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
    assert(tut.size()==tutini.size());
    SST_SER(tut);
    SST_SER(tutini);
    SST_SER(subcompEnd);
}

SST::CPTSubComp::CPTSubCompVecStruct::CPTSubCompVecStruct(ComponentId_t id, Params &params)  : CPTSubCompAPI(id, params) 
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompVecStruct[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "max=%lx seed=%" PRIu32 "\n", max, seed);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    tut.resize(max);
    tutini.resize(max);
    for (size_t i=0; i<max; i++) {
        uint64_t n = rng->generateNextUInt64();
        tut[i] = n;
        tutini[i] = n;
    }
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}


void SST::CPTSubComp::CPTSubCompVecStruct::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check %s\n", clocks, tut[0].toString().c_str());
}

void SST::CPTSubComp::CPTSubCompVecStruct::finish()
{
    output.verbose(CALL_INFO, 2, 0, "finish() clocks %d check %s\n", clocks, tut[0].toString().c_str());
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");

}

int SST::CPTSubComp::CPTSubCompVecStruct::check()
{
    assert(tut.size() == tutini.size());
    assert(tut.size() == max);
    for (size_t i=0;i<max; i++) {
        output.verbose(CALL_INFO, 3, 0, 
            "Comparing {%s} against {%s} + %" PRId32 "\n", 
            tut[i].toString().c_str(), tutini[i].toString().c_str(), clocks);
        if (tut[i] != tutini[i] + clocks) 
            return 1;
    }
    return 0;
}

void SST::CPTSubComp::CPTSubCompVecStruct::update()
{
    clocks++;
    for (size_t i=0;i<max; i++)
        tut[i]++;
}

void SST::CPTSubComp::CPTSubCompVecStruct::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
    assert(tut.size()==tutini.size());
    SST_SER(tut);
    SST_SER(tutini);
    SST_SER(subcompEnd);
}

SST::CPTSubComp::CPTSubCompPair::CPTSubCompPair(ComponentId_t id, Params &params) : CPTSubCompAPI(id, params)
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompPairOfStructs[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    //max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "seed=%" PRIu32 "\n", seed);
    rng = new SST::RNG::MersenneRNG(seed);
    unsigned n = rng->generateNextUInt32();
    tut.first = n; tut.second = n*n;
    tutini.first = n; tutini.second = n*n;
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}


void SST::CPTSubComp::CPTSubCompPair::setup()
{
    output.verbose(CALL_INFO, 2, 0, 
        "setup() clocks %d check {0x%" PRIx32 " 0x%" PRIx32 "}\n", 
        clocks, tut.first, tut.second);
}

void SST::CPTSubComp::CPTSubCompPair::finish()
{
    output.verbose(CALL_INFO, 2, 0, 
        "finish() clocks %d check {0x%" PRIx32 " 0x%" PRIx32 "}\n", 
        clocks, tut.first, tut.second);
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");  
}

int SST::CPTSubComp::CPTSubCompPair::check()
{
    output.verbose(CALL_INFO, 3, 0, 
        "Checking tut {0x%" PRIx32 " 0x%" PRIx32 " } against tutini {0x%" PRIx32 " 0x%" PRIx32 " } + %" PRId32 " clocks\n", 
        tut.first, tut.second, tutini.first, tutini.second, clocks);
    if (tut.first != tutini.first + clocks) 
        return 1;
    if (tut.second != tutini.second + clocks)
        return 1;
    return 0;
}

void SST::CPTSubComp::CPTSubCompPair::update()
{
    clocks++;
    tut.first++;
    tut.second++;
}

void SST::CPTSubComp::CPTSubCompPair::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
    SST_SER(tut.first);
    SST_SER(tut.second);
    SST_SER(tutini.first);
    SST_SER(tutini.second);
    SST_SER(subcompEnd);
}

SST::CPTSubComp::CPTSubCompPairOfStructs::CPTSubCompPairOfStructs(ComponentId_t id, Params &params) : CPTSubCompAPI(id, params)
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompPairOfStructs[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "seed=%" PRIu32 "\n", seed);
    rng = new SST::RNG::MersenneRNG(seed);
    uint64_t n = rng->generateNextUInt64();
    tut.first = struct_t{n};
    tut.second = struct_t{n*n};
    tutini.first = tut.first;
    tutini.second = tut.second;
    
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;    
}


void SST::CPTSubComp::CPTSubCompPairOfStructs::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check {%s} : {%s}\n", 
        clocks, tut.first.toString().c_str(), tut.second.toString().c_str());

}

void SST::CPTSubComp::CPTSubCompPairOfStructs::finish()
{
    output.verbose(CALL_INFO, 2, 0, 
        "finish() clocks %d check {%s} : {%s}\n", 
        clocks, tut.first.toString().c_str(), tut.second.toString().c_str());
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n"); 
}

int SST::CPTSubComp::CPTSubCompPairOfStructs::check()
{
    output.verbose(CALL_INFO, 3, 0, 
        "Checking tut.first {%s} against tutini.first {%s} + %" PRId32 " clocks\n", 
        tut.first.toString().c_str(), tutini.first.toString().c_str(), clocks);
    if (tut.first != tutini.first + clocks)
        return 1;
    output.verbose(CALL_INFO, 3, 0, 
        "Checking tut.second {%s} against tutini.second {%s} + %" PRId32 " clocks\n", 
        tut.second.toString().c_str(), tutini.second.toString().c_str(), clocks);
    if (tut.second != tutini.second + clocks)
        return 1;
    return 0;
}

void SST::CPTSubComp::CPTSubCompPairOfStructs::update()
{
    clocks++;
    tut.first++;
    tut.second++;
}

void SST::CPTSubComp::CPTSubCompPairOfStructs::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
    SST_SER(tut.first);
    SST_SER(tut.second);
    SST_SER(tutini.first);
    SST_SER(tutini.second);
    SST_SER(subcompEnd);     
}

SST::CPTSubComp::CPTSubCompVecPair::CPTSubCompVecPair(ComponentId_t id, Params &params) : CPTSubCompAPI(id, params)
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompVecPair[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "max=%lx seed=%" PRIu32 "\n", max, seed);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    tut.resize(max);
    tutini.resize(max);
    for (size_t i=0; i<max; i++) {
        unsigned n = rng->generateNextUInt32();
        tut[i].first = n;
        tut[i].second = n*n;
        tutini[i].first = tut[i].first;
        tutini[i].second = tut[i].second;
    }
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}

void SST::CPTSubComp::CPTSubCompVecPair::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check {%" PRIx32 "} : {%" PRIx32 "}\n", 
        clocks, tut[0].first, tut[0].second);

}

void SST::CPTSubComp::CPTSubCompVecPair::finish()
{
    output.verbose(CALL_INFO, 2, 0, 
        "finish() clocks %d check {%x" PRIx32 "} : {%x" PRIx32 "}\n", 
        clocks, tut[0].first, tut[0].second);
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");  

}

int SST::CPTSubComp::CPTSubCompVecPair::check()
{
    assert(tut.size() == tutini.size());
    assert(tut.size() == max);
    for (size_t i=0;i<tut.size(); i++) {
        output.verbose(CALL_INFO, 3, 0, 
            "Checking tut[%zu].first {%" PRIx32 "} against tutini[%zu].first {%" PRIx32 "} + %" PRId32 " clocks\n", 
            i, tut[i].first, i, tutini[i].first, clocks);
        if (tut[i].first != tutini[i].first + clocks)
            return 1;
        output.verbose(CALL_INFO, 3, 0, 
            "Checking tut[%zu].second {%" PRIx32 "} against tutini[%zu].second {%" PRIx32 "} + %" PRId32 " clocks\n", 
            i, tut[i].second, i, tutini[i].second, clocks);
        if (tut[i].second != tutini[i].second + clocks)
            return 1;
    }
    return 0;
}

void SST::CPTSubComp::CPTSubCompVecPair::update()
{
    clocks++;
    for (size_t i=0; i<max; i++) {
        tut[i].first++;
        tut[i].second++;
    }
}

void SST::CPTSubComp::CPTSubCompVecPair::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
        SST_SER(tut);
        SST_SER(tutini);
    SST_SER(subcompEnd);
}

SST::CPTSubComp::CPTSubCompListPairOfStructs::CPTSubCompListPairOfStructs(ComponentId_t id, Params &params) : CPTSubCompAPI(id, params)
{
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompListPairOfStructs[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "max=%lx seed=%" PRIu32 "\n", max, seed);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    for (size_t i=0; i<max; i++) {
        uint64_t n = rng->generateNextUInt64();
        tut.push_back({struct_t{n},struct_t{n*n}});
        tutini.push_back({struct_t{n},struct_t{n*n}});
    }
    assert(tut.size()==max);
    assert(tutini.size()==max);
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;    
}

void SST::CPTSubComp::CPTSubCompListPairOfStructs::setup()
{
    auto it = tut.begin();
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check {%s} : {%s}\n", 
        clocks, it->first.toString().c_str(), it->second.toString().c_str());
}

void SST::CPTSubComp::CPTSubCompListPairOfStructs::finish()
{
    auto it = tut.begin();
    output.verbose(CALL_INFO, 2, 0, 
        "finish() clocks %d check {%s} : {%s}\n", 
        clocks, it->first.toString().c_str(),it->second.toString().c_str());
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");  
}

int SST::CPTSubComp::CPTSubCompListPairOfStructs::check()
{
    assert(tut.size() == tutini.size());
    assert(tut.size() == max);
    // convert lists to vectors for checking
    std::vector<std::pair<struct_t, struct_t>> v_tut(tut.begin(), tut.end());
    std::vector<std::pair<struct_t, struct_t>> v_tutini(tutini.begin(), tutini.end());

    //TODO make checking more generic
    for (size_t i=0;i<tut.size(); i++) {
        output.verbose(CALL_INFO, 3, 0, 
            "Checking tut[%zu].first {%s} against tutini[%zu].first {%s} + %" PRId32 " clocks\n", 
            i, v_tut[i].first.toString().c_str(), i, v_tutini[i].first.toString().c_str(), clocks);
        if (v_tut[i].first != v_tutini[i].first + clocks)
            return 1;
        output.verbose(CALL_INFO, 3, 0, 
            "Checking tut[%zu].second {%s} against tutini[%zu].second {%s} + %" PRId32 " clocks\n", 
            i, v_tut[i].second.toString().c_str(), i, v_tutini[i].second.toString().c_str(), clocks);
        if (v_tut[i].second != v_tutini[i].second + clocks)
            return 1;
    }
    return 0;
}

void SST::CPTSubComp::CPTSubCompListPairOfStructs::update()
{
    clocks++;
    assert(tut.size()==max);
    for (auto it = tut.begin(); it != tut.end(); ++it) {
        it->first++;
        it->second++;
    }
}

void SST::CPTSubComp::CPTSubCompListPairOfStructs::serialize_order(SST::Core::Serialization::serializer &ser)
{
    CPTSubCompAPI::serialize_order(ser);
    SST_SER(subcompBegin);
    assert(tut.size()==tutini.size());
    SST_SER(tut);
    SST_SER(tutini);
    SST_SER(subcompEnd);
}
