// Copyright 2009-2025 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2025, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef INTERACTIVE_CONSOLE_DEBUG_SST15_H
#define INTERACTIVE_CONSOLE_DEBUG_SST15_H

#include "SST.h"
//#include "sst/core/eli/elementinfo.h"
//#include <sst/core/watchPoint.h>
//#include <sst/core/interactiveConsole.h>
//#include "sst/core/serialization/objectMapDeferred.h"
#include "probe.h"
using namespace SSTDEBUG::Probe;


namespace SST::ICDbgSST15 {

class ICDebugSST15 : public SST::InteractiveConsole
{

public:
    SST_ELI_REGISTER_INTERACTIVE_CONSOLE(
        ICDebugSST15,   // class 
        "dbgsst15",     // library
        "ICDebugSST15", // name
        SST_ELI_ELEMENT_VERSION(1, 0, 0),
        "{EXPERIMENTAL} Interactive console debug probe")

    ICDebugSST15(Params& params);
    ~ICDebugSST15() {}

    void execute(const std::string& msg) override;

private:
    // This is the stack of where we are in the class hierarchy.  This
    // is needed because when we advance time, we'll need to delete
    // any ObjectMap because they could change during execution.
    // After running, this will allow us to recreate the working
    // directory as far as we can.
    std::vector<std::string> name_stack;

    SST::Core::Serialization::ObjectMap* obj_ = nullptr;
    bool                                 done = false;

    // Keep a pointer to the ObjectMap for the top level Component
    SST::Core::Serialization::ObjectMapDeferred<BaseComponent>* base_comp_ = nullptr;
    
    // Keep track of all the WatchPoints
    std::vector<std::pair<WatchPoint*, BaseComponent*>> watch_points_;

    std::vector<std::string> tokenize(std::vector<std::string>& tokens, const std::string& input);

    void cmd_help(std::vector<std::string>& UNUSED(tokens));
    void cmd_pwd(std::vector<std::string>& UNUSED(tokens));
    void cmd_ls(std::vector<std::string>& UNUSED(tokens));
    void cmd_cd(std::vector<std::string>& tokens);
    void cmd_print(std::vector<std::string>& tokens);
    void cmd_set(std::vector<std::string>& tokens);
    void cmd_time(std::vector<std::string>& tokens);
    void cmd_run(std::vector<std::string>& tokens);
    void cmd_watch(std::vector<std::string>& tokens);
    void cmd_unwatch(std::vector<std::string>& tokens);
    void cmd_shutdown(std::vector<std::string>& tokens);

    void dispatch_cmd(std::string cmd);

    // Trace functionality
    void cmd_watchlist(std::vector<std::string>& tokens);
    void cmd_trace(std::vector<std::string>& tokens);
    void cmd_addTraceVar(std::vector<std::string>& tokens);
    void cmd_resetTraceBuffer(std::vector<std::string>& tokens);
    void cmd_printTrace(std::vector<std::string>& tokens);
    void cmd_printWatchpoint(std::vector<std::string>& tokens);
};

} // namespace SSTDEBUG::DbgSST15

#endif
