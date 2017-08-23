/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __REDUCE_ALIGNEMNT_H__
#define __REDUCE_ALIGNEMNT_H__

#include "llvm/Pass.h"


using namespace llvm;

namespace intel {

  /// @brief  ReduceAlignment class replace align X access with align 1.
  class ReduceAlignment : public FunctionPass {
    public:
      // Pass identification, replacement for typeid.
      static char ID;

      /// @brief Constructor
      ReduceAlignment() : FunctionPass(ID) {}

      /// @brief  LLVM Function pass entry
      /// @param  F  Function to transform
      /// @return true if changed
      virtual bool runOnFunction(Function &F);


      /// @brief Provides name of pass
      virtual llvm::StringRef getPassName() const {
        return "ReduceAlignment";
      }

  };
} //namespace Intel

#endif // __REDUCE_ALIGNEMNT_H__
