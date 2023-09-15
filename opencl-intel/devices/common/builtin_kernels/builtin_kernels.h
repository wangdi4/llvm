// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#pragma once

#include "ICLDevBackendProgram.h"
#include "cl_device_api.h"
#include "cl_synch_objects.h"
#include "cl_thread.h"
#include "task_executor.h"

#include <map>
#include <string>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace BuiltInKernels {
#ifndef __OMP2TBB__
class OMPExecutorThread;
#endif
class BuiltInProgram
    : public Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_ {
public:
  BuiltInProgram(){};

  cl_dev_err_code ParseFunctionList(const char *szBuiltInKernelList);

  unsigned long long int GetProgramID() const override {
    return (unsigned long long int)this;
  }
  const char *GetBuildLog() const override { return nullptr; }
  const Intel::OpenCL::DeviceBackend::ICLDevBackendCodeContainer *
  GetProgramCodeContainer() const override {
    return nullptr;
  }
  const Intel::OpenCL::DeviceBackend::ICLDevBackendCodeContainer *
  GetProgramIRCodeContainer() const override {
    return nullptr;
  }

  cl_dev_err_code
  GetKernelByName(const char *pKernelName,
                  const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *
                      *ppKernel) const override;

  int GetNonBlockKernelsCount() const override { return GetKernelsCount(); }
  int GetKernelsCount() const override { return (int)m_mapKernels.size(); }

  virtual cl_dev_err_code
  GetKernel(int kernelIndex,
            const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ **pKernel)
      const override;

  virtual const Intel::OpenCL::DeviceBackend::
      ICLDevBackendProgramJITCodeProperties *
      GetProgramJITCodeProperties() const override {
    return nullptr;
  }
  virtual size_t GetGlobalVariableTotalSize() const override { return 0; }

  cl_ulong GetFunctionPointerFor(const char *) const override { return 0; }

  void GetGlobalVariablePointers(const cl_prog_gv **, size_t *) const override {
  }

  cl_dev_err_code Finalize() override { return CL_DEV_SUCCESS; }

protected:
  // Stores a list of MKL kernels perticipated in the Built-In kernel program
  typedef std::map<std::string,
                   Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *>
      BIKernelsMap_t;
  BIKernelsMap_t m_mapKernels;
  std::vector<Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *>
      m_listKernels;
};

class IBuiltInKernelExecutor {
public:
  virtual ~IBuiltInKernelExecutor() {}
  virtual cl_dev_err_code Execute() const = 0;
};

class IBuiltInKernel
    : public Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ {
public:
#ifndef __OMP2TBB__
  virtual cl_dev_err_code Execute(const void *pParamBuffer,
                                  OMPExecutorThread *pThread) const = 0;
#else
  virtual cl_dev_err_code
  Execute(const Intel::OpenCL::TaskExecutor::ITaskList *pList,
          const void *pParamBuffer) const = 0;
#endif

  const Intel::OpenCL::DeviceBackend::ICLDevBackendKernelProporties *
  GetKernelProporties() const override {
    return &m_mklProperties;
  }

protected:
  class BuiltInKernelProperties
      : public Intel::OpenCL::DeviceBackend::ICLDevBackendKernelProporties {
  public:
    unsigned int GetKernelPackCount() const override { return 1; }
    const size_t *GetRequiredWorkGroupSize() const override { return nullptr; }
    size_t GetBarrierBufferSize() const override { return 1; }
    size_t GetPrivateMemorySize() const override { return 1; }
    size_t GetMaxWorkGroupSize(size_t const maxWGSize,
                               size_t const) const override {
      return maxWGSize;
    }
    size_t GetMaxSubGroupSize() const override { return 1; }
    size_t GetNumberOfSubGroups(size_t, const size_t *) const override {
      return 1;
    }
    size_t GetMaxNumSubGroups(size_t const) const override { return 0; }
    size_t GetRequiredNumSubGroups() const override { return 0; };
    size_t GetImplicitLocalMemoryBufferSize() const override { return 0; }
    size_t GetKernelExecutionLength() const override { return -1; }
    void GetLocalSizeForSubGroupCount(size_t const, size_t const, size_t const,
                                      size_t *, size_t const) const override {}
    bool HasPrintOperation() const override { return false; }
    bool HasNoBarrierPath() const override { return false; }
    bool HasMatrixCall() const override { return false; }
    bool HasDebugInfo() const override { return false; }
    bool HasKernelCallOperation() const override { return false; }
    bool IsNonUniformWGSizeSupported() const override { return false; }
    unsigned int GetMinGroupSizeFactorial() const override { return 0; }
    bool IsBlock() const override { return false; }
    bool IsAutorun() const override { return false; }
    bool IsTask() const override { return false; }
    bool CanUseGlobalWorkOffset() const override { return true; }
    bool NeedSerializeWGs() const override { return false; }
    const char *GetKernelAttributes() const override { return attributes; }
    size_t GetRequiredSubGroupSize() const override { return 0; }
    void Print() const override {}

  protected:
    static const char *attributes;
  };

  BuiltInKernelProperties m_mklProperties;
};

