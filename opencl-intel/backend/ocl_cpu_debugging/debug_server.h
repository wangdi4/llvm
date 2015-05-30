/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#ifndef DEBUG_SERVER_H
#define DEBUG_SERVER_H

#ifndef OclCpuDebugging_EXPORTS
#define OclCpuDebugging_EXPORTS
#endif // OclCpuDebugging_EXPORTS

#include "export/icldebuggingservice.h"

#include "llvm/Support/Mutex.h"

#include <memory>


#pragma warning (disable : 4985 ) /* disable ceil warnings */ 

namespace llvm {
    class MDNode;
}

// Initialize the debug server if debugging is enabled.
// Note: this blocks further execution until a client connects to the server
// and initiates a debugging session.
// Return false iff debugging is enabled and initialization failed.
//
bool InitDebugServer(unsigned int port_number);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// Interface of the debug server.
//
class DebugServer : public ICLDebuggingService
{
public:
    // The debug server is a singleton. Access it via this method
    //
    static DebugServer& GetInstance() {return instance;}

    DebugServer();
    ~DebugServer();

    // Initialize the server. Note that this blocks until a client is 
    // connected.
    //
    bool Init(unsigned int port_number);

    // Explicitly terminate the server connection
    //
    void TerminateConnection();

    // These methods are called by the corresponding debug builtins
    //
    void Stoppoint(const llvm::MDNode* line_metadata);
    void EnterFunction(const llvm::MDNode* subprogram_mdn);
    void ExitFunction(const llvm::MDNode* subprogram_mdn);
    void DeclareLocal(void* addr, const llvm::MDNode* description, const llvm::MDNode* expression);
    void DeclareGlobal(void* addr, const llvm::MDNode* description);

    // Check if the global id passed in as a triple of numbers matches the
    // debugged global id
    //
    bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z);

    // Wait until the client asks to start debugging, initializing the 
    // debugging session according to the client's request.
    //
    void WaitForStartCommand();

private:
    static DebugServer instance;
    struct DebugServerImpl;
    std::auto_ptr<DebugServerImpl> d;

    // No copying
    DebugServer(const DebugServer&);
    DebugServer& operator=(const DebugServer&);

protected:
    mutable llvm::sys::Mutex       m_Lock;
};

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // DEBUG_SERVER_H
