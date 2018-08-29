// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS
#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "RuntimeService.h"
#include "Serializer.h"

#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class KernelProperties;
class KernelJITProperties;

class IKernelJITContainer : public ICLDevBackendJITContainer {
public:
  typedef void JIT_PTR(const void *, const size_t *, void *);

  virtual ~IKernelJITContainer() {}

  /*
   * Free machine code
   */
  virtual void FreeJITCode() {}

  /*
   * JITContainer methods
   */
  virtual KernelJITProperties *GetProps() const = 0;

  /**
   * Serialization methods for the class (used by the serialization service)
   */
  virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const = 0;
  virtual void Deserialize(IInputStream& ist, SerializationStatus* stats) = 0;
};

class Kernel : public ICLDevBackendKernel_, public ICLDevBackendKernelRunner {
public:
  Kernel() : m_pProps(NULL) {}

  Kernel(const std::string &name, const std::vector<cl_kernel_argument> &args,
         const std::vector<unsigned int> &memArgs, KernelProperties *pProps);

  virtual ~Kernel();

  /*
   * ICLDevBackendKernel interface implementation
   */

  /**
   * @returns an unsigned long which represents the kernel id - this id is
   * unique
   *  per kernel - ; in case of failure 0 will be returned
   */
  virtual unsigned long long int GetKernelID() const;

  /**
   * @returns a pointer to the kernel name, in case of failure NULL will be
   * returned
   */
  virtual const char *GetKernelName() const;

  /**
   * Gets the kernels paramaters count
   *
   * @returns the count of the parameters, in case of failure -1 will be
   *returned
   */
  virtual int GetKernelParamsCount() const;

  /**
   * Gets the kernel parameters description
   *
   * @returns
   *  In success will return the kernel arguments descriptor; otherwise, NULL
   *  value will be returned
   */
  virtual const cl_kernel_argument *GetKernelParams() const;

  /**
   * Gets the kernel parameters extended information
   *
   * @returns
   *  In success will return the kernel arguments information; otherwise, NULL
   *  value will be returned
   */
  virtual const cl_kernel_argument_info *GetKernelArgInfo() const;

  /**
   * Gets the description of the kernel body, the returned object contains all
   *the kernel
   * body proporties
   *
   * @returns reference to IKernelDescription object
   */
  virtual const ICLDevBackendKernelProporties *GetKernelProporties() const;

  /**
   * @param pointer Pointer to an instruction contained in this kernel's
   * JITted code.
   * @returns the the source line number from which the instruction
   * pointed to was compiled. If the pointer does not point to an
   * instruction in this kernel, or if line number information is missing,
   * this returns -1.
   */
  virtual int GetLineNumber(void *pointer) const;

  /**
   * @returns the size of argument/parameter buffer requried by the kernel
   */
  virtual size_t GetExplicitArgumentBufferSize() const;

  /**
   * @returns the required alignement of the argument buffer
   */
  virtual size_t GetArgumentBufferRequiredAlignment() const;

  /**
  * @returns the number of memory object arguments passed to the kernel
  */
  virtual unsigned int GetMemoryObjectArgumentCount() const;

  /**
  * @returns the array of indexes of memory object arguments passed to the
  * kernel
  */
  virtual const unsigned int *GetMemoryObjectArgumentIndexes() const;

  /**
   * @retruns the kernelRunner object which is responsible for running the
   * kernel
   */
  virtual const ICLDevBackendKernelRunner *GetKernelRunner() const {
    return this;
  }

  /*
   * Runner Section
   */

  /**
   * pair of helper functions which gets JIT Entry and encodes it in the host
   * the second function responsible to resolve to the real Entry Point.
   * These are needed since on MIC, for example, the JIT is serialized to the
   * device
   * and the address is resolved on the device.
   */
  // CreateEntryPointHandle - Returns a handle that can be transferred to the
  // device and later be used to retrieve the respective JIT entry point.
  // JitEP - pointer to the JIT entry point
  virtual const void *CreateEntryPointHandle(const void *JitEP) const {
    // Default implementation used by CPU
    return JitEP;
  }
  // ResolveEntryPointHandle - Returns the JIT entry point
  // Handle - handle to JIT entry point that will be resolved
  virtual const void *ResolveEntryPointHandle(const void *Handle) const {
    // Default implementation used by CPU
    return Handle;
  }

  virtual const ICLDevBackendKernel_ *GetKernel() const { return this; }

  // InitRunner - Prepares the kernel's implicit uniform arguments
  // Called on the device at most once per NDRange.
  // See description of cl_uniform_kernel_args for interpreting pKernelUniformArgs.
  virtual cl_dev_err_code InitRunner(void *pKernelUniformArgs) const;

  /**
   * @effects prepares the thread for kernel execution
   * Called on device before a set of groups is about to be excecuted
   * NOTICE: only the thread which called this api is ready to execute the
   * thread
   * @param state object to save the old state in
   * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
   */
  virtual cl_dev_err_code PrepareThreadState(ICLDevExecutionState &state) const;

