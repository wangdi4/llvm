// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef ICLDevBackendCompilationService_H
#define ICLDevBackendCompilationService_H

#include "ICLDevBackendProgram.h"
#include "cl_device_api.h"
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class ICLDevBackendOptions;

/**
 * This interface class is responsible for the compilation services which should
 * be loaded at the Host side and Device side, programs can be created and
 * compiled through this interface;
 */
class ICLDevBackendCompilationService {
public:
  virtual ~ICLDevBackendCompilationService() {}

  virtual cl_dev_err_code CheckProgramBinary(const void *pBinary,
                                             size_t uiBinarySize) = 0;

  /**
   * Creates program from the specified bytecode, it should verify the byte code
   * before creation
   *
   * @param pByteCodeContainer is the byte code buffer
   * @param ppProgram will be modified to point to the generated program
   *
   * @returns in case of success CL_DEV_SUCCESS will be returned and ppProgram
   * will be modified to point to the generated program; otherwise pProgram will
   * point to NULL and will return: CL_DEV_INVALID_BINARY if the program
   * bytecode is invalid CL_DEV_OUT_OF_MEMORY if there's no sufficient memory
   *  CL_DEV_ERROR_FAIL in any other error
   */
  virtual cl_dev_err_code CreateProgram(const void *pBinary,
                                        size_t uiBinarySize,
                                        ICLDevBackendProgram_ **ppProgram) = 0;

  /**
   * Releases Program instance
   */
  virtual void ReleaseProgram(ICLDevBackendProgram_ *pProgram) const = 0;

  /**
   * Builds the program
   *
   * @param pProgram pointer to the program which will be passed to the builder
   *  (will be modified with the build output)
   * @param pOptions pointer to ICLDevBackendOptions that describes the build
   *options to be used for building the program
   *
   * @returns
   *  CL_DEV_BUILD_ERROR      - in case their is a build errors
   *  CL_DEV_BUILD_WARNING  - in case their is warnings
   *  CL_DEV_SUCCESS          - the build succeeded
   *  CL_DEV_INVALID_BUILD_OPTIONS  - if the build options specified by
   *pOptions are invalid CL_DEV_OUT_OF_MEMORY          - if the there is a
   *failure to allocate memory CL_DEV_BUILD_ALREADY_COMPLETE - if the program
   *has been already compiled
   */
  virtual cl_dev_err_code BuildProgram(ICLDevBackendProgram_ *pProgram,
                                       const ICLDevBackendOptions *pOptions,
                                       const char *pBuildOpts) = 0;

  /// Finalize program, e.g. apply runtime config, run global ctor, load
  /// program dll for windows, etc.
  virtual cl_dev_err_code FinalizeProgram(ICLDevBackendProgram_ *Prog) = 0;

  /**
   * Creates program from backend library kernels.
   *
   * @param Prog will be modified to point to the generated program
   *
   * @returns in case of success CL_DEV_SUCCESS will be returned and Prog
   *  will be modified to point to the generated program; otherwise Prog will
   *  point to NULL and will return:
   *  CL_DEV_OUT_OF_MEMORY if there's no sufficient memory.
   *  CL_DEV_INVALID_VALUE if Prog or KernelNames is nullptr.
   *  CL_DEV_ERROR_FAIL If other internal exceptions are throwed.
   */
  virtual cl_dev_err_code GetLibraryProgram(ICLDevBackendProgram_ **Prog,
                                            const char **KernelNames) = 0;

  /**
   * Dumps the content of the given code container
   * using the options passed in pOptions parameter
   *
   * @param pCodeContainer Code container to dump
   * @param pOptions pointer to the options object which may contain the dump
   *settings. /see cl_dev_backend_dump_options
   *
   * @returns
   *  CL_DEV_SUCCESS          - the build succeeded
   *  CL_DEV_INVALID_BUILD_OPTIONS  - if the build options specified by
   *pOptions are invalid CL_DEV_OUT_OF_MEMORY          - if the there is a
   *failure to allocate memory
   */
  virtual cl_dev_err_code
  DumpCodeContainer(const ICLDevBackendCodeContainer *pCodeContainer,
                    const ICLDevBackendOptions *pOptions) const = 0;

  /**
   * Releases the Compilation Service
   */
  virtual void Release() = 0;

  /**
   * Dumps the JIT code
   *
   * @param program pointer to the program.
   * @param options Pointer to the options object which may contain the dump
   *                settings. /see cl_dev_backend_dump_options
   * @param dumpBinary Dump code as ELF binary or disassembled x86 assembly
   */
  virtual cl_dev_err_code
  DumpJITCodeContainer(ICLDevBackendProgram_ *program,
                       const ICLDevBackendOptions *options,
                       bool dumpBinary = false) const = 0;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // ICLDevBackendCompilationService_H
