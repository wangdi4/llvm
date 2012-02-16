/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __CL_WG_LOOP_BOUNDARIES_H__
#define __CL_WG_LOOP_BOUNDARIES_H__

#include <set>
#include "llvm/Pass.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/DenseMap.h"
#include "OpenclRuntime.h"
#include "CLWGBoundDecoder.h"
#include "KernelAnalysis.h"


using namespace llvm;

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
//  on Volcano environment (Jit on 3 dimensions) we will get following work
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
//  on Apple environment (Jit on 1 dimension) we will get following work
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
class CLWGLoopBoundaries : public ModulePass {
public:
  static char ID;
  /// @brief C'tor
  CLWGLoopBoundaries();
  /// @brief D'tor
  ~CLWGLoopBoundaries();
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "CLWGLoopBoundaries";
  }

  ///@brief LLVM interface.
  ///@param M - module to process.
  ///@returns true if the module changed
  virtual bool runOnModule(Module &M);
  
  ///@brief additional interface to be on a function not as Pass. 
  ///       needed since on apple environment and volcano different 
  ///       functions are processed.
  ///@param F - function to process.
  ///@returns true if the function changed
  virtual bool runOnFunction(Function &F);

  ///@brief LLVM interface.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {};

private: 
  /// struct that contain boundary early exit description.
  typedef struct TIDDesc{
    Value *m_bound;         //actual bound 
    unsigned m_dim;         //dimesnion of boundary
    bool m_isUpperBound;    //true upper bound, false lower bound 
    bool m_containsVal;     //true inclusive, false exclusive
    bool m_isSigned;        //true bound is signed comparison, false not
    bool m_isGID;           //true bound is on global id, false on local id
    
    TIDDesc(Value *bound, unsigned dim, bool isUpperBound,
            bool containsVal, bool isSigned, bool isGID) :
      m_bound(bound), m_dim(dim), m_isUpperBound(isUpperBound),
      m_containsVal(containsVal), m_isSigned(isSigned) ,m_isGID(isGID)
    {}
  } TIDDesc;

  ///@brief struct that contain uniform early exit description.
  typedef struct UniDesc{
    Value *m_cond;    //condition an i1 value
    bool m_exitOnTrue;  //if true exit when condition is set.

    UniDesc (Value *cond, bool exitOnTrue) :
    m_cond(cond), m_exitOnTrue(exitOnTrue)
    {}
  } UniDesc;

  ///@brief helpful shortcuts for structures.
  typedef SmallVector<Value *, 4> VVec;
  typedef SmallVector<Instruction *, 4> IVec;
  typedef DenseMap<Value *, Value *> VMap;
  ///@brief Module of function being processed.
  Module *m_M;
  ///@brief Function being processed.
  Function *m_F;
  ///@brief Context of the current function.
  LLVMContext *m_context;
  ///@brief size_t type 
  Type *m_indTy;
  ///@brief size_t one constant
  Constant *m_constOne;
  ///@brief size_t zero constant
  Constant *m_constZero;
  ///@brief runtime services object.
  const OpenclRuntime* m_rtServices;
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
  std::map<Value *, std::pair<unsigned, bool> > m_TIDs;
  ///@brief maps instruction to whether they are uniform or not.
  std::map<Value *, bool> m_Uni;
  ///@brief vector boundary descriptions.
  typedef SmallVector<TIDDesc, 4> TIDDescVec;
  TIDDescVec m_TIDDesc;
  ///@brief vector of uniform early exit descriptions.
  SmallVector<UniDesc, 4> m_UniDesc;

  ///@brief in case entry block branch is an early exit branch, remove the
  ///       branch and create early exit description\s.
  ///@returns true iff the entry block branch is an early exit branch.
  bool findAndCollapseEarlyExit();

  ///@brief returns true iff BB contains instruction with side effect.
  ///@param BB - basic block to check.
  ///@retruns as above.
  bool hasSideEffectInst(BasicBlock *BB);

  ///@brief returns true if the block lead unconditionally to return 
  ///       instruction with no side effect instructions.
  ///@param BB - basic block to check.
  ///@retruns as above.
  bool isEarlyExitSucc(BasicBlock *BB);

  ///@brief checks whether the branch is an early exit pattern. fills
  ///        class members with early exit description if so.
  ///@param Br - branch to analyze.
  ///@param EETrueSide - inidicate whether early exit occurs if cmp is true.
  ///@returns true iff the branch is early exit instruction.
  bool isEarlyExitBranch(BranchInst *Br, bool EETrueSide);

  ///@brief retruns true if the value as uniform across all work items.
  ///@retruns as above.
  bool isUniform(Value *v);

  ///@brief returns true if all operands are uniform.
  ///@param I - Instruction to check.
  ///@retruns as above.
  bool isUniformByOps(Instruction *I);

  ///@brief root is and\or instruction. if it is recursive and\or of icmp and
  ///  uniform conditions into compares, uniformConds and returns true.
  ///@param compares - vector of compares to fill
  ///@param uniformConds - vector uniform conditions to fill
  ///@param root - original and\or to traceback
  ///@returns true iff this recurive and\or of icmp and uniform conditions
  bool collectCond (SmallVector<ICmpInst *, 4>& compares,
                    IVec &uniformConds, Instruction *root);
  
  ///@brief examine tid generator call, checks if it is uniform, and updates 
  ///       the m_TID maps to it's dimension
  ///@param CI - tid call to examine.
  void processTIDCall(CallInst *CI, StringRef &name);

  
  ///@brief Collect tid calls, and check uniformity of instructions in the
  ///       in the input block.
  ///@param BB - basic block to check.
  void CollcectBlockData(BasicBlock *BB);

  ///@brief checks if the input cmp instruction is supported boundary compare
  ///       if so fills description of the boundary compare into eeVec.
  ///@param cmp - compare instruction to check.
  ///@param EETrueSide - inidicate whether early exit occurs if cmp is true.
  ///@param eeVec - vector of early exit description to fill.
  ///@returns true iff cmp is supported boundary compare.
  bool checkBoundaryCompare(ICmpInst *cmp, bool EETrueSide, TIDDescVec& eeVec);

  ///@brief returns loop boundaries function declaration with the original
  ///       function arguments.
  ///@retruns as above.
  Function *createLoopBoundariesFunctionDcl();

  ///@brief Recover all Values in roots and instruction leading to them
  ///       from m_F into BasicBlock BB in newF. Updates valueMap on the way.
  ///@param valueMap - maps values from m_F to their clone in newF,
  ///@param roots - original roots to recover.
  ///@param BB - basic block to put instructions in.
  ///@param newF - new Function.
  void recoverInstructions (VMap &valueMap, VVec &roots, BasicBlock *BB,
                            Function *newF);

  ///@brief traces back  the boundary value for cmp. currently this supports
  ///       only cases of direct comparison and trucations.
  ///@param cmp - compare to track bound for.
  ///@param TIDInd - index of tid candiate among cmp operands
  ///@param bound - will hold the boundary value in case of success
  ///@params tid - will hold the get***id in case of success
  ///@returns true iff succeeded to trace back bound.
  bool traceBackBound(ICmpInst *cmp, unsigned TIDInd, Value *&bound, 
                      Value *&tidCand);

  ///@brief fills m_loopSizes, m_lowerBounds, m_localSize, m_baseGIDs
  ///       with initial values in case no boundary early exit.
  ///@param BB - basic block to put instructions.
  void fillInitialBoundaries(BasicBlock *BB);

  ///@brief Recover boundary values, and uniform early exit conditions and the
  ///       the instructions leading to them in BasicBlock BB.
  ///       Updates valueMap on the way.
  ///@param valueMap - maps values from m_F to their clone in newF,
  ///@param BB - basic block to put instructions in.
  void recoverBoundInstructions(VMap &valueMap, BasicBlock *BB);

  ///@brief Safely corrects the boundary value in case bound is on 
  ///       local_id, or is inclusive\exclusive when it shouldn't.
  ///@param td - maps values from m_F to their clone in newF,
  ///@param BB - basic block to put instructions in.
  Value *correctBound(TIDDesc &td, BasicBlock *BB, Value *bound);

  ///@brief create the loop boundaries function for the current kernel.
  void createWGLoopBoundariesFunction();

  ///@brief Run through all descriptions, and update m_loopSizes, 
  //        m_lowerBounds according to the boundary descriptions.
  void obtainEEBoundaries(BasicBlock *BB, VMap &valueMap);

  ///@brief retruns the uniform early exit condition.
  Value *obtainUniformCond(BasicBlock *BB, VMap &valueMap);

  /// @brief print data collected by the pass on the given module
  /// @param OS stream to print the info regarding the module into
  /// @param M pointer to the Module
  void print(raw_ostream &OS, const Module *M = 0) const;

};
} // namespace


#endif //__CL_WG_LOOP_BOUNDARIES_H__
