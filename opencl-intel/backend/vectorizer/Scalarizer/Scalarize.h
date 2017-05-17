/*==================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __SCALARIZE_H__
#define __SCALARIZE_H__

#include "BuiltinLibInfo.h"
#include "Logger.h"
#include "SoaAllocaAnalysis.h"
#include "TargetArch.h"
#include "VectorizerCommon.h"

#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/ilist.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"

#include <string>
#include <sstream>

namespace intel {

/// @brief Scalarization pass used for converting code in functions
///  which operate on vector types, to work on scalar types (by breaking
///  data elements to scalars, and breaking each vector operation
///  to several scalar operations).
///  Functions are also replaced (similar to instructions), according
///  to data received from RuntimeServices.
class ScalarizeFunction : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  ScalarizeFunction(Intel::ECPU Cpu = Intel::DEVICE_INVALID);
  ~ScalarizeFunction();

  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "ScalarizeFunction";
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<SoaAllocaAnalysis>();
    AU.addRequired<BuiltinLibInfo>();
  }

  virtual bool runOnFunction(Function &F);

private:

  /// @brief main Method for dispatching instructions (according to inst type) for scalarization
  /// @param I instruction to dispatch
  void dispatchInstructionToScalarize(Instruction *I);

  /// @brief Instructions which cannot be Scalarized reach this function.
  ///  They may have a vector value, so create empty SCM entries for them if needed,
  ///  and also re-create any vector input which may have been scalarized
  /// @param Inst instruction to work on
  void recoverNonScalarizableInst(Instruction *Inst);

  /*! \name Scalarizarion Functions
   *  \{ */
  /// @brief Scalarize an instruction
  /// @param I Instruction to scalarize
  void scalarizeInstruction(BinaryOperator *BI, bool supportsWrap);
  void scalarizeInstruction(CmpInst *CI);
  void scalarizeInstruction(CastInst *CI);
  void scalarizeInstruction(PHINode *CI);
  void scalarizeInstruction(SelectInst *SI);
  void scalarizeInstruction(ExtractElementInst *SI);
  void scalarizeInstruction(InsertElementInst *II);
  void scalarizeInstruction(ShuffleVectorInst *SI);
  void scalarizeInstruction(CallInst *CI);
  void scalarizeInstruction(AllocaInst *CI);
  void scalarizeInstruction(GetElementPtrInst *CI);
  void scalarizeInstruction(LoadInst *CI);
  void scalarizeInstruction(StoreInst *CI);


