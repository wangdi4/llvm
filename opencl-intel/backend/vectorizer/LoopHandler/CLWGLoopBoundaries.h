/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __CL_WG_LOOP_BOUNDARIES_H__
#define __CL_WG_LOOP_BOUNDARIES_H__

#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"

#include <set>

// class CLWGLoopBoundaries
//-----------------------------------------------------------------------------
// This class implements a pass that recieves an openCL kernel, and checks if
// it contains branches that have 2 properties:
// a. They are uniform across all work items or they can be replaced by setting
//    a boundary on the Work group loop (e.g. if get_global_id(0)<5 ).
// b. if a certain work item takes one side of the branch than it no side
//    effect instructions (load, store etc..)
// a common scenario where this properties apply are kernels of the form:
//  __kernel foo( int4 dim, .... ) {
//    if (get_global_id(0) >= dim.x || get_global_id(1) >= dim.y) return;
//    kernel code..
//
// If early exit is found than the pass removes the branch, so the only the
// kernel code remains, and calculate the new loop size to be used by the loop
// generator.
// Finally it creates a function which returns the inital global ids and loop
// and work group loop sizes in LLVM array (even if no early exits found) and
// uniform early exit (0 - skip the kernel).
// This array will conatin 2 entries per Jit dimension + 1 for unifrom early
// exit.
//
// the former example will result with
//  void __kernel foo( int4 dim, .... ) {
//    kernel code..
//
//  In case (Jit on 3 dimensions) we will get following work
//  group boundaries function:
//  [7 x size_t] __kernel WG.boundaries.foo( int4 dim, .... ) {
//    size_t dim0_lower = get_base_gid(0);
//    size_t dim0_upper = min(get_base_gid(0) + get_local_size(0), dim.x);
//    size_t dim0_size = dim0_upper - dim0_lower;
//    size_t dim1_lower = get_base_gid(1);
//    size_t dim1_upper = min(get_base_gid(1) + get_local_size(1), dim.x);
//    size_t dim1_size = dim1_upper - dim1_lower;
//    size_t dim2_lower = get_base_gid(2);
//    size_t dim2_size = get_local_size(2);
//    size_t uni = (0 < dim0_size) && (0 < dim1_size)
//    retrun [uni, dim0_lower, dim0_size, dim1_lower, dim1_size, dim2_lower, dim2_size];
//  }
//
//  In case (Jit on 1 dimension) we will get following work
//  group boundaries function:
//  [3 x size_t] __kernel WG.boundaries.foo( int4 dim, .... ) {
//    size_t dim0_lower = get_base_gid(0);
//    size_t dim0_upper = min(get_base_gid(0) + get_local_size(0), dim.x);
//    size_t dim0_size = dim0_upper - dim0_lower;
//    size_t uni = (0 < dim0_size) && (get_global_gid(0) < dim.y );
//    retrun [uni, dim0_lower, dim0_size];
//
//  This pass and Loop Generator agree on the indices of the array using
//  interface implemented in CLWGBoundDecoder Class.
//-----------------------------------------------------------------------------

namespace intel {
class CLWGLoopBoundaries : public llvm::ModulePass {
public:
  static char ID;
  /// @brief C'tor
  CLWGLoopBoundaries();
  /// @brief D'tor
  ~CLWGLoopBoundaries();
  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "CLWGLoopBoundaries";
  }

  ///@brief LLVM interface.
  ///@param M - module to process.
  ///@returns true if the module changed
  virtual bool runOnModule(llvm::Module &M);

  ///@brief additional interface to be on a function not as Pass.
  ///@param F - function to process.
  ///@returns true if the function changed
  virtual bool runOnFunction(llvm::Function &F);

  ///@brief LLVM interface.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BuiltinLibInfo>();
  };

