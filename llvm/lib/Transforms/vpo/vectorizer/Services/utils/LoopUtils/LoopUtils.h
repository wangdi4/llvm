/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __LOOP_UTILS_H__
#define __LOOP_UTILS_H__

#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include <set>

using namespace llvm;
namespace intel {
class OpenclRuntime;
///@brief Helpful shortcuts for structures.
typedef SmallVector<Value *, 4> VVec;
typedef SmallVector<Instruction *, 4> IVec;
typedef SmallVector<IVec, 4> IVecVec;

///@brief struct that represent loop Region in the CFG.
struct loopRegion {
    BasicBlock *m_preHeader; // Pre header block of the loop.
    BasicBlock *m_exit;      // Exit block of the loop.

    ///@brief C'tor.
    loopRegion(BasicBlock *preHeader, BasicBlock *exit)
        : m_preHeader(preHeader), m_exit(exit) {}
};

namespace LoopUtils {
  ///@brief creates loop with loopSize iterations arround the CFG region that
  ///       begins in head and finishes in latch.
  ///@param head - The of the created loop.
  ///@param latch - The latch block of the created loop.
  ///@param loopSize - Number of loop iterations.
  ///@returns struct with pre header and exit block fot the created loop.
loopRegion createLoop(BasicBlock *head, BasicBlock *latch, Value *begin,
                      Value *increment, Value *end, std::string &name,
                      LLVMContext &C);

  ///@brief create WI func (get_local_size, get_global_id etc) general 
  ///       util as they all have the same signature.
  ///@param M - module to add function to.
  ///@param name - name of the function.
  ///@param retTy - return type of the function.
  Function *getWIFunc(Module *M, StringRef name, Type *retTy);

  ///@brief creates work item call instruction.
  ///@param M - current module.
  ///@param funcName - name of the function.
  ///@param retTy - return type of the function.
  ///@param dim - argument of the WI call.
  ///@param BB - basic block to put the call at it's end.
  ///@param callName = name of the function call.
  ///@returns call to the given Work item function.
  CallInst *getWICall(Module *M, StringRef funcName, Type *retTy,
                      unsigned dim, BasicBlock *BB, const Twine &callName="");

  ///@brief fills call vector with all calls to function named func name in 
  ///       funcToSearch
  ///@param funcName - name of functions to obtain calls to her.
  ///@param funcToSearch - Function to look call instructions in.
  ///@param calls - vector to fill.
  void getAllCallInFunc(StringRef funcName, Function *funcToSearch,
                      SmallVectorImpl<CallInst *> &calls);

  ///@brief returns size_t type.
  ///@param M - current module.
  ///@returns as above.
  Type *getIndTy(Module *M);

  ///@brief checks if the BB is in SubLoop of L.
  ///@pararm L - loop to serach in.
  ///@param BB - basic block to query.
  ///@returns true iff BB is in sub loop of L.
  bool inSubLoop(Loop *L, BasicBlock *BB);

  ///@brief checks if the Instruction is in SubLoop of L.
  ///@pararm L - loop to serach in.
  ///@param I - Instruction to query.
  ///@returns true iff I is in sub loop of L.
  bool inSubLoop(Loop *L, Instruction *I);

  ///@brief fills the input vector with blocks that are executed once in L.
  ///@pararm L - loop to process.
  ///@param DT - dominator tree t use.
  ///@param alwaysExecuteOnce - vector to fill.
  void getBlocksExecutedExactlyOnce(Loop * L, DominatorTree *DT,
                             SmallVectorImpl<BasicBlock *> &alwaysExecuteOnce);

  ///@brief fills the users function through call instructions of roots
  ///       (also indirect users) into userFuncs.
  ///@param roots - function to obtain their user functions.
  ///@param userFuncs - set to fill with users of roots
  void fillFuncUsersSet(std::set<Function *> &roots, 
                               std::set<Function *> &userFuncs);

  ///@brief fills direct user functions through instructions of functions in 
  ///       funcs set into userFuncs. If a function is introduced into 
  ///       userFuncs for the first time it will be inserted into newUsers.
  ///@param funcs - function to obtain direct users.
  ///@param userFuncs - set of users functions to fills.
  ///@param newUsers - set of newly found users.
  void fillDirectUsers(std::set<Function *> *funcs, 
      std::set<Function *> *userFuncs, std::set<Function *> *newUsers);

  /// @brief fill the user instructions (including users via other values)
  ///        of the input Function into the input vector.
  /// @param F function to get user instructions.
  /// @param userInsts vector to fill.
  void fillInstructionUsers(Function *F,
                                   SmallVectorImpl<Instruction *> &userInsts);
  void fillInternalFuncUsers(Module &m, const OpenclRuntime *rt,
                             std::set<Function *> &userFuncs);
  /// Finds all the function which are users of calls to work item functions with non-const
  /// arguments.
  void fillVariableWIFuncUsers(Module &m, const OpenclRuntime *rt,
                                     std::set<Function *> &userFuncs);

  ///@brief collect the get_***_id() in F.
  ///@param name - name of the tid generator get_global_id\ get_local_id.
  ///@param tidCalls - array of get_***_id call to fill.
  ///@param F - kernel to collect information for.
  void collectTIDCallInst(const char *name, IVecVec &tidCalls,
                                 Function *F);

}//LoopUtils
}// namespace intel

#endif //__LOOP_UTILS_H__
