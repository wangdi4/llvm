// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef ICLDevBackendPlugin_H
#define ICLDevBackendPlugin_H

#include "ICLDevBackendBinary.h"
#include "ICLDevBackendKernel.h"
#include "ICLDevBackendOptions.h"
#include "ICLDevBackendProgram.h"
#include "cl_device_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * This interface that each plug-in to the Device Backend should implement
 */
class ICLDevBackendPlugin_ {
public:
  /**
   * This method will be called on the creation of program
   *
   * @param pByteCodeContainer byte code container
   * @param pProgram
   * @param pOptions options of the Compilation Service Configuration
   */
  virtual void OnCreateProgram(cl_prog_container_header *pByteCodeContainer,
                               ICLDevBackendProgram_ *pProgram,
                               ICLDevBackendOptions *pOptions) = 0;

  /**
   * This method will be called on program release
   *
   * @param pProgram pointer to program which will be released
   */
  virtual void OnReleaseProgram(ICLDevBackendProgram_ *pProgram) = 0;

  /**
   * This method will be called upon the binary creation
   *
   * @param pProgram pointer to the program
   * @param pKernel pointer to the specific kernel which will be in the binary
   * @param pFunction pointer to the llvm function structure
   * @param pWorkDescription contains the OCL work description
   * @param pContext context which contains the argument values as required for
   * the given kernel
   * @param contextSize context size in bytes
   */
  virtual void OnCreateBinary(ICLDevBackendProgram_ *pProgram,
                              ICLDevBackendKernel_ *pKernel,
                              llvm::Function *pFunction,
                              cl_work_description_type *pWorkDescription,
                              void *pContext, size_t contextSize) = 0;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // ICLDevBackendPlugin_H
