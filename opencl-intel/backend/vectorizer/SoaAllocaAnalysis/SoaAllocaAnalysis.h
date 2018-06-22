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

#ifndef __SOA_ALLOCA_ANALYSIS_H_
#define __SOA_ALLOCA_ANALYSIS_H_

#include "Logger.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include <map>
#include <set>

using namespace llvm;

namespace intel {

  /// @brief SOA Alloca Analysis class used to provide information on
  ///  individual instructions. The analysis class detects values which
  ///  depend on alloca instruction that can be converted to SOA alloca
  ///  instruction.
  class SoaAllocaAnalysis : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    SoaAllocaAnalysis() : FunctionPass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "SoaAllocaAnalysis";
    }

    /// @brief LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F);

    /// @brief Returns true if given value is derived from
    ///   SOA-alloca instruction
    /// @param val Value to test
    /// @return is derived from SOA-alloca instruction
    bool isSoaAllocaRelated(const Value* val);

    /// @brief Returns true if given value is derived from
    ///   SOA-alloca instruction with scalar base type
    /// @param val Value to test
    /// @return is derived from scalar SOA-alloca instruction
    bool isSoaAllocaScalarRelated(const Value* val);

    /// @brief Returns true if given value is derived from
    ///   SOA-alloca instruction with vector base type
    /// @param val Value to test
    /// @return is derived from vector SOA-alloca instruction
    bool isSoaAllocaVectorRelated(const Value* val);

    /// @brief Returns true if given value is Alloca or GEP
    ///   instructions that are derived fromSOA-alloca instruction
    /// @param val Value to test
    /// @return is Alloca/GEP derived from vector SOA-alloca instruction
    bool isSoaAllocaRelatedPointer(const Value* val);

    /// @brief Returns width of given value
    ///  (assumed it is derived from SOA alloca instruction with vector base type)
    /// @param val Value to test
    /// @return width of given value
    unsigned int getSoaAllocaVectorWidth(const Value* val);

    /// @brief print data collected by the pass on the given module
    /// @param OS stream to print the info regarding the module into
    /// @param M pointer to the Module
    void print(raw_ostream &OS, const Module *M = 0) const;

  private:
    /// @brief Returns true if given alloca instruction is supported.
    /// @param pAI - alloca instruction
    /// @param isVectorBasedType - true if and only if alloca instruction with vector base type
    /// @param arrayNestedLevel - number of nested array in alloca type
    /// @param visited - [output] list of all instructions derived from supported alloca instruction.
    /// @return true if alloca instruction is supported, false otherwise.
    bool isSupportedAlloca(const AllocaInst *pAI, bool isVectorBasedType,
      unsigned int arrayNestedLevel, std::set<const Value*> &visited);

    /// @brief Returns true if given call instruction is supported memset.
    /// @param CI - call instruction
    /// @return true if call instruction is supported memset, false otherwise.
    bool isSupportedMemset(const CallInst *CI);

  private:
    // All instrctions derived from SOA-alloca instructions,
    // mapped to base vector type length - 0 for scalar base type
    std::map<const Value*, unsigned int> m_allocaSOA;

  };
} // namespace


#endif //define __SOA_ALLOCA_ANALYSIS_H_
