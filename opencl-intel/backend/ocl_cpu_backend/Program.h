// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERIALIZE METHODS
#pragma once

#include "ICLDevBackendProgram.h"
#include "RuntimeService.h"
#include "Serializer.h"
#include "cl_dev_backend_api.h"
#include "cl_types.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <memory>
#include <string>

namespace llvm {
class ExecutionEngine;
class Module;
} // namespace llvm

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class KernelSet;
class BitCodeContainer;
class ObjectCodeContainer;

enum CodeProfilingStatus : unsigned int {
  PROFILING_NONE = 0x00,
  PROFILING_CLANG = 0x01,
  PROFILING_GCOV = 0x02,
};

class Program : public ICLDevBackendProgram_ {
public:
  Program();
  virtual ~Program();

  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;

  /**
   * @returns an unsigned long which represents the program id - this id is
   * unique per program - ; in case of failure 0 will be returned
   */
  virtual unsigned long long int GetProgramID() const override;

  /**
   * Gets the program build log
   *
   * @Returns
   *  if the log already exist , pointer to the build log will be returned;
   * otherwise NULL will be returned
   */
  virtual const char *GetBuildLog() const override;

  /**
   * @returns the virtual IR code container which represents the program
   */
  virtual const ICLDevBackendCodeContainer *
  GetProgramIRCodeContainer() const override;

  /**
   * Gets the program Code; Program code is an abstraction between which contain
   * all the kernel's codes (the executable code, the IR and with some metadata)
   */
  virtual const ICLDevBackendCodeContainer *
  GetProgramCodeContainer() const override;

  /**
   * Gets the program JIT Code Properties;
   *
   * @returns JIT Code properties interface, NULL in case of failure
   */
  virtual const ICLDevBackendProgramJITCodeProperties *
  GetProgramJITCodeProperties() const override;

  /**
   * Retrieves a pointer to a kernel object by kernel name
   *
   * @param pKernelName pointer to null terminated string that specifiy the
   * kernel name
   * @param ppKernel pointer which will be modified to point to requested kernel
   * object notice it will return the kernel object itself (not a copy)
   *
   * @returns
   *  if the program already build:
   *      CL_DEV_SUCCESS              - if kernel descriptor was successfully
   * retrieved CL_DEV_INVALID_KERNEL_NAME  - if kernel name was not found else
   *      CL_DEV_NOT_SUPPORTED will be returned
   */
  virtual cl_dev_err_code
  GetKernelByName(const char *pKernelName,
                  const ICLDevBackendKernel_ **ppKernel) const override;

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
  virtual int GetNonBlockKernelsCount() const override;

  /**
   * Gets how many kernels in the program
   *
   * @returns
   *  if the program already build:
   *      the number of the kernels in the program will be returned
   *  else
   *      0 will be returned
   */
  virtual int GetKernelsCount() const override;

  /**
   * Retrieves a pointer to a kernel object by kernel index
   *
   * @param kernelIndex is the index of the kernel should be in the range [0 ..
   * (KernelsCount - 1)]
   * @param pKernel pointer which will hold the returned kernel object
   *      notice it will return the kernel object itself (not a copy)
   *
   * @returns
   *  if the program already build:
   *      CL_DEV_SUCCESS              - if kernel descriptor was successfully
   * retrieved CL_DEV_INVALID_KERNEL_INDEX - if kernel was not found or
   * incorrect index else CL_DEV_NOT_SUPPORTED will be returned
   */
  virtual cl_dev_err_code
  GetKernel(int kernelIndex,
            const ICLDevBackendKernel_ **ppKernel) const override;

  /**
   * Gets the total amount of storage, in bytes, used by
   * program variables in the global address space.
   *
   * @returns
   *  if the program already build:
   *      the total size of global variables in program
   *  otherwise
   *      0 will be returned
   */
  virtual size_t GetGlobalVariableTotalSize() const override {
    return m_globalVariableTotalSize;
  }

  /**
   * Sets the total amount of storage, in bytes, used by
   * program variables in the global address space.
   */
  void SetGlobalVariableTotalSize(size_t size) {
    m_globalVariableTotalSize = size;
  }

