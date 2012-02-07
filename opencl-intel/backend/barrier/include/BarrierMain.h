/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __BARRIER_MAIN_H__
#define __BARRIER_MAIN_H__

#include "llvm/Pass.h"

using namespace llvm;

namespace intel {


  /// @brief BarrierMain pass is a module pass that handles the whole
  /// barriers passes. It simply calls all other passes for you.
  class BarrierMain : public ModulePass {

  public:
      static char ID;

      /// @brief C'tor
      /// @param isDBG true if and only if we are running in DBG mode
      BarrierMain(bool isDBG);

      /// @brief D'tor
      ~BarrierMain() {}

      /// @brief Provides name of pass
      virtual const char *getPassName() const {
          return "Intel OpenCL BarrierMain";
      }

      /// @brief execute pass on given module
      /// @param M module to optimize
      /// @returns True if module was modified
      virtual bool runOnModule(Module &M);

      /// @brief return stride size per work item in special buffer
      /// @returns stride size per work item in special buffer
      unsigned int getStrideSize() {
        return m_strideSize;
      }

  private:
    /// true if and only if we are running in DBG mode
    bool m_isDBG;
    /// stride size per work item in special buffer
    unsigned int m_strideSize;

  };

} // namespace intel

#endif // __BARRIER_MAIN_H__

