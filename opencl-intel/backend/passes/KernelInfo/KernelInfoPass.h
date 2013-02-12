/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __KERNEL_INFO_H__
#define __KERNEL_INFO_H__

#include "TLLVMKernelInfo.h"
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include "llvm/Analysis/LoopInfo.h"

#include <map>
#include <set>
#include <vector>

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// This pass is Warraper of the Kernel Info Pass, which currently outputs
  /// two information about the kernel: Has Barrier and Execution estimation
  /// This pass should run before the Barrier Pass, createPrepareKernelArgsPass

  class KernelInfoWrapper : public ModulePass {
  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor 
    KernelInfoWrapper() : ModulePass(ID) {
    }

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "KernelInfoWrapper";
    }

    /// @brief performs KernelInfo pass on the module
    bool runOnModule(Module& Mod);

    /// @brief get Kernel Info Map
    std::map<std::string, TKernelInfo>& getKernelInfoMap() {
      return m_mapKernelInfo;
    }
  protected:
    /// @brief map between kernel and its kernel info
    std::map<std::string, TKernelInfo>  m_mapKernelInfo;
  };

  /// This pass is responsible of getting some info about the OCL
  /// kernels in the supplied program (module)
  class KernelInfo : public FunctionPass {
  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor 
    KernelInfo() : FunctionPass(ID) {
      initializeLoopInfoPass(*PassRegistry::getPassRegistry());
    }

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "KernelInfo";
    }

    /// @brief gets the required info on specific function
    /// @param pFunc ptr to function
    /// @returns True if module was modified
    bool runOnFunction(Function &Func);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.setPreservesAll();
    }

    /// @brief get Kernel Info Map
    std::map<std::string, TKernelInfo>& getKernelInfoMap() {
      return m_mapKernelInfo;
    }

  protected:

    /// @brief checks if the function has a barrier in it
    /// @param pFunc ptr to function
    bool conatinsBarrier(Function *pFunc);

    /// @brief returns approximation of the execution lenght of the given func
    /// @param pFunc ptr to function
    size_t getExecutionLength(Function *pFunc);

    /// returns the execution estimation of basic block based on it's nesting
    /// level
    size_t getExecutionEstimation(unsigned depth);

  protected:

    /// @brief map between kernel and its kernel info
    std::map<std::string, TKernelInfo>  m_mapKernelInfo;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __KERNEL_INFO_H__

