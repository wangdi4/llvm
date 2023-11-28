// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef ICLDEBUGGINGSERVICE_H
#define ICLDEBUGGINGSERVICE_H

namespace llvm {
class MDNode;
}

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

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
  virtual void Stoppoint(const llvm::MDNode *line_metadata) = 0;
  virtual void EnterFunction(const llvm::MDNode *subprogram_mdn) = 0;
  virtual void ExitFunction(const llvm::MDNode *subprogram_mdn) = 0;
  virtual void DeclareLocal(void *addr, const llvm::MDNode *description,
                            const llvm::MDNode *expression) = 0;
  virtual void DeclareGlobal(void *addr, const llvm::MDNode *description) = 0;
  virtual bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z) = 0;
  virtual ~ICLDebuggingService() {}
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#ifdef __cplusplus
extern "C" {
#endif
// Initialize the debugging service. Return 'true' if initialization
// succeeded, false otherwise. If the debugging service is disabled, true
// is returned.
//
DEBUG_SERVICE_API bool InitDebuggingService(unsigned int port_number);

// Get a pointer to the debugging service instance
//
DEBUG_SERVICE_API Intel::OpenCL::DeviceBackend::ICLDebuggingService *
DebuggingServiceInstance();

// Terminates the debugging service
//
DEBUG_SERVICE_API void TerminateDebuggingService();

// Function pointer types
//
typedef bool (*DEBUGGING_SERVICE_INIT_FUNC)(unsigned int);
typedef Intel::OpenCL::DeviceBackend::ICLDebuggingService *(
    *DEBUGGING_SERVICE_INSTANCE_FUNC)();
typedef void (*DEBUGGING_SERVICE_TERMINATE_FUNC)();

#ifdef __cplusplus
}
#endif

#endif // ICLDEBUGGINGSERVICE_H
