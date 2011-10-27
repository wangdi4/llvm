/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __BARRIER_PASS_H__
#define __BARRIER_PASS_H__

#include "BarrierUtils.h"
#include "DataPerBarrierPass.h"
#include "DataPerValuePass.h"
#include "DataPerInternalFunctionPass.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"

using namespace llvm;

namespace intel {

  /// @brief Barrier pass is a module pass that handles
  /// Special values
  ///   Group-A   : Alloca instructions
  ///   Group-B.1 : Values crossed barriers and the value is
  ///              related to WI-Id or initialized inside a loop
  ///   Group-B.2 : Value crossed barrier but does not suit Group-B.2
  /// Synchronize instructions
  ///   barrier(), fiber() and dummyBarrier() instructions.
  /// Get LID/GID instructions
  ///   get_local_id() will be replaced with get_new_local_id()
  ///   get_global_id() will be replaced with get_new_global_id()
  /// Non Inlined Internal Function
  ///   module functions with barriers that are called from inside the module
  class Barrier : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    Barrier();

    /// @brief D'tor
    ~Barrier() {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "Intel OpenCL Barrier";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DataPerBarrier>();
      AU.addRequired<DataPerValue>();
      AU.addRequired<DataPerInternalFunction>();
    }

    /// @brief return stride size per work item in special buffer
    /// @returns stride size per work item in special buffer
    unsigned int getStrideSize() {
      return getAnalysis<DataPerValue>().getStrideSize();
    }

  private:
    /// @brief execute pass on given function
    /// @param F function to optimize
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    /// @brief Hanlde Values of Group-A of processed function
    void fixAllocaValues();

    /// @brief Hanlde Values of Group-B.1 of processed function
    void fixSpecialValues();

    /// @brief Hanlde Values of Group-B.2 of processed function
    /// @param pInsertBefore instruction to insert new alloca before
    void fixCrossBarrierValues(Instruction *pInsertBefore);

    /// @brief Handle synchronize value of processed function
    void replaceSyncInstructions();

    /// @brief Initialize general values used to handle
    ///  special values and synchronize instructions in the processed function
    /// @param pInsertBefore instruction to insert new instructions before
    /// @param hasNoInternalCalls true if and only if processed function
    ///  has no calls function inside the module
    void initArgumentValues(Instruction *pInsertBefore, bool hasNoInternalCalls);

    /// @brief calculate address in special buffer
    /// @param offset offset of the address in the structure
    /// @param pType type of the address to calculate
    /// @param pInsertBefore instruction to insert new instructions before
    /// @returns value represnting the calculated address in the special buffer
    Value* getAddressInSpecialBuffer(
      unsigned int offset, const PointerType *pType, Instruction *pInsertBefore);

    /// @brief return instruction to insert new instruction before
    ///  if pUserInst is not a PHINode then return pUserInst. Otherwise, 
    ///  return termenator of prevBB of pUserInst with respect to pInst
    /// @param pInst value that pUserInst is using
    /// @param pUserInst instruction that is using pInst value
    /// @param expectNULL true if allow returning NULL when
    //   pUserInst is a PHINode and BB(pInst) == PrevBB(pUserInst)
    /// @returns best instruction to insert new instruction before
    Instruction* getInstructionToInsertBefore(
      Instruction *pInst, Instruction *pUserInst, bool expectNULL);

    /// @brief fix get_local_id and get_global_id
    /// @param M module to optimize
    /// @returns True if module was modified
    bool fixGetWIIdFunctions(Module &M);

    /// @brief fix non inlined internal functions
    /// @param M module to optimize
    /// @returns True if module was modified
    bool fixNonInlinedInternalFunctions(Module &M);

    /// @brief create new fixed function with extra offset arguments
    /// @param pFuncToFix original function to clone
    /// @returns new created function
    Function* createFixFunctionVersion(Function *pFuncToFix);

    /// @brief fix usage of argument by loading it from special buffer
    ///  if needed instead of reading it directly from the function arguments
    /// @param pOriginalArg original argument that need to be fix its usages
    /// @param pOffsetArg offset in special buffer to load the argument value from
    /// @param alwaysInSB if its true, then argument will always be loaded from
    ///        special buffer, otherwise need to check bad offset at execute time
    void fixArgumentUsage(Value *pOriginalArg, Value *pOffsetArg, bool alwaysInSB);

    /// @brief fix return value by storing it to special buffer at given offset
    /// @param pRetVal value to be saved, it is the function return value
    /// @param pOffsetArg offset in special buffer to save the return value at
    /// @param pNextInst new instructions will be added before this instruction
    void fixReturnValue(Value *pRetVal, Value *pOffsetArg, Instruction* pNextInst);

    /// @brief create new call instruction to new function
    ///  based on the original call instruction
    /// @param pOriginalCall original call instruction to replace
    /// @param pOriginalFunc original cloned function
    /// @param pNewFunc new function to call
    /// @returns true if new call instruction created and false otherwise
    bool createFixedCallInstruction(
      CallInst *pOriginalCall, Function *pOriginalFunc, Function *pNewFunc);

    /// @brief Remove all instructions in m_toRemoveInstructions
    void eraseAllToRemoveInstructions();

  private:
    /// This is barrier utility class
    BarrierUtils m_util;

    /// This holds the processed module context
    LLVMContext        *m_pContext;
    /// This holds size of size_t of processed module
    unsigned int       m_uiSizeT;
    /// This holds type of size_t of processed module
    const Type         *m_sizeTType;

    /// This holds instruction to be removed in the processed function/module
    TInstructionVector m_toRemoveInstructions;

    /// This holds the data per value analysis pass
    DataPerValue       *m_pDataPerValue;
    /// This holds the container of all Group-A values in processed function
    TValueVector       *m_pAllocaValues;
    /// This holds the container of all Group-B.1 values in processed function
    TValueVector       *m_pSpecialValues;
    /// This holds the container of all Group-B.2 values in processed function
    TValueVector       *m_pCrossBarrierValues;

    /// This holds the data per barrier analysis pass
    DataPerBarrier     *m_pDataPerBarrier;
    /// This holds the container of all sync instructions in processed function
    TInstructionSet    *m_pSyncInstructions;

    /// This holds the data per internal function analysis pass
    DataPerInternalFunction *m_pDataPerInternalFunction;

    /// This holds the address of current WI iteration
    Value              *m_pCurrWIValue;
    /// This holds the alloca value of processed barrier id
    Value              *m_pCurrBarrierValue;
    /// This holds the argument value of special buffer address
    Value              *m_pSpecialBufferValue;
    /// This holds the argument value of number of loop iterations over WIs
    Value              *m_pWIIterationCountValue;
    /// This holds the value represnt a bad offset
    Value               *m_pBadOffsetValue;

    /// This holds the alloca value of current stride offset in Special Buffer
    Value *m_pCurrSBValue;
    /// This holds the constant value of structure size of Special Buffer
    Value *m_pStructureSizeValue;

    typedef std::map<Function*, Value*> TMapFunctionToValue;
    /// This holds a map between function and its m_pCurrWIValue
    TMapFunctionToValue m_pCurrWIperFunction;
  };

} // namespace intel

#endif // __BARRIER_PASS_H__

