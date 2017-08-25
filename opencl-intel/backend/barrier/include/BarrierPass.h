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
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"

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
    virtual llvm::StringRef getPassName() const {
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
    Value *getAddressInSpecialBuffer(unsigned int offset, PointerType *pType,
                                     Instruction *pInsertBefore,
                                     const DebugLoc *pDB);

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

    // fixSynclessTIDUsers - Patch functions which are users of get_*_id() and do not produce the values within
    void fixSynclessTIDUsers(Module &M, const TFunctionSet&);

    /// @brief Remove all instructions in m_toRemoveInstructions
    void eraseAllToRemoveInstructions();

    /// @brief Update Map with structure stride size for each kernel
    /// @param M module to optimize
    void updateStructureStride(Module &M);

    unsigned computeNumDim(Function *F);
    typedef std::vector<std::pair<ConstantInt *, BasicBlock *> >
    BarrierBBIdList;
    BasicBlock *createLatchNesting(unsigned Dim, BasicBlock *Body,
                                   BasicBlock *Dispatch, Value *Step,
                                   const DebugLoc &DL);
    // createBarrierLatch - Return the innermost nested BB (where all the action happens)
    BasicBlock *createBarrierLatch(BasicBlock *pPreSyncBB, BasicBlock *pSyncBB,
                                   BarrierBBIdList &BBId, Value *UniqueID,
                                   bool needsFence, const DebugLoc &DL);
    // Below are generators which are used for accessing and setting the function's
    // various values. Most rely on that m_currBarrierKeyValues is set for the
    // function being processed.
    Instruction *createGetCurrBarrierId(IRBuilder<> &B) {
      return B.CreateLoad(m_currBarrierKeyValues->m_pCurrBarrierId,
                          "CurrBarrierId");
    }
    Instruction *createSetCurrBarrierId(Value *V, IRBuilder<> &B) {
      return B.CreateStore(V, m_currBarrierKeyValues->m_pCurrBarrierId);
    }
    Instruction *createGetCurrSBIndex(IRBuilder<> &B) {
      return B.CreateLoad(m_currBarrierKeyValues->m_pCurrSBIndex, "SBIndex");
    }
    Instruction *createSetCurrSBIndex(Value *V, IRBuilder<> &B) {
      return B.CreateStore(V, m_currBarrierKeyValues->m_pCurrSBIndex);
    }
    Instruction *createGetLocalId(unsigned Dim, IRBuilder<> &B) {
      Value *Ptr = createGetPtrToLocalId(Dim);
      return B.CreateLoad(Ptr, AppendWithDimension("LocalId_", Dim));
    }
    Instruction *createGetLocalId(Value *LocalIdValues, Value *Dim,
                                  IRBuilder<> &B) {
      Value *Ptr = createGetPtrToLocalId(LocalIdValues, Dim, B);
      return B.CreateLoad(Ptr, AppendWithDimension("LocalId_", Dim));
    }
    Instruction *createGetLocalId(Value *LocalIdValues, unsigned Dim,
                                  IRBuilder<> &B) {
      Value *Ptr = createGetPtrToLocalId(
          LocalIdValues, ConstantInt::get(m_I32Type, APInt(32, Dim)), B);
      return B.CreateLoad(Ptr, AppendWithDimension("LocalId_", Dim));
    }
    Instruction *createSetLocalId(unsigned Dim, Value *V, IRBuilder<> &B) {
      Value *Ptr = createGetPtrToLocalId(Dim);
      return B.CreateStore(V, Ptr);
    }
    Value *createGetPtrToLocalId(unsigned Dim) {
      // For accesses to constant dimensions, cache the GEP instruction
      Value **Ptr = m_currBarrierKeyValues->m_pPtrLocalId + Dim;
      if (!*Ptr) {
        IRBuilder<> LB(m_currBarrierKeyValues->m_TheFunction->getEntryBlock()
                           .getTerminator());
        // If the LocalIDValues are generated externally to the function, make
        // sure we place the GEP before the value is accessed
        if (!isa<Instruction>(m_currBarrierKeyValues->m_pLocalIdValues))
          LB.SetInsertPoint(&*m_currBarrierKeyValues->m_TheFunction->getEntryBlock().begin());
        *Ptr = createGetPtrToLocalId(
            m_currBarrierKeyValues->m_pLocalIdValues,
            ConstantInt::get(m_I32Type, APInt(32, Dim)), LB);
      }
      return *Ptr;
    }
    Value *createGetPtrToLocalId(Value *LocalIdValues, Value *Dim,
                                 IRBuilder<> &B) {
      SmallVector<Value *, 4> Indices;
      Indices.push_back(m_Zero);
      Indices.push_back(Dim);
      return B.CreateInBoundsGEP(LocalIdValues, Indices,
                                 AppendWithDimension("pLocalId_", Dim));
    }
    Value *getLocalSize(unsigned Dim) {
      return m_currBarrierKeyValues->m_pLocalSize[Dim];
    }
    void createDebugInstrumentation(BasicBlock *Then, BasicBlock *Else);
    Instruction *createOOBCheckGetLocalId(CallInst *Call);
    // resolveGetLocalIDCall - emits code equivalent to get_local_id()
    // Call - a call where first arg is dimension. New instructions are emitted before this instruction.
    // ppLocalIdValue - Array 
    Value *resolveGetLocalIDCall(CallInst *Call);
    unsigned getNumDims() const { return m_currBarrierKeyValues->m_NumDims; }

   private:
    static const unsigned MaxNumDims = 3;
    /// This is barrier utility class
    BarrierUtils m_util;

    /// This holds the processed module context
    LLVMContext        *m_pContext;
    /// This holds size of size_t of processed module
    unsigned int       m_uiSizeT;
    /// This holds type of size_t of processed module
    Type               *m_sizeTType;
    Type               *m_I32Type;
    // Type of allocation used for storing local ID values for all dimensions
    PointerType *m_LocalIdAllocTy;
    Value* m_Zero;
    Value* m_One;

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

    struct SBarrierKeyValues {
      SBarrierKeyValues()
          : m_TheFunction(0), m_NumDims(0), m_pLocalIdValues(0),
            m_pCurrBarrierId(0), m_pSpecialBufferValue(0), m_pCurrSBIndex(0),
            m_pStructureSizeValue(0), m_currVectorizedWidthValue(0) {
        Value *V = 0;
        std::fill(m_pPtrLocalId, m_pLocalSize + MaxNumDims, V);
        std::fill(m_pLocalSize, m_pLocalSize + MaxNumDims, V);
      }
      // Pointer to function is needed because it is not always known how a
      // SBarrierKeyValues was obtained.
      Function *m_TheFunction;
      unsigned m_NumDims;
      /// This value is an array of size_t with MaxNumDims elements
      Value *m_pLocalIdValues;
      // This array of pointers is used to cache GEP instructions
      Value *m_pPtrLocalId[MaxNumDims];
      /// This holds the alloca value of processed barrier id
      Value *m_pCurrBarrierId;
      /// This holds the argument value of special buffer address
      Value *m_pSpecialBufferValue;
      /// This holds the alloca value of current stride offset in Special Buffer
      Value *m_pCurrSBIndex;
      Value *m_pLocalSize[MaxNumDims];
      /// This holds the constant value of structure size of Special Buffer
      Value *m_pStructureSizeValue;
      Value* m_currVectorizedWidthValue;
    };
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