/// @brief this function handles cases when call instruction that should be
  ///  packetized has vector argument or vector return value. we assume that in
  ///  theses cases that arg\return should have soa form, so we force assembly
  ///  of the vectors args from scalars, and update the SCM with extracts of
  ///  the vector return - these will be transformed by the packetizer into
  ///  vectors
  /// @param CI - call instruction to handle
  void scalarizeCallWithVecArgsToScalarCallsWithScalarArgs(CallInst* CI);

  ///@brief this function handles case when scalar builtin return vector by
  /// creating extracts of the return value, and update the SCM as if the Call
  /// was generated with scalar arguments.
  ///@param callerInst call instrunction to handle
  void handleScalarRetVector(CallInst* callerInst);
  /*! \} */

  ///@brief this function handles case when scalar builtin return vector by
  ///       creating extracts of the return value, but does *not* update the SCM.
  ///       Instead it returns the resulting scalarized extracts and leaves updating
  ///       the SCM to the caller.
  ///@param callerInst call instrunction to handle
  ///@param scalarizedExtracts Container for scalarized extracts to be written to
  void handleScalarRetVector(CallInst* callerInst, SmallVectorImpl<Value*>& scalarizedExtracts);
  /*! \} */

  ///@brief Check if worth scalarize Load/Store with given vector type
  ///@param type vector type of Load/Store (can be NULL)
  ///@return true if Load/Store worth scalarize, false otherwise
  bool isScalarizableLoadStoreType(VectorType *type);

  /*! \name Scalarizarion Utility Functions
   *  \{ */

  /// @brief Takes a vector value, and returns the scalarized "breakdown" of that value
  /// @param retValues Array for returning scalar elements in
  /// @param retIsConstant Return (by reference) if the given value is a constant
  /// @param origValue Vector value to obtain elements from
  /// @param origInst Instruction for which service is requested (may be used as insertion point)
  void obtainScalarizedValues(Value *retValues[], bool *retIsConstant,
                              Value *origValue, Instruction *origInst);


  /// @brief a set contains vector from original kernel that need to be used after sclarization
  DenseSet<Value*> m_usedVectors;

  /// @brief update museVectors set with the vectori value to be obtained at when scalarization finish
  /// @param vectorVal Vector being added to set
  void obtainVectorValueWhichMightBeScalarized(Value *vectorVal);

  /// @brief Given a vector value, check if still exists, or rebuild from scalar elements
  ///  this funciton assumes the SCM map is updated and thus should be run after sclarization is finished
  /// @param vectorVal Vector being checked
  void obtainVectorValueWhichMightBeScalarizedImpl(Value * vectorVal);

  /// @brief obtaining vector values that are needed after scalarizaion by invoking
  ///  obtainVectorValueWhichMightBeScalarizedImpl over m_usedVectors
  void resolveVectorValues();

  ///@brief this function forces assembly of vectors from their parts, no metter if this
  /// vector was originally sclarized or not. this is used to allow packetization of scalar
  /// built-ins containing vector argumetns in order to obtain soa form of the vector when packetizing
  ///@param vectorVal - vector value to force it's assembly
  ///@param loc - location to put vector assembly instructions
  ///@return the assembled vector
  Value *obtainAssembledVector(Value *vectorVal, Instruction *loc);

  /// @brief Resolve deferred insts (Values which were scalarized with dummies)
  void resolveDeferredInstructions();

  /*! \} */

  ///@brief this function gets the function type and attributes of giving scalar function name.
  /// If it is a ret-by-vector functionm then it calculates the function type and attributes,
  /// otherwise it check the function in the runtime module.
  /// It returns true upon success and false otherwise.
  ///@param strScalarFuncName - scalar function name
  ///@param funcType - [output] place to return function type
  ///@param funcAttr - [output] place to return function attribute
  ///@return true if succeeded and false otherwise.
  bool getScalarizedFunctionType(std::string &strScalarFuncName, FunctionType*& funcType, AttributeSet& funcAttr);

  /// @brief Pointer to current function's context
  LLVMContext *m_moduleContext;
  /// @brief Accessor to current function's context
  LLVMContext& context() {return *m_moduleContext;}
  /// @brief Pointer to current function
  Function *m_currFunc;
  /// @brief Pointer to runtime service object
  const RuntimeServices *m_rtServices;
  /// @brief Set containing all the removed instructions in the function.
  DenseSet<Instruction*> m_removedInsts;
  /// @brief Counters for "transpose" statistics
  int m_transposeCtr[Instruction::OtherOpsEnd];

  /// @brief The SCM (scalar conversions map). Per each value - map of its scalar elements
  struct SCMEntry
  {
    Value *scalarValues[MAX_INPUT_VECTOR_WIDTH];
    bool isOriginalVectorRemoved;
  };
  DenseMap<Value*, SCMEntry*> m_SCM;

  /// @brief called to create a new SCM entry. If entry already exists - return it instead
  /// @param origValue Value pointer to search in SCM
  /// @return pointer to found or created SCM entry
  SCMEntry *getSCMEntry(Value *origValue);

  /// @brief called to update values in SCM entry
  /// @param entry SCM entry to update
  /// @param scalarValues array of values to place in SCMEntry
  /// @param origValue Value which is the key of the SCMEntry
  /// @param isOrigValueRemoved True if original (vector) value was erased during scalarization
  /// @param matchDbgLoc True if we want to match debug loc of the scalar value to orig Value.
  void updateSCMEntryWithValues(SCMEntry *entry, Value *scalarValues[],
                                const Value *origValue, bool isOrigValueRemoved,
                                bool matchDbgLoc = true);

  /// @brief returns an SCM entry if it exists. otherwise return NULL.
  /// @param origValue Value used as key in SCM
  /// @return SCMEntry if found, NULL otherwise
  SCMEntry *getScalarizedValues(Value *origValue);

  /// @brief release all allocations of SCM entries
  void releaseAllSCMEntries();

  // @brief pointer to Soa alloca analysis performed for this function
  SoaAllocaAnalysis *m_soaAllocaAnalysis;

  /// @brief An array of available SCMEntry's
  SCMEntry *m_SCMAllocationArray;

  /// @brief Index, in "SCMAllocationArray", of next free SCMEntry
  unsigned m_SCMArrayLocation;

  /// @brief Vector containing all the "SCMAllocationArray" arrays which were allocated
  SmallVector<SCMEntry*, 4> m_SCMArrays;

  /// @brief The DRL (Deferred resolution list).
  typedef struct DRLEntry
  {
    Value *unresolvedInst;
    Value *dummyVals[MAX_INPUT_VECTOR_WIDTH];
  } DRLEntry;
  SmallVector<DRLEntry, 4> m_DRL;

  /*! \name Pre-Scalarization function arguments scan
   *  \{ */

  /// @brief Data structure for holding "real" inputs/output of function call
  //         first - arguments of function
  //         second - retuns of function. There may be more than one return value, e.g. sincos
  typedef std::pair< SmallVector<Value*, 4>, SmallVector<Value*, 4>  > funcRootsVect;
  /// @brief Some getters which access funcRootsVect and make the code more readable
  static SmallVectorImpl<Value*>& getReturns(funcRootsVect& FRV) { return FRV.second; }
  static const SmallVectorImpl<Value*>& getReturns(const funcRootsVect& FRV) { return FRV.second; }
  static SmallVectorImpl<Value*>& getArgs(funcRootsVect& FRV) { return FRV.first; }
  static const SmallVectorImpl<Value*>& getArgs(const funcRootsVect& FRV) { return FRV.first; }

  /// @brief Map of all function calls, to strucutres of "real" inputs/output
  DenseMap<CallInst*, funcRootsVect> m_scalarizableRootsMap;

  /// @brief Initiate scanning of the code prior to scalarization
  ///  used for rooting input arguments of built-in and special-case functions
  void preScalarizeScanFunctions();

  /// @brief Validate a CALL instruction - to see if it can be scalarized
  ///  Trace the roots of all the argumets and return value -
  ///  in case there are type casts and dereferencing by pointers
  /// @param CI The CALL instruction
  /// @param rootVals structure to fill with "real" input values
  /// @return True if found function to scalarize (and filled rootVals)
  bool scanFunctionCall(CallInst *CI, funcRootsVect &rootVals);

  /// @brief flag to enable scatter/gather to/from memory.
  bool UseScatterGather;

  /// @brief cpuid
  Intel::ECPU m_Cpu;

  /// @brief This holds DataLayout of processed module
  const DataLayout *m_pDL;

};

} // Namespace



#endif // __SCALARIZE_H__


