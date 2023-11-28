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

#ifndef DEBUG_SERVER_H
#define DEBUG_SERVER_H

#ifndef OclCpuDebugging_EXPORTS
#define OclCpuDebugging_EXPORTS
#endif // OclCpuDebugging_EXPORTS

#include "export/icldebuggingservice.h"
#include "llvm/Support/Mutex.h"
#include <memory>
#include <windows.h>
#pragma warning(disable : 4985) /* disable ceil warnings */

namespace llvm {
class MDNode;
}

// Initialize the debug server if debugging is enabled.
// Note: this blocks further execution until a client connects to the server
// and initiates a debugging session.
// Return false iff debugging is enabled and initialization failed.
//
bool InitDebugServer(unsigned int port_number);

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// Interface of the debug server.
//
class DebugServer : public ICLDebuggingService {
public:
  // The debug server is a singleton. Access it via this method
  //
  static DebugServer &GetInstance() { return instance; }

  DebugServer();
  ~DebugServer();

  // No copying
  DebugServer(const DebugServer &) = delete;
  DebugServer &operator=(const DebugServer &) = delete;

  // Initialize the server. Note that this blocks until a client is
  // connected.
  //
  bool Init(unsigned int port_number);

  // Explicitly terminate the server connection
  //
  void TerminateConnection();

  // These methods are called by the corresponding debug builtins
  //
  void Stoppoint(const llvm::MDNode *line_metadata) override;
  void EnterFunction(const llvm::MDNode *subprogram_mdn) override;
  void ExitFunction(const llvm::MDNode *subprogram_mdn) override;
  void DeclareLocal(void *addr, const llvm::MDNode *description,
                    const llvm::MDNode *expression) override;
  void DeclareGlobal(void *addr, const llvm::MDNode *description) override;

  // Check if the global id passed in as a triple of numbers matches the
  // debugged global id
  //
  bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z) override;

  // Wait until the client asks to start debugging, initializing the
  // debugging session according to the client's request.
  //
  void WaitForStartCommand();

private:
  static DebugServer instance;
  struct DebugServerImpl;
  std::unique_ptr<DebugServerImpl> d;
  HANDLE e = NULL;

protected:
  mutable llvm::sys::Mutex m_Lock;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // DEBUG_SERVER_H
