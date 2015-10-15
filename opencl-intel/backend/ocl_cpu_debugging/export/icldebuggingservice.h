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

#ifndef ICLDEBUGGINGSERVICE_H
#define ICLDEBUGGINGSERVICE_H

namespace llvm {
    class MDNode;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

#ifndef DEBUG_SERVICE_API
    #if defined(_WIN32)
        #ifdef OclCpuDebugging_EXPORTS
            #define DEBUG_SERVICE_API __declspec(dllexport)
        #else
            #define DEBUG_SERVICE_API __declspec(dllimport)
        #endif
    #else
        #define DEBUG_SERVICE_API 
    #endif
#endif

class ICLDebuggingService {
public:
    virtual void Stoppoint(const llvm::MDNode* line_metadata) = 0;
    virtual void EnterFunction(const llvm::MDNode* subprogram_mdn) = 0;
    virtual void ExitFunction(const llvm::MDNode* subprogram_mdn) = 0;
    virtual void DeclareLocal(void* addr, const llvm::MDNode* description, const llvm::MDNode* expression) = 0;
    virtual void DeclareGlobal(void* addr, const llvm::MDNode* description) = 0;
    virtual bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z) = 0;
};

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {


#ifdef __cplusplus
extern "C"
{
#endif
    // Initialize the debugging service. Return 'true' if initialization 
    // succeeded, false otherwise. If the debugging service is disabled, true
    // is returned.
    //
    DEBUG_SERVICE_API bool InitDebuggingService(unsigned int port_number);

    // Get a pointer to the debugging service instance
    //
    DEBUG_SERVICE_API Intel::OpenCL::DeviceBackend::ICLDebuggingService* DebuggingServiceInstance();

    // Terminates the debugging service
    //
    DEBUG_SERVICE_API void TerminateDebuggingService();

    // Function pointer types
    //
    typedef bool (*DEBUGGING_SERVICE_INIT_FUNC)(unsigned int);
    typedef Intel::OpenCL::DeviceBackend::ICLDebuggingService* (*DEBUGGING_SERVICE_INSTANCE_FUNC)();
    typedef void (*DEBUGGING_SERVICE_TERMINATE_FUNC)();

#ifdef __cplusplus
}
#endif


#endif // ICLDEBUGGINGSERVICE_H
