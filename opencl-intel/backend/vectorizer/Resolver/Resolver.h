#define DEBUG_TYPE "resolver"
#ifndef __RESOLVER_H_
#define __RESOLVER_H_
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Module.h"

using namespace llvm;

namespace intel {

/// @brief The resolver pass is a function pass which resolves
/// 'masked' function calls. It is similar to the Inliner pass,
///  however, unlike the inliner, it actually creates the implementation
///  of these functions, based on their naming convention.
/// @author Nadav Rotem
class FuncResolver : public FunctionPass {
public:
  // Pass identification, replacement for typeid
  static char ID;
  /// @brief C'tor
  FuncResolver() : FunctionPass(ID) {}
  /// @brief LLVM Function pass entry
  /// @param F Function to transform
  /// @return True if changed
  virtual bool runOnFunction(Function &F);
  /// Standard LLVM interface - Nothing to preserve
  virtual void getAnalysisUsage(AnalysisUsage &AU) const { }

private:
  /// @brief Resolve a call-site. This is a target specific hook.
  /// @param caller Instruction to resolve
  /// @return true if this call was handled by the resolver
  bool TargetSpecificResolve(CallInst* caller);

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

  /// Instructions to protect using CF guard
  //(predicator - controlled instructions)
  std::map<Value*, std::vector<Instruction*> > m_toCF;

  // Cached types
  const Type *m_v16i1, *m_v16i32, *m_v16f32,
             *m_v8i1,  *m_v8i64,  *m_v8f64;
};

}
#endif //define __RESOLVER_H_
