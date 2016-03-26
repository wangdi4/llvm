/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __LOOP_STRIDED_CODE_MOTION_H_
#define __LOOP_STRIDED_CODE_MOTION_H_

#include "LoopWIAnalysis.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace intel {
class LoopStridedCodeMotion : public LoopPass {
public:
  ///@brief Pass identification.
  static char ID;

  /// @brief C'tor.
  LoopStridedCodeMotion();

  /// @brief destructor.
  ~LoopStridedCodeMotion() {}

  /// @brief LLVM interface.
  /// @param L - Loop to transform.
  /// @param LPM - Loop Pass manager (unused).
  /// @returns true if the pass made changes.
  virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

  /// @brief LLVM interface.
  /// @param AU - usage of analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopWIAnalysis>();
    AU.setPreservesCFG();
    AU.addPreserved<LoopWIAnalysis>();
  };


private:

  /// @breif pre header of the current loop.
  BasicBlock *m_preHeader;

  /// @brief latch of the current loop.
  BasicBlock *m_latch;

  /// @brief header of the current loop.
  BasicBlock *m_header;

  /// @brief current loop.
  Loop *m_curLoop;

  /// @brief dominator tree analysis.
  DominatorTree *m_DT;

  /// @brief set containing the loop header phi nodes.
  SmallPtrSet<Value *, 4> m_headerPhi;

  /// @brief set containing the loop header phi nodes latch entries.
  SmallPtrSet<Value *, 4> m_headerPhiLatchEntries;

  /// @brief type of i32.
  Type *m_i32Ty;

  // @brief i32 zero constant.
  Constant *m_zero;

  // @brief i32 one constant.
  Constant *m_one;

  /// @brief Loop work item analysis.
  LoopWIAnalysis *m_WIAnalysis;

  /// @brief contains the strided instruction to be moved according to
  ///        dominance order.
  SmallVector<Instruction *, 16> m_orderedCandidates;

  /// @brief contains the strided instruction to be moved.
  SmallPtrSet<Value *, 16> m_instToMoveSet;

  /// @brief Obtains the current loop header phi nodes and their latch entries.
  void getHeaderPHi();

  /// @brief Scans the loop for strided values to be moved to the pre-header.
  /// @param N - dominator tree node of the current processed basic block.
  void ScanLoop(DomTreeNode *N);

  /// @brief move strided instructions to the loop pre header, create Phi node
  ///        to replace them if needed.
  void HoistMarkedInstructions();

  /// @brief fix strided instruction that use header phi node to use their
  ///        pre-header entry.
  /// @param I - instruction to fix.
  void fixHeaderPhiOps(Instruction *I);

  /// @brief creates phi node in the loop header for the moved stride
  ///        instruction if needed.
  /// @param I - instruction to creare phi node for.
  void createPhiIncrementors(Instruction *I);

  /// @brief checks if it is possible this instruction to the pre header.
  /// @param I - instruction to check.
  /// @returns true iff can hoist I.
  bool canHoistInstruction(Instruction *I);

  /// @brief get the stride to increment I between loop iterations.
  /// @param I - instruction to obtain stride for.
  /// @returns the stride for I.
  Value *getStrideForInst(Instruction *I);


  /// @brief get the vector version of stride for I if I is a vector.
  /// @param I - instruction to get vector stride.
  /// @param stride - the scalar stride for I.
  /// @returns the vector stride for I if I is a vector, stride if I is scalar.
  Value *getVectorStrideIfNeeded(Instruction *I, Value *stride);

  /// @brief screens from m_instToMoveSet values that are likely to cauese
  ///        performance degradation.
  void screenNonProfitableValues();

  /// @brief Fills vec with users of v that are out m_instToMoveSet
  /// @param v  - The value whose users are checked.
  /// @param vec - vector to fill.
  void ObtainNonHoistedUsers(Value *v, SmallVectorImpl<User *> &vec);

};
}

#endif //define __LOOP_STRIDED_CODE_MOTION_H_
