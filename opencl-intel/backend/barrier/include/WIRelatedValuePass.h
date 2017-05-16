/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __WI_RELATED_VALUE_PASS_H__
#define __WI_RELATED_VALUE_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/SetVector.h"

#include <map>

using namespace llvm;

namespace intel {

  /// @brief WIRelatedValue pass is a analysis module pass used to
  /// distinguish between values dependent on WI id and those who are not
  class WIRelatedValue : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    WIRelatedValue();

    /// @brief D'tor
    ~WIRelatedValue() {}

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "Intel OpenCL WIRelatedValue";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

    /// @brief execute pass on given function
    /// @param F function to optimize
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

    /// @brief print data collected by the pass on the given module
    /// @param OS stream to print the info regarding the module into
    /// @param M pointer to the Module
    void print(raw_ostream &OS, const Module *M = 0) const;

    /// @brief return true if given value depends on WI Id
    /// @param pVal pointer to Value
    /// @returns true if and only if given value depends on WI Id
    bool isWIRelated(Value *pVal) {
      // TODO: if assertion fails ==> replace it with (return false)!
      //assert( m_specialValues.count(pVal) && "value has no WI related data!" );
      if ( !m_specialValues.count(pVal) ) {
        //This might happen for function parameters
        return false;
      }
      return m_specialValues[pVal];
    }

  protected:
    /// @brief Update dependency relations between all values
    void updateDeps();

    /*! \name Dependency Calculation Functions
     *  \{ */
    /// @brief Calculate the WI Id relation for the given instruction
    /// @param pInst Instruction to inspect
    /// @returns true if and only if insruction related on WI id
    void calculate_dep(Value *pVal);
    bool calculate_dep(BinaryOperator *pInst);
    bool calculate_dep(CallInst *pInst);
    bool calculate_dep(CmpInst *pInst);
    bool calculate_dep(ExtractElementInst *pInst);
    bool calculate_dep(GetElementPtrInst *pInst);
    bool calculate_dep(InsertElementInst *pInst);
    bool calculate_dep(InsertValueInst *pInst);
    bool calculate_dep(PHINode *pInst);
    bool calculate_dep(ShuffleVectorInst *pInst);
    bool calculate_dep(StoreInst *pInst);
    bool calculate_dep(TerminatorInst *pInst);
    bool calculate_dep(SelectInst *pInst);
    bool calculate_dep(AllocaInst *pInst);
    bool calculate_dep(CastInst *pInst);
    bool calculate_dep(ExtractValueInst *pInst);
    bool calculate_dep(LoadInst *pInst);
    bool calculate_dep(VAArgInst *pInst);
    /*! \} */

    /// @brief return true of given value ralated on WI Id
    /// @param pVal Value to inspect
    /// @returns true if and only if value is related on WI Id
    bool getWIRelation(Value *pVal);

    /// @brief updates function argument dependency
    /// @param pFunc function to update its argument dependency
    void updateArgumentsDep(Function* pFunc);

    /// @brief calculate the calling order of all functions need to be handled
    void calculateCallingOrder();

  private:
    /// This is barrier utility class
    BarrierUtils m_util;

    /// This is a list of all functions to be fixed in processed module
    /// that are ordered according to call graph from leaf to root
    TFunctionVector m_orderedFunctionsToAnalyze;

    typedef std::map<Value*, bool> TValuesMap;
    // Internal Data used to calculate user Analysis Data
    /// Saves which values changed in this round
    SetVector<Value*> m_changed;

    // Analysis Data for pass user
    /// This holds a map between value and it relation on WI-id (related or not)
    TValuesMap m_specialValues;

    /// OpenCL C version the module is compiled for
    unsigned m_oclVersion;
  };

} // namespace intel

#endif // __WI_RELATED_VALUE_PASS_H__

