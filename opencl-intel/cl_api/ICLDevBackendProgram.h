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

#ifndef ICLDevBackendProgram_H
#define ICLDevBackendProgram_H

#include "ICLDevBackendKernel.h"
#include "cl_device_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class ICLDevBackendKernel_;

/**
 * This interface represent the bitcode container responsible
 * for holding the bitcode buffer
 */
class ICLDevBackendCodeContainer {
public:
  virtual ~ICLDevBackendCodeContainer() {}

  /**
   * @returns a pointer to the binary bitcode representation buffer of the
   * program
   */
  virtual const void *GetCode() const = 0;

  /**
   * @returns the size of the bitcode buffer
   */
  virtual size_t GetCodeSize() const = 0;
};

/**
 * This interface represent the JIT code (per module) properties
 */
class ICLDevBackendProgramJITCodeProperties {
public:
  /**
   * @returns the size of the JIT code
   */
  virtual size_t GetJITCodeSize() const = 0;

  virtual ~ICLDevBackendProgramJITCodeProperties() {}
};

/**
 * An interface class to OpenCL program object provided by the Back-end Compiler
 * This class is responsible of compiling the program to JIT
 */
class ICLDevBackendProgram_ {
public:
  virtual ~ICLDevBackendProgram_() {}

  /**
   * @returns an unsigned long which represents the program id - this id is
   * unique per program - ; in case of failure 0 will be returned
   */
  virtual unsigned long long int GetProgramID() const = 0;

  /**
   * Gets the program build log
   *
   * @Returns
   *  if the log already exist , pointer to the build log will be returned;
   * otherwise NULL will be returned
   */
  virtual const char *GetBuildLog() const = 0;

  /// Finalize program, e.g. run global ctor, load program dll for windows
  /// native debugger, and retrieve global variable address.
  virtual cl_dev_err_code Finalize() = 0;

  /**
   * Gets the program Code Container; Program code is an abstraction which
   * contain all the kernel's codes (the executable code with it's metadata)
   *
   * @returns code container interface.
   */
  virtual const ICLDevBackendCodeContainer *GetProgramCodeContainer() const = 0;

  /**
   * @returns the program IR bitcode container
   */
  virtual const ICLDevBackendCodeContainer *
  GetProgramIRCodeContainer() const = 0;

  /**
   * Retrieves a pointer to a kernel object by kernel name
   *
   * @param pKernelName pointer to null terminated string that specifiy the
   *kernel name
   * @param ppKernel pointer which will be modified to point to requested kernel
   *object notice it will return the kernel object itself (not a copy)
   *
   * @returns
   *  if the program already build:
   *    CL_DEV_SUCCESS            - if kernel descriptor was
   *successfully retrieved CL_DEV_INVALID_KERNEL_NAME  - if kernel name was not
   *found else CL_DEV_NOT_SUPPORTED will be returned
   */
  virtual cl_dev_err_code
  GetKernelByName(const char *pKernelName,
                  const ICLDevBackendKernel_ **ppKernel) const = 0;

  /**
   * OpenCL 2.0 introduced a feature called Extended Execution. Programs may
   * have so called block kernels which can be enqueued for execution w\o host
   * interaction from inside running kernels. This method returns how many
   * non-block kernels in the program. I.e. the kernels enqueud for execution by
   * a host.
   *
   * @returns
   *  if the program already build:
   *      the number of the non-block kernels in the program will be returned
   */
  virtual int GetNonBlockKernelsCount() const = 0;

  /**
   * Gets how many kernels in the program
   *
   * @returns
   *  if the program already build:
   *      the number of the kernels in the program will be returned
   *  else
   *      0 will be returned
   */
  virtual int GetKernelsCount() const = 0;

  /**
   * Retrieves a pointer to a kernel object by kernel index
   *
   * @param kernelIndex is the index of the kernel should be in the range [0 ..
   *(KernelsCount - 1)]
   * @param pKernel pointer which will hold the returned kernel object
   *      notice it will return the kernel object itself (not a copy)
   *
   * @returns
   *  if the program already build:
   *    CL_DEV_SUCCESS            - if kernel descriptor was
   *successfully retrieved CL_DEV_INVALID_KERNEL_INDEX  - if kernel was not
   *found or incorrect index else CL_DEV_NOT_SUPPORTED will be returned
   */
  virtual cl_dev_err_code
  GetKernel(int kernelIndex, const ICLDevBackendKernel_ **pKernel) const = 0;

  /**
   * Gets the program JIT code Properties;
   *
   * @returns JIT code properties interface, NULL in case of failure
   */
  virtual const ICLDevBackendProgramJITCodeProperties *
  GetProgramJITCodeProperties() const = 0;

  /**
   * Gets The total amount of storage, in bytes, used by
   * program variables in the global address space.
   *
   * @returns
   *  if the program already build:
   *      the total size of global variables in program
   *  otherwise
   *      0 will be returned
   */
  virtual size_t GetGlobalVariableTotalSize() const = 0;

  virtual cl_ulong GetFunctionPointerFor(const char *FunctionName) const = 0;

  /**
   * Retrieves sizes/pointers of all global variables.
   *
   * @param gvPtrs a map from global variable name to its property
   * (size/pointer)
   */
  virtual void GetGlobalVariablePointers(const cl_prog_gv **gvPtrs,
                                         size_t *gvCount) const = 0;
};
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // ICLDevBackendProgram_H
