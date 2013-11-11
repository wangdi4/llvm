/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __BARRIER_PASS_H__
#define __BARRIER_PASS_H__

#include "BarrierUtils.h"
#include "DataPerBarrierPass.h"
#include "DataPerValuePass.h"

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
    typedef std::map<std::string, unsigned int> TMapFunctionNameToBufferStride;

    static char ID;

    /// @brief C'tor
    /// @param isNativeDebug true if we are debugging natively (gdb)
    Barrier(bool isNativeDebug = false);

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
    }

    /// @brief return special buffer stride size map
    /// @param bufferStrideMap - the map to output all data into
    void getStrideMap(std::map<std::string, unsigned int>& bufferStrideMap) {
      bufferStrideMap.clear();
      bufferStrideMap.insert(m_bufferStrideMap.begin(), m_bufferStrideMap.end());
    }

  private:
    /// @brief execute pass on given function
    /// @param F function to optimize
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    /// @brief Use the stack for kernel function execution rather then the special
    ///  work item buffer. This is needed for DWARF based debugging.
    ///  Variable data will be copied out of a basic block from the special work 
    ///  item buffer up on entry. Variable data will be copied back out to the work 
    ///  item buffer upon leaving the basic block.
    /// @param pInsertBeforeBegin instruction to insert copy out of stack before
    /// @param pInsertBeforeEnd instruction to insert copy into stack before
    void useStackAsWorkspace(Instruction* pInsertBeforeBegin, Instruction* pInsertBeforeEnd);

    /// @brief Hanlde Values of Group-A of processed function
    void fixAllocaValues();

    /// @brief Hanlde Values of Group-B.1 of processed function
    void fixSpecialValues();

    /// @brief Hanlde Values of Group-B.2 of processed function
    /// @param pInsertBefore instruction to insert new alloca before
    void fixCrossBarrierValues(Instruction *pInsertBefore);

    /// @brief Handle synchronize value of processed function
    void replaceSyncInstructions();

    /// @brief Initialize general values used to handle special values
    ///  and synchronize instructions in the processed function
    /// @param pFunc function to create key values for.
    /// @param hasNoInternalCalls true if and only if processed function
    ///  has no calls function inside the module
    void createBarrierKeyValues(Function *pFunc, bool hasNoInternalCalls);

    /// @brief Get general values used to handle special values
    ///   and synchronize instructions in the processed function
    /// @param pFunc function to get key values for.
    void getBarrierKeyValues(Function* pFunc);

    /// @brief calculate address in special buffer
    /// @param offset offset of the address in the structure
    /// @param pType type of the address to calculate
    /// @param pInsertBefore instruction to insert new instructions before
    /// @param pDB Debug location, NULL if not available
    /// @returns value represnting the calculated address in the special buffer
    Value* getAddressInSpecialBuffer(
      unsigned int offset, PointerType *pType, Instruction *pInsertBefore, const DebugLoc* pDB);

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

    /// @brief create new fixed function with extra offset arguments
    /// @param pFuncToFix original function to clone
    void fixNonInlineFunction(Function *pFuncToFix);

    /// @brief fix usage of argument by loading it from special buffer
    ///  if needed instead of reading it directly from the function arguments
    /// @param pOriginalArg original argument that need to be fix its usages
    /// @param offsetArg offset in special buffer to load the argument value from
    void fixArgumentUsage(Value *pOriginalArg, unsigned int offsetArg);

    /// @brief fix return value by storing it to special buffer at given offset
    /// @param pRetVal value to be saved, it is the function return value
    /// @param offsetRet offset in special buffer to save the return value at
    /// @param pInsertBefore new instructions will be added before this instruction
    void fixReturnValue(Value *pRetVal, unsigned int offsetRet, Instruction* pInsertBefore);

    /// @brief handle parameters and return value of call instruction
    //        store relevent parametrs in sepcial buffer and load result
    //        from special buffer.
    /// @param pOriginalCall original call instruction to handle
    void fixCallInstruction(CallInst *pOriginalCall);

    /// @brief Remove all instructions in m_toRemoveInstructions
    void eraseAllToRemoveInstructions();

    /// @brief Update Map with structure stride size for each kernel
    /// @param M module to optimize
    void updateStructureStride(Module &M);

  private:
    /// This is barrier utility class
    BarrierUtils m_util;

    /// This holds the processed module context
    LLVMContext        *m_pContext;
    /// This holds size of size_t of processed module
    unsigned int       m_uiSizeT;
    /// This holds type of size_t of processed module
    Type               *m_sizeTType;

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

    typedef struct {
      /// This holds the address of current WI iteration
      Value *m_pCurrWIValue;
      /// This holds the alloca value of processed barrier id
      Value *m_pCurrBarrierValue;
      /// This holds the argument value of special buffer address
      Value *m_pSpecialBufferValue;
      /// This holds the argument value of number of loop iterations over WIs
      Value *m_pWIIterationCountValue;
      /// This holds the alloca value of current stride offset in Special Buffer
      Value *m_pCurrSBValue;
      /// This holds the constant value of structure size of Special Buffer
      Value *m_pStructureSizeValue;
    } SBarrierKeyValues;
    typedef std::map<Function*, SBarrierKeyValues> TMapFunctionToKeyValues;

    /// This holds barrier key values for current handled function
    SBarrierKeyValues *m_currBarrierKeyValues;
    /// This holds a map between function and its barrier key values
    TMapFunctionToKeyValues m_pBarrierKeyValuesPerFunction;

    typedef std::map<BasicBlock*, BasicBlock*> TMapBasicBlockToBasicBlock;
    /// This holds a map between sync basic block and previous pre sync loop header basic block
    TMapBasicBlockToBasicBlock m_preSyncLoopHeader;

    /// This holds a map between kernel function name and buffer stride size
    TMapFunctionNameToBufferStride m_bufferStrideMap;

    /// true if and only if we are running in native (gdb) dbg mode
    bool m_isNativeDBG;
  };

} // namespace intel

#endif // __BARRIER_PASS_H__

