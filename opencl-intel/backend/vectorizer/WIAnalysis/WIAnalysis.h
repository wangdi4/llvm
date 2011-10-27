#ifndef __WIANALYSIS_H_
#define __WIANALYSIS_H_
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Module.h"
#include "llvm/Value.h"
#include "llvm/Support/InstIterator.h"

#include "RuntimeServices.h"
#include "Logger.h"

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
    WIDependancy calculate_dep(const Value* val);
    WIDependancy calculate_dep(const BinaryOperator* inst);
    WIDependancy calculate_dep(const CallInst* inst);
    WIDependancy calculate_dep(const GetElementPtrInst* inst);
    WIDependancy calculate_dep(const PHINode* inst);
    WIDependancy calculate_dep(const TerminatorInst* inst);
    WIDependancy calculate_dep(const SelectInst* inst);
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

    

    /// @brief  LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

private:
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