private:
  /// struct that contain boundary early exit description.
  struct TIDDesc{
    llvm::Value *m_bound;   //actual bound
    unsigned m_dim;         //dimesnion of boundary
    bool m_isUpperBound;    //true upper bound, false lower bound
    bool m_containsVal;     //true inclusive, false exclusive
    bool m_isSigned;        //true bound is signed comparison, false not
    bool m_isGID;           //true bound is on global id, false on local id

    TIDDesc(llvm::Value *bound, unsigned dim, bool isUpperBound,
            bool containsVal, bool isSigned, bool isGID) :
      m_bound(bound), m_dim(dim), m_isUpperBound(isUpperBound),
      m_containsVal(containsVal), m_isSigned(isSigned) ,m_isGID(isGID)
    {}
  };

  ///@brief struct that contain uniform early exit description.
  struct UniDesc{
    llvm::Value *m_cond;    //condition an i1 value
    bool m_exitOnTrue;      //if true exit when condition is set.

    UniDesc (llvm::Value *cond, bool exitOnTrue) :
    m_cond(cond), m_exitOnTrue(exitOnTrue)
    {}
  };

  ///@brief helpful shortcuts for structures.
  typedef llvm::SmallVector<llvm::Value *, 4> VVec;
  typedef llvm::SmallVector<Instruction *, 4> IVec;
  typedef llvm::DenseMap<llvm::Value *, llvm::Value *> VMap;
  ///@brief Module of function being processed.
  llvm::Module *m_M;
  ///@brief Function being processed.
  llvm::Function *m_F;
  ///@brief Context of the current function.
  llvm::LLVMContext *m_context;
  ///@brief size_t type
  llvm::Type *m_indTy;
  ///@brief size_t one constant
  llvm::Constant *m_constOne;
  ///@brief size_t zero constant
  llvm::Constant *m_constZero;
  ///@brief runtime services object.
  const OpenclRuntime* m_clRtServices;
  ///@brief number of WG dimensions per dimension.
  unsigned m_numDim;
  ///@brief local_id lower bounds per dimension.
  VVec m_lowerBounds;
  ///@brief local_size lower bounds per dimension.
  IVec m_localSizes;
  ///@brief base_global_ids per dimension.
  IVec m_baseGIDs;
  ///@brief loop_size per dimension (upper_bound - lower_bound)
  IVec m_loopSizes;
  ///@brief map get***id calls to their dimension and whether the are global\local
  std::map<llvm::Value *, std::pair<unsigned, bool> > m_TIDs;
  ///@brief maps instruction to whether they are uniform or not.
  std::map<llvm::Value *, bool> m_Uni;
  ///@brief vector boundary descriptions.
  typedef SmallVector<TIDDesc, 4> TIDDescVec;
  TIDDescVec m_TIDDesc;
  ///@brief vector of uniform early exit descriptions.
  SmallVector<UniDesc, 4> m_UniDesc;
  ///@brief indicates wether there are calls to get***id with non-constant argument.
  bool m_hasVariableTid;
  ///@brief contains calls to get***id with varibale argument.
  SmallPtrSet<CallInst *, 2> m_varibaleTIDCalls;
  ///@brief the dim's entry holds the get***id of dimension dim.
  SmallVector<SmallVector<CallInst *, 4>, 4> m_TIDByDim;
  ///@brief holds instruction marked for removal.
  SmallPtrSet<Instruction *, 8> m_toRemove;
  ///@brief true iff the function call is an atomic or pipe built-in
  bool m_hasWIUniqueCalls;
  ///@brief Users of atomic/pipe functions.
  std::set<llvm::Function *> m_WIUniqueFuncUsers;
  ///@brief OpenCL C version the module is compiled for
  unsigned m_oclVersion;
  ///@brief true iff upper bound was set to be inclusive.
  bool m_rightBoundInc;

  // Statistics:
  intel::Statistic::ActiveStatsT m_kernelStats;
  // a "low quality" statistics. The fact that early exit was given up due to loads
  // does not imply that otherwise it could have been done.
  // testing for side effects is done Before checking the condition
  // is actually in a supported format, and might not include tid at all.
  Statistic Early_Exit_Givenup_Due_To_Loads;
  // set to 1 if early exit (or late start) was done for this
  // kernel. This counter is only 0 or 1,
  // even if early-exit was done for several conditions and/or dimensions.
  Statistic Created_Early_Exit;

  ///@brief Collect kernels MaxD WG loop boundaries must be always created for.
  void collectWIUniqueFuncUsers(llvm::Module &M);

  ///@brief checks if the current function has an atomic/pipe call.
  ///@returns returns true if the current function has an atomic/pipe call.
  bool currentFunctionHasWIUniqueCalls() const;

  ///@brief in case entry block branch is an early exit branch, remove the
  ///       branch and create early exit description\s.
  ///@returns true iff the entry block branch is an early exit branch.
  bool findAndCollapseEarlyExit();

  ///@brief handles the case where tidInst has cmp-select boundary.
  ///@param tidInst - tid generator to check.
  ///@returns true iff cmp-select boundary was found.
  bool handleCmpSelectBoundary(Instruction *tidInst);

  ///@brief handles the case where tidInst has min\max boundary.
  ///@param tidInst - tid generator to check.
  ///@returns true iff min\max boundary was found.
  bool handleBuiltinBoundMinMax(Instruction *tidInst);

  ///@brief check if there is cmp-select or min\max early exit pattern
  ///       and handles it.
  ///@returns true iff early exit pattern found.
  bool findAndHandleTIDMinMaxBound();

  ///@brief returns true iff BB contains instruction with side effect.
  ///@param BB - basic block to check.
  ///@retruns as above.
  bool hasSideEffectInst(llvm::BasicBlock *BB);

  ///@brief returns true if the block lead unconditionally to return
  ///       instruction with no side effect instructions.
  ///@param BB - basic block to check.
  ///@retruns as above.
  bool isEarlyExitSucc(llvm::BasicBlock *BB);

  ///@brief checks whether the branch is an early exit pattern. fills
  ///        class members with early exit description if so.
  ///@param Br - branch to analyze.
  ///@param EETrueSide - inidicate whether early exit occurs if cmp is true.
  ///@returns true iff the branch is early exit instruction.
  bool isEarlyExitBranch(BranchInst *Br, bool EETrueSide);

  ///@brief retruns true if the value as uniform across all work items.
  ///@retruns as above.
  bool isUniform(llvm::Value *v);

  ///@brief returns true if all operands are uniform.
  ///@param I - Instruction to check.
  ///@retruns as above.
  bool isUniformByOps(Instruction *I);

  ///@brief updates internal data structures with the get***id call.
  ///@param CI get***id call to process.
  ///@param isGID true iff call is get_global_id.
  void processTIDCall(CallInst *CI, bool isGID);

  ///brief updates data structures with get***id data.
  void collectTIDData();

  ///@brief root is and\or instruction. if it is recursive and\or of icmp and
  ///  uniform conditions into compares, uniformConds and returns true.
  ///@param compares - vector of compares to fill
  ///@param uniformConds - vector uniform conditions to fill
  ///@param root - original and\or to traceback
  ///@returns true iff this recurive and\or of icmp and uniform conditions
  bool collectCond (SmallVector<llvm::ICmpInst *, 4>& compares,
                    IVec &uniformConds, Instruction *root);

  ///@brief Collect tid calls, and check uniformity of instructions in the
  ///       in the input block.
  ///@param BB - basic block to check.
  void CollcectBlockData(llvm::BasicBlock *BB);

  ///@brief checks if the input cmp instruction is supported boundary compare
  ///       if so fills description of the boundary compare into eeVec.
  ///@param cmp - compare instruction to check.
  ///@param bound - the early exit boundaries.
  ///@param tid - the get***id call.
  ///@param EETrueSide - inidicate whether early exit occurs if cmp is true.
  ///@param eeVec - vector of early exit description to fill.
  ///@returns true iff cmp is supported boundary compare.
  bool obtainBoundaryEE(llvm::ICmpInst *cmp, llvm::Value **bound, llvm::Value *tid,
                            bool EETrueSide, TIDDescVec& eeVec);

  ///@brief returns loop boundaries function declaration with the original
  ///       function arguments.
  ///@retruns as above.
  llvm::Function *createLoopBoundariesFunctionDcl();

  ///@brief Recover all Values in roots and instruction leading to them
  ///       from m_F into BasicBlock BB in newF. Updates valueMap on the way.
  ///@param valueMap - maps values from m_F to their clone in newF,
  ///@param roots - original roots to recover.
  ///@param BB - basic block to put instructions in.
  ///@param newF - new Function.
  void recoverInstructions (VMap &valueMap, VVec &roots, llvm::BasicBlock *BB,
                            llvm::Function *newF);

  ///@brief traces back the two input value if one is tid dependent and the
  ///       other is uniform, assuming the two are compared.
  ///@param v1 - first input value.
  ///@param v2 - second input value.
  ///@param isCmpSigned - is this is a signed comparison.
  ///@param loc - place to put instructions to correct the bound.
  ///IMPORTANT - upon a cmp early exit the location should be the cmp
  ///intruction.
  ///@param bound - will hold the boundary values in case of success.
  ///@params tid - will hold the get***id in case of success.
  ///@returns true iff succeeded to trace back bound.
  bool traceBackBound(llvm::Value *v1, llvm::Value *v2, bool isCmpSigned,
                Instruction *loc, llvm::Value **bound, llvm::Value *&tid);

  ///@brief serves as easier interface for traceBackBound for tracking
  ///       compare instruction operands.
  ///@param cmp - compare instruction to inspect.
  ///@param bound - will hold the boundary values in case of success.
  ///@params tid - will hold the get***id in case of success.
  bool traceBackCmp(llvm::ICmpInst *cmp, llvm::Value **bound, llvm::Value *&tid);

  ///@brief serves as easier interface for traceBackBound for tracking
  ///       min\max builtins opernads.
  ///@param - CI min\max builtin to inspect.
  ///@param bound - will hold the boundary value in case of success.
  ///@params tid - will hold the get***id in case of success.
  bool traceBackMinMaxCall(CallInst *CI, llvm::Value **bound, llvm::Value *&tid);

  ///@brief updates the internal data members with cmp-select boundary.
  ///@param cmp - compare for which cmp-select pattern was found.
  ///@param bound - the early exit boundary.
  ///@param tid - the get***id call.
  ///@param isSameOrder - true iff the select and cmp agree on operands order.
  ///@returns true if cmp-select boundary pattern was found.
  bool obtainBoundaryCmpSelect(llvm::ICmpInst *cmp, llvm::Value *bound,
                               llvm::Value *tid, bool isSameOrder);

  ///@brief helper function checks that cmp predicate is supported.
  ///@param p - predicate to inspect.
  ///@returns true iff compare relational predicate is supported.
  bool isSupportedRelationalComparePredicate(llvm::CmpInst::Predicate p);

  ///@brief helper function checks that cmp predicate is <,<=.
  ///@param p - predicate to inspect.
  ///@returns true iff compare predicate is supported <,<=.
  bool isComparePredicateLower(llvm::CmpInst::Predicate p);

  ///@brief helper function checks that cmp predicate is <=,>=.
  ///@param p - predicate to inspect.
  ///@returns true iff compare predicate is supported <=,>=.
  bool isComparePredicateInclusive(llvm::CmpInst::Predicate p);

  ///@brief fills m_loopSizes, m_lowerBounds, m_localSize, m_baseGIDs
  ///       with initial values in case no boundary early exit.
  ///@param BB - basic block to put instructions.
  void fillInitialBoundaries(llvm::BasicBlock *BB);

  ///@brief Recover boundary values, and uniform early exit conditions and the
  ///       the instructions leading to them in llvm::BasicBlock BB.
  ///       Updates valueMap on the way.
  ///@param valueMap - maps values from m_F to their clone in newF,
  ///@param BB - basic block to put instructions in.
  void recoverBoundInstructions(VMap &valueMap, llvm::BasicBlock *BB);

  ///@brief Safely corrects the boundary value in case bound is on
  ///       local_id, or is inclusive\exclusive when it shouldn't.
  ///@param td - maps values from m_F to their clone in newF,
  ///@param BB - basic block to put instructions in.
  llvm::Value *correctBound(TIDDesc &td, llvm::BasicBlock *BB, llvm::Value *bound);

  ///@brief create the loop boundaries function for the current kernel.
  void createWGLoopBoundariesFunction();

  ///@brief Run through all descriptions, and update m_loopSizes,
  //        m_lowerBounds according to the boundary descriptions.
  void obtainEEBoundaries(llvm::BasicBlock *BB, VMap &valueMap);

  ///@brief retruns the uniform early exit condition.
  llvm::Value *obtainUniformCond(llvm::BasicBlock *BB, VMap &valueMap);

  ///@brief replaces tid calls with given value.
  ///@param isGID - true get_global_id , false get_local_id.
  ///@param dim - dimension argument.
  ///@param toRep - value to replace tid with.
  void replaceTidWithBound (bool isGID, unsigned dim, llvm::Value *toRep);

  /// @brief print data collected by the pass on the given module
  /// @param OS stream to print the info regarding the module into
  /// @param M pointer to the Module
  void print(raw_ostream &OS, const llvm::Module *M = 0) const;

  /// @brief sign extends the bound in case a trunc instruction was called
  /// over the result and the comparison is signed
  /// @param isCmpSigned is the comparison signed
  /// @param loc the location to insert the new instructions
  /// @param bound the current bound - will be updated by the function
  /// @param leftbound is the left bound of the early exit
  /// @param originalType the original type of the bound (NULL if no trunc
  /// istruction was performed)
  /// @param newType the type to which we are doing the sext
  /// @param inst the instruction used to compute the right bound
  /// @returns true if right bound is created and false if failed
  /// to create an early exit
  bool createRightBound(bool isCmpSigned, Instruction *loc, Value **bound,
    Value *leftBound, Type * originalType, Type * newType,
    Instruction::BinaryOps inst);

  /// @brief checks if given the left bound and the comparisonType,
  /// early exit can be created.
  /// @param comparisonType data type of the comparison
  /// @param leftbound is the left bound of the early exit
  /// @returns true if left bound fits and false otherwise
  bool doesLeftBoundFit(Type * comparisonType, Value *leftBound);
};
} // namespace


#endif //__CL_WG_LOOP_BOUNDARIES_H__