typedef cl_dev_err_code fn_BuiltInFunctionCreate(IBuiltInKernel **ppBIKernel);

class BuiltInKernelRegistry {
public:
  static BuiltInKernelRegistry *GetInstance();

  BuiltInKernelRegistry() : m_stKernelNameStrLength(0) {}
  void RegisterBuiltInKernel(const char *szBIKernelName,
                             fn_BuiltInFunctionCreate *pCreator);

  void GetBuiltInKernelList(char *szBIKernelList, size_t stSize) const;
  size_t GetBuiltInKernelListSize() const { return m_stKernelNameStrLength; }

  cl_dev_err_code CreateBuiltInProgram(
      const char *szKernelList,
      Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_ **ppProgram);

protected:
  friend class BuiltInProgram;
  cl_dev_err_code CreateBuiltInKernel(const char *szMKLFuncName,
                                      IBuiltInKernel **pMKLExecutor) const;

  typedef std::map<std::string, fn_BuiltInFunctionCreate *> KernelCreatorMap_t;

  KernelCreatorMap_t m_mapKernelCreators;
  size_t m_stKernelNameStrLength; // Holds the total size of the list of
                                  // built-in functions

  static BuiltInKernelRegistry *g_pMKLRegistery;
};

cl_kernel_arg_address_qualifier ArgType2AddrQual(llvm::KernelArgumentType type);

#define REGISTER_BUILTIN_KERNEL(BI_KENREL_NAME, BI_CREATOR_FUNCTION)           \
  struct BI_KENREL_NAME##CreatorClassRegister {                                \
    BI_KENREL_NAME##CreatorClassRegister() {                                   \
      Intel::OpenCL::BuiltInKernels::BuiltInKernelRegistry::GetInstance()      \
          ->RegisterBuiltInKernel(#BI_KENREL_NAME, BI_CREATOR_FUNCTION);       \
    }                                                                          \
  };                                                                           \
  BI_KENREL_NAME##CreatorClassRegister class                                   \
      ##BI_KENREL_NAME##CreatorClassRegister;

#ifndef __OMP2TBB__
// Invoke OMP based function from a separate thread
// Better managment of the OpenMP threading layer
class OMPExecutorThread : public Intel::OpenCL::Utils::OclThread {
public:
  virtual ~OMPExecutorThread();

  cl_dev_err_code Execute(
      Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor &kernelToExecute);

  // OclThread overides
  int Join() override;

  static OMPExecutorThread *Create(unsigned int uiNumOfWorkers);

protected:
  OMPExecutorThread(unsigned int uiNumOfThreads);

  typedef std::pair<Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor *,
                    Intel::OpenCL::Utils::OclOsDependentEvent *>
      ExecutionRecord;

  // Queue of execution requests for the MKL library
  Intel::OpenCL::Utils::OclNaiveConcurrentQueue<ExecutionRecord>
      m_ExecutionQueue;
  // Pool of OS events to be used for sincronization between threads
  Intel::OpenCL::Utils::OclNaiveConcurrentQueue<
      Intel::OpenCL::Utils::OclOsDependentEvent *>
      m_OSEventPool;
  // Event to start execution
  Intel::OpenCL::Utils::OclOsDependentEvent m_StartEvent;

  // OclThread overides
  RETURN_TYPE_ENTRY_POINT Run() override;
};
#endif
} // namespace BuiltInKernels
} // namespace OpenCL
} // namespace Intel
