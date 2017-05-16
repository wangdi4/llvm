/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __RESOLVER_H_
#define __RESOLVER_H_
#include "BuiltinLibInfo.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace intel {

/// @brief The resolver pass is a function pass which resolves
/// 'masked' function calls. It is similar to the Inliner pass,
///  however, unlike the inliner, it actually creates the implementation
///  of these functions, based on their naming convention.
class FuncResolver : public FunctionPass {
public:
  /// @brief C'tor
  FuncResolver(char &ID) : FunctionPass(ID) {
    m_unresolvedInstrCtr = 0;
    m_unresolvedLoadCtr = 0;
    m_unresolvedStoreCtr = 0;
    m_unresolvedCallCtr = 0;
    m_rtServices = NULL;
  }

  /// @brief LLVM Function pass entry
  /// @param F Function to transform
  /// @return True if changed
  virtual bool runOnFunction(Function &F);
  /// Standard LLVM interface - Nothing to preserve
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BuiltinLibInfo>();
  }

private:
  /// @brief Resolve a call-site. This is a target specific hook.
  /// @param caller Instruction to resolve
  /// @return true if this call was handled by the resolver
  virtual bool TargetSpecificResolve(CallInst* caller) = 0;

  virtual bool isBitMask(const VectorType& vecType) const { return false; }

  /// @brief Resolve a call-site
  /// @param caller Instruction to resolve
  void resolve(CallInst* caller);
  /*! \name Control Flow Helpers
   * \{ */
  /// @brief Create a control-flow around an instruction using a predicator.
  ///  Inserts an if-then-else section
  /// @param inst Instruction to predicate using control flow
  /// @param pred Predicator to use
  void CFInstruction(std::vector<Instruction*> insts, Value* pred);
  /// @brief Returns a constant deafult value for givan type. On integers
  ///        return undef, on FP return 0 to avoid denormals calculation.
  /// @param ty - given type.
  /// @returns default value for input type.
  Constant *getDefaultValForType(Type *ty);
  /// @brief Add instruction to the data structure which records which
  /// instructions need to be predicated.
  /// @param inst Instruction to predicate using control flow
  /// @param pred Predicator to use
  void toPredicate(Instruction* inst, Value* pred);
  /// @brief Performe predication on a group of instructions
  ///   which use the same predicate
  ///  Inserts multiple if-then-else section
  /// @param elements Instructions to predicate using control flow
  /// @param pred Predicator to use
  void resolvePredicate(Value* pred, std::vector<Instruction*>& elements);
  /*! \} */

  /*! \name Load Resolvers
   * \{ */
  /// @brief Resolve a call to 'masked_load'
  /// @param caller Instruction to resolve
  void resolveLoad(CallInst* caller);
  /// @brief Resolve a call to 'masked_load' of the following signature:
  ///   define <V x T> @masked_load(<V x i1> %pred, <V x T>* %ptr)
  /// @param caller Instruction to resolve
  void resolveLoadVector(CallInst* caller, unsigned align);
  /// @brief Tries to produce masked vector load
  /// @param caller original load instruction
  /// @return TRUE if masked load was produced, or FALSE otherwise
  bool isResolvedMaskedLoad(CallInst* caller);  
  /// @brief Resolve a call to 'masked_load' of the following signature:
  ///   define T @masked_loat(i1 %pred, T* %ptr)
  /// @param caller Instruction to resolve
  void resolveLoadScalar(CallInst* caller, unsigned align);
  /*! \} */

  /*! \name Store Resolvers
   * \{ */
  /// @brief Resolve a call to 'masked_store'
  /// @param caller Instruction to resolve
  void resolveStore(CallInst* caller);
  /// @brief Resolve a call to 'masked_store' of the following signature:
  ///   define void @masked_store(<V x i1> %prd, <V x T>, %vl, <V x T>* %ptr)
  /// @param caller Instruction to resolve
  void resolveStoreVector(CallInst* caller, unsigned align);
  /// @brief Tries to produce masked vector store
  /// @param caller original store instruction
  /// @return TRUE if masked store was produced, or FALSE otherwise
  bool isResolvedMaskedStore(CallInst* caller);  
  /// @brief Resolve a call to 'masked_load' of the following signature:
  ///   define @masked_loat(i1 %pred, T %val, T* %ptr)
  /// @param caller Instruction to resolve
  void resolveStoreScalar(CallInst* caller, unsigned align);
  /*! \} */

  /*! \name GP Function Resolver
   * \{ */
  /// @brief Resolve a general purpose masked function call named
  // 'masked_XXX' of the following signature:
  ///   define @masked_XXX(i1 %pred, ... )
  /// All non-predicate parameters are used to call a new external
  //  function call which is un-predicated.
  /// @param caller Instruction to resolve
  void resolveFunc(CallInst* caller);
  /// @brief Recover the original function name from the mangled masked name
  /// @param name masked mangled name
  /// @return original function name
  std::string recoverOriginalFunctionName(const std::string name);
  /*! \} */

  /*! \Fake Function Resolver
   * \{ */
  /// @brief Resolve a fake insert function call
  /// @param caller Instruction to resolve
  void resolveFakeInsert(CallInst* caller);
  /// @brief Resolve a fake extract function call
  /// @param caller Instruction to resolve
  void resolveFakeExtract(CallInst* caller);  
  /*! \} */

  /// @brief Resolve a ret-by-vector function call
  /// @param caller Instruction to resolve
  void resolveRetByVectorBuiltin(CallInst* caller);

  /// load storage
  typedef std::vector<CallInst*> bin_t;

  /// @brief Reorders load instructions which are consecutive
  /// into sequences of loads which all use the same predicate.
  /// @param BB block to re-order
  void packPredicatedLoads(BasicBlock* BB);
  /// @brief
  /// Reorder a group of loads consecutive based on their predicate.
  /// In case there are multiple loads, which are vectorized several times,
  ///  this pass with group each of the loads to groups of loads, so that
  ///  we will have a single jump per predicate.
  /// @param bin Load instructions to reorder.
  void packLoadBin(const bin_t& bin);

  /// @brief extend mask value to that expected by masked load/store BI (as the LAST argument)
  /// Vectorizer mask is of 'i1' type - which is not supported in OpenCL functions
  /// @param maskLoadStoreBI function with mask of concern.
  /// @param Mask            mask to be extended
  /// @return BitCast which is generated for the extension
  Instruction* extendMaskAsBIParameter(Function* maskLoadStoreBI, Value* Mask);

  /// @brief convert memory pointer to address space which is used by masked load/store BI (in its FIRST argument)
  /// @param maskLoadStoreBI function with memory pointer of concern.
  /// @param Ptr             memory pointer to be converted
  /// @return BitCast which is generated for the conversion
  Instruction* adjustPtrAddressSpace(Function* maskLoadStoreBI, Value* Ptr);

  /// Instructions to protect using CF guard
  //(predicator - controlled instructions)
  std::map<Value*, std::vector<Instruction*> > m_toCF;
  /// Counter of unresolved masked instructions
  int m_unresolvedInstrCtr;
  /// Counter of unresolved masked load instructions
  int m_unresolvedLoadCtr;
  /// Counter of unresolved masked store instructions
  int m_unresolvedStoreCtr;
  /// Counter of unresolved masked call instructions
  int m_unresolvedCallCtr;

  // Pointer to runtime service object
  const RuntimeServices * m_rtServices;

};

class X86Resolver : public FuncResolver {
public:
  // Pass identification, replacement for typeid
  static char ID;
  /// @brief C'tor
  X86Resolver() : FuncResolver(ID) {}

  /// @brief Provides name of pass
  virtual StringRef getPassName() const {
    return "X86Resolver";
  }

  /// @brief Resolve a call-site. This is a target specific hook.
  /// @param caller Instruction to resolve
  /// @return true if this call was handled by the resolver
  virtual bool TargetSpecificResolve(CallInst* caller) { return false; }
};

}
#endif //define __RESOLVER_H_
