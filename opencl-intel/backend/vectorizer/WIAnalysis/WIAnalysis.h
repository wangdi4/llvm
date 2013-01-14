/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __WIANALYSIS_H_
#define __WIANALYSIS_H_
#include "RuntimeServices.h"
#include "SoaAllocaAnalysis.h"
#include "Logger.h"

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/DenseMap.h"

#include <set>

using namespace llvm;

namespace intel {

/// @brief Work Item Analysis class used to provide information on
///  individual instructions. The analysis class detects values which
///  depend in work-item and describe their dependency.
///  The algorithm used is recursive and new instructions are updated
///  according to their operands (which are already calculated).
///  @Author: Nadav Rotem
class WIAnalysis : public FunctionPass {
public:
    static char ID; // Pass identification, replacement for typeid
    WIAnalysis() : FunctionPass(ID) {
      m_rtServices = RuntimeServices::get();
      V_ASSERT(m_rtServices && "Runtime services were not initialized!");
    }

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "WIAnalysis";
    }

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F);

    /// @brief Update dependency relations between all values
    void updateDeps();

    /// @brief describes the type of dependency on the work item
    enum WIDependancy {
      UNIFORM         = 0, /// All elements in vector are constant
      CONSECUTIVE     = 1, /// Elements are consecutive
      PTR_CONSECUTIVE = 2, /// Elements are pointers which are consecutive
      STRIDED         = 3, /// Elements are in strides
      RANDOM          = 4, /// Unknown or non consecutive order
      NumDeps         = 5  /// Overall amount of dependencies
    };

    /// The WIAnalysis follows pointer arithmetic
    ///  and Index arithmetic when calculating dependency
    ///  properties. If a part of the index is lost due to 
    ///  a transformation, it is acceptible.
    ///  This constant decides how many bits need to be 
    ///  preserved before we give up on the analysis.
    static const unsigned int MinIndexBitwidthToPreserve;

    /// @brief Returns the type of dependency the instruction has on
    /// the work-item
    /// @param val Value to test
    /// @return Dependency kind
    WIDependancy whichDepend(const Value* val);

    /// @brief Inform analysis that instruction was invalidated
    /// as pointer may later be reused
    /// @param val Value to invalidate
    void invalidateDepend(const Value* val);

    /// @brief Checks if all of the control flow in the analized function is uniform.
    /// @param F function to check
    /// @return True if masks are needed
    bool isControlFlowUniform(const Function *F);

private:
    /*! \name Dependency Calculation Functions
     *  \{ */
    /// @brief Calculate the dependency type for the instruction
    /// @param inst Instruction to inspect
    /// @return Type of dependency.
    void calculate_dep(const Value* val);
    WIDependancy calculate_dep(const BinaryOperator* inst);
    WIDependancy calculate_dep(const CallInst* inst);
    WIDependancy calculate_dep(const GetElementPtrInst* inst);
    WIDependancy calculate_dep(const PHINode* inst);
    WIDependancy calculate_dep(const TerminatorInst* inst);
    WIDependancy calculate_dep(const SelectInst* inst);
    WIDependancy calculate_dep(const AllocaInst* inst);
    WIDependancy calculate_dep(const CastInst* inst);
    WIDependancy calculate_dep(const VAArgInst* inst);
    /*! \} */

    /// @brief do the trivial checking WI-dep
    /// @param I instruction to check
    /// @return Dependency type. Returns Uniform if all operands are 
    ///         Uniform, Random othewise
    WIDependancy calculate_dep_simple(const Instruction *I);

    /// @brief Provide known dependency type for requested value
    /// @param val Value to examine
    /// @return Dependency type. Returns Uniform for unknown type
    WIDependancy getDependency(const Value *val);

    /// @brief return true if there is calculated dependency type for requested value
    /// @param val Value to examine
    /// @return true if value has dependency type, false otherwise.
    bool hasDependency(const Value *val);
    

    /// @brief  LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Analysis pass preserve all
      AU.setPreservesAll();
      AU.addRequired<SoaAllocaAnalysis>();
    }

private:
    // @brief pointer to Soa alloca analysis performed for this function
    SoaAllocaAnalysis *m_soaAllocaAnalysis;
    /// Stores an updated list of all dependencies
    DenseMap<const Value*, WIDependancy> m_deps;
    /// Runtime services pointer
    RuntimeServices * m_rtServices;
    /// Iteratively one set holds the changed from the previous iteration and
    /// the other holds the new changed values from the current iteration.
    std::set<const Value*> m_changed1;
    std::set<const Value*> m_changed2;
    /// ptr to m_changed1, m_changed2 
    std::set<const Value*> *m_pChangedOld;
    std::set<const Value*> *m_pChangedNew;

  };
} // namespace


#endif //define __WIANALYSIS_H_
