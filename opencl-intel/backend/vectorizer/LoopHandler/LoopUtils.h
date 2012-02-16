/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __LOOP_UTILS_H__
#define __LOOP_UTILS_H__

#include "llvm/Instructions.h"
#include "llvm/BasicBlock.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;
namespace intel {
class LoopUtils {
public:

  ///@brief create WI func (get_local_size, get_global_id etc) general 
  ///       util as they all have the same signature.
  ///@param M - module to add function to.
  ///@param name - name of the function.
  ///@param retTy - return type of the function.
  static Function *getWIFunc(Module *M, StringRef name, Type *retTy);

  ///@brief creates work item call instruction.
  ///@param M - current module.
  ///@param funcName - name of the function.
  ///@param retTy - return type of the function.
  ///@param dim - argument of the WI call.
  ///@param BB - basic block to put the call at it's end.
  ///@param callName = name of the function call.
  ///@returns call to the given Work item function.
  static CallInst *getWICall(Module *M, StringRef funcName, Type *retTy,
                      unsigned dim, BasicBlock *BB, const Twine &callName="");

  ///@brief fills call vector with all calls to function named func name in 
  ///       funcToSearch
  ///@param funcName - name of functions to obtain calls to her.
  ///@param funcToSearch - Function to look call instructions in.
  ///@param calls - vector to fill.
  static void getAllCallInFunc(StringRef funcName, Function *funcToSearch,
                      SmallVectorImpl<CallInst *> &calls);

  ///@brief returns size_t type.
  ///@param M - current module.
  ///@returns as above.
  static Type *getIndTy(Module *M);

  ///@brief checks if the BB is in SubLoop of L.
  ///@pararm L - loop to serach in.
  ///@param BB - basic block to query.
  ///@returns true iff BB is in loop but not in sub loop of L.
  static bool inSubLoop(Loop *L, BasicBlock *BB);

  ///@brief fills kernels vector with openCL kernels according to metadata.
  ///@param M - module to search.
  ///@param kernels - vector to fill.
  static void GetOCLKernel(Module &M, SmallVectorImpl<Function *> &kernels);
};//LoopUtils
}// namespace intel

#endif //__LOOP_UTILS_H__
