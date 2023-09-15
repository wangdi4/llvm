// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef OPENCL_PROGRAM_H
#define OPENCL_PROGRAM_H

#include "IProgram.h"
#include "OpenCLProgramConfiguration.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "llvm/IR/Module.h"
#include <string>

namespace llvm {
class LLVMContext;
}
namespace Validation {

// FIXME 'using namespace' shouldn't be used in header file.
using namespace Intel::OpenCL::DeviceBackend;

/// @brief This class contains OpenCL test program information
class OpenCLProgram : public IProgram {
public:
  /// @brief Constructor
  /// @param [IN] Configuration file of the OpenCL test file
  /// @param [IN] Cpu architecture of the OpenCL program
  OpenCLProgram(OpenCLProgramConfiguration *oclProgramConfig,
                const std::string cpuArch);

  /// @brief destructor
  virtual ~OpenCLProgram(void);

  /// @brief Returns program container
  /// @return Program container of this OpenCL program
  const char *GetProgramContainer() const;

  /// @brief Returns size of program container
  /// @return Size of program container of this OpenCL program
  unsigned int GetProgramContainerSize() const;

  /// @brief Extracts LLVM program from 'program' and parse it
  ///        and create a LLVM module.
  /// @return module
  llvm::Module *ParseToModule(void) const;

private:
  /// @brief create OpenCL program from BC type
  /// @param [IN] programFile Name of OpenCL test program file
  void BCOpenCLProgram(const std::string &programFile);

private:
  /// @brief OpenCL program byte code container
  std::vector<char> m_buffer;

  // LLVM Context used for creation of LLVM state
  const std::unique_ptr<llvm::LLVMContext> C;
};

class ProgramHolder {
public:
  ProgramHolder(const ICLDevBackendCompilationService *pService)
      : m_pService(pService), m_pProgram(NULL) {
    assert(m_pService);
  }

  ProgramHolder(const ICLDevBackendCompilationService *pService,
                ICLDevBackendProgram_ *pProgram)
      : m_pService(pService), m_pProgram(pProgram) {
    assert(m_pService);
  }

  ~ProgramHolder() { m_pService->ReleaseProgram(m_pProgram); }

  ICLDevBackendProgram_ *getProgram() const { return m_pProgram; }

  void setProgram(ICLDevBackendProgram_ *pProgram) {
    // Disabling assertion to allow setting SATest build iterations > 1
    assert(NULL == m_pProgram && "Resetting the program is not supported");
    m_pProgram = pProgram;
  }

private:
  const ICLDevBackendCompilationService *m_pService;
  ICLDevBackendProgram_ *m_pProgram;
};

} // namespace Validation

#endif // OPENCL_PROGRAM_H