  /**
   * Set non-internal global variables.
   * @param gvs a vector of global variables.
   */
  void SetGlobalVariables(std::vector<cl_prog_gv> gvs);

  /**
   * Record name and promote linkage of global ctors and dtors.
   */
  void RecordCtorDtors(llvm::Module &M);

  /**
   * Return names of global dtors
   */
  const std::vector<std::string> &GetGlobalDtors() { return m_globalDtors; }

  /**
   * Sets the Object Code Container (program will take ownership of the
   * container)
   */
  void SetObjectCodeContainer(ObjectCodeContainer *objCodeContainer);
  ObjectCodeContainer *GetObjectCodeContainer();

  /**
   * Sets the Bit Code Container (program will take ownership of the container)
   */
  void SetBitCodeContainer(BitCodeContainer *bitCodeContainer);

  /**
   * Returns hash value of the program. The hash is used in dump filename.
   */
  std::string GenerateHash();

  /*
   * Program specific methods
   */
  void SetBuildLog(const std::string &buildLog);

  /**
   * Store the given kernel set into the program
   *
   * Note: will take ownership on passed kernel set
   */
  void SetKernelSet(std::unique_ptr<KernelSet> pKernels);
  KernelSet *GetKernelSet() { return m_kernels.get(); }

  /**
   * Store the given LLVM module into the program
   *
   * Note: will take ownership on passed module
   */
  void SetModule(std::unique_ptr<llvm::Module> M);

  /**
   * Store the given LLVM module into the program
   *
   * Note: will take ownership on passed module
   */
  void SetModule(llvm::orc::ThreadSafeModule TSM);

  /**
   * Returns the LLVM module pointer
   */
  llvm::Module *GetModule();

  /**
   * Returns the LLVM module owner (smart pointer)
   */
  std::unique_ptr<llvm::Module> GetModuleOwner();

  virtual void SetBuiltinModule(llvm::SmallVector<llvm::Module *, 2> &) = 0;

  virtual void SetExecutionEngine(std::unique_ptr<llvm::ExecutionEngine>) = 0;

  virtual void SetLLJIT(std::unique_ptr<llvm::orc::LLJIT> LLJIT) = 0;

  virtual llvm::orc::LLJIT *GetLLJIT() = 0;

  virtual void LoadProfileLib() = 0;

  /// get runtime service
  RuntimeServiceSharedPtr GetRuntimeService() const { return m_RuntimeService; }

  /// set runtime service
  void SetRuntimeService(const RuntimeServiceSharedPtr &rs) {
    m_RuntimeService = rs;
  }

  /**
   * Serialization methods for the class (used by the serialization service)
   */
  virtual void Serialize(IOutputStream &ost, SerializationStatus *stats) const;
  virtual void Deserialize(IInputStream &ist, SerializationStatus *stats);

  /**
   * Checks if this program has an object binary to be loaded from
   */
  bool HasCachedExecutable() const;

  /// Set device code profiling status
  void SetCodeProfilingStatus(unsigned int stats) {
    m_codeProfilingStatus = stats;
  }

  /// Get device code profiling status
  unsigned int GetCodeProfilingStatus(void) const {
    return m_codeProfilingStatus;
  }

  unsigned int m_binaryVersion = 0;

protected:
  ObjectCodeContainer *m_pObjectCodeContainer;
  BitCodeContainer *m_pIRCodeContainer;
  std::string m_hash;
  std::string m_buildLog;
  std::unique_ptr<KernelSet> m_kernels;
  /// Runtime service. Reference counted
  RuntimeServiceSharedPtr m_RuntimeService;
  // Total size, in bytes, of program variables in the global address space
  size_t m_globalVariableTotalSize = 0;
  // Global variables (with non-internal linkage).
  std::vector<cl_prog_gv> m_globalVariables;
  // Names of global ctors sorted by priority
  std::vector<std::string> m_globalCtors;
  // Names of global dtors sorted by priority
  std::vector<std::string> m_globalDtors;
  // Whether the device code enabled clang profiling or gcov
  unsigned int m_codeProfilingStatus;
};
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