  /**
   * Prepares the kernel implicit uniform arguments
   * Called on the host at most once per NDRange.
   * Explicit arguments should be filled by the RT\Device agent
   *   before execution
   * @param pKernelUniformArgs pointer to the Uniform arguments object to be
   *   updated by the device backend. See description of cl_uniform_kernel_args
   * for interpreting pKernelUniformArgs
   * @param pDevMemObjArray [internal use] pointer to the exiplicit arguments
   * @param devMemObjArrayLength [internal use] size of the array
   * @param numOfComputeUnits number of compute units ND-range is to be run on
   * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
   */
  virtual cl_dev_err_code
  PrepareKernelArguments(void *pKernelUniformArgs,
                         const cl_mem_obj_descriptor **pDevMemObjArray,
                         unsigned int devMemObjArrayLength,
                         size_t numOfComputeUnits) const;

  /**
   * Execute the specified kernel with the given arguments
   * @param pKernelUniformArgs uniformed arguments for execution
   * @param pGroupID the workgroup id to be executed
   * @param pRuntimeHandle a handle which will be passed to some built-ins
   * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
   */
  virtual cl_dev_err_code RunGroup(const void *pKernelUniformArgs,
                                   const size_t *pGroupID,
                                   void *pRuntimeHandle) const;

  /**
   * @effects restore the thread state after kernel execution
   * Called on device after a set of groups is about to be excecuted
   * @param state object to restore from
   * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
   */
  virtual cl_dev_err_code RestoreThreadState(ICLDevExecutionState &state) const;

  /*
   * Kernel class methods
   */

  /**
   * Returns the vector of kernel parameters
   */
  const std::vector<cl_kernel_argument> *GetKernelParamsVector() const;

  /**
   * Adds kernel JIT version to the kernel.
   */
  void AddKernelJIT(IKernelJITContainer *pJIT);

  /**
   * Returns the kernel JIT buffer for the specified index
   */
  const IKernelJITContainer *GetKernelJIT(unsigned int index) const;

  /**
   * Returns the count of the JIT buffer for current kernel
   */
  unsigned int GetKernelJITCount() const;

  /**
   * Asks the execution engine to free the machine code
   */
  void FreeAllJITs();

  /**
   * Calculate the local workgroup sizes if one was not specified in the input
   * work sizes
   */
  void CreateWorkDescription(cl_uniform_kernel_args *UniformImplicitArgs,
                             size_t numOfComputeUnits) const;
  /**
   * get RuntimeService
   */
  RuntimeServiceSharedPtr GetRuntimeService() const { return m_RuntimeService; }

  /**
   * set RuntimeService
   */
  void SetRuntimeService(const RuntimeServiceSharedPtr &rs) {
    assert(rs.get() && "RuntimeService is non-initialized");
    m_RuntimeService = rs;
  }

  /**
   * Serialization methods for the class (used by the serialization service)
   */
  virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
  virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);

protected:
  void DebugPrintUniformKernelArgs(const cl_uniform_kernel_args *Arguments,
                                   size_t offsetToImplicit,
                                   std::ostream &OS) const;

  std::string m_name;
  unsigned int m_CSRMask;  // Mask to be applied to set the execution flags
  unsigned int m_CSRFlags; // Flags to be set during execution
  std::vector<cl_kernel_argument> m_explicitArgs;
  unsigned int m_explicitArgsSizeInBytes;
  unsigned int m_RequiredUniformKernelArgsAlignment;
  std::vector<unsigned int> m_memArgs;
  KernelProperties *m_pProps;
  std::vector<IKernelJITContainer *> m_JITs;
  // RuntimeService. Refcounted
  RuntimeServiceSharedPtr m_RuntimeService;

private:
  // Minimum alignment in bytes for Kernel Uniform Args
  static const unsigned MinRequiredKernelArgAlignment = 8;
  // Disable copy ctor and assignment operator
  Kernel(const Kernel &);
  bool operator=(const Kernel &);
protected:
#ifdef OCL_DEV_BACKEND_PLUGINS
    // m_pluginManager is mutable to allow kernel's const methods,
    // like Kernel::PrepareKernelArguments init m_pluginManager, which
    // is initialised on each kernel
    mutable Intel::OpenCL::PluginManager m_pluginManager;
#endif
};

/**
 * Kernels holder
 *
 * The main usage of this class is for keeping the creation of the kernels
 * and updating the program as one transaction.
 */
class KernelSet {
public:
  KernelSet();

  ~KernelSet();

  void AddKernel(Kernel *pKernel);

  size_t GetCount() const { return m_kernels.size(); }

  size_t GetBlockCount() const { return m_blockKernelsCount; }

  bool Empty() const { return m_kernels.empty(); }

  Kernel *GetKernel(int index) const;

  Kernel *GetKernel(const char *name) const;

private:
  std::vector<Kernel *> m_kernels;
  size_t                m_blockKernelsCount;
};
}
}
}
