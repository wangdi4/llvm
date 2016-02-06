/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __LOOP_WI_ANALYSIS_H_
#define __LOOP_WI_ANALYSIS_H_

#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;
namespace intel {
class LoopWIAnalysis : public LoopPass {
public:
  ///@brief Pass identification.
  static char ID;

  /// @brief C'tor.
  LoopWIAnalysis();

  /// @brief destructor.
  ~LoopWIAnalysis() {}

  /// @brief LLVM interface.
  /// @param L - Loop to analyze.
  /// @param LPM - Loop Pass manager (unused).
  /// @returns true if the pass made changes (no).
  virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

  /// @brief LLVM interface.
  /// @param AU - usage of analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.setPreservesAll();
  };

  /// @brief returns true iff v is uniform.
  /// @param v - value to query.
  bool isUniform(Value *v);

  /// @brief returns true iff v is strided.
  /// @param v - value to query.
  bool isStrided(Value *v);

  /// @brief returns true iff v is random.
  /// @param v - value to query.
  bool isRandom(Value *v);

  /// @brief returns true iff v intermediate value of strided value.
  /// @param v - value to query.
  bool isStridedIntermediate(Value *v);

  /// @brief returns constant stride of v if v is strided, NULL other wise.
  /// @param v - value to query.
  Constant *getConstStride(Value *v);

  /// @brief clear data structure from information regarding v.
  /// @param v - value to process.
  void clearValDep(Value *v);

  /// @brief set v as strided value.
  /// @param v - value to set dependency.
  /// @param constStride - stride of v.
  void setValStrided(Value *v, Constant *constStride=NULL);


  enum ValDependancy {
/// Loop invarinant value. For vectors this means that this all vector
/// elements are the same (broadcast).
      UNIFORM  = 0,
/// Elements are in strides. (for vectors this assumes stride between vector
/// elements and between loops)
      STRIDED  = 1,
/// Unknown or non consecutive order
      RANDOM   = 2,
/// Overall amount of dependencies
      NumDeps  = 3
  };

private:
  /// @brief current loop.
  Loop *m_curLoop;

  /// @brief dominator tree analysis.
  DominatorTree *m_DT;

  /// @breif pre header of the current loop.
  BasicBlock *m_preHeader;

  /// @brief latch of the current loop.
  BasicBlock *m_latch;

  /// @brief header of the current loop.
  BasicBlock *m_header;

  /// @brief set containing the loop header phi nodes.
  SmallPtrSet<Value *, 4> m_headerPhi;

  /// @brief set cotaining value that are intermediate to strided vectors.
  SmallPtrSet<Value *, 4> m_stridedIntermediate;

  /// @brief maps values to their dependency.
  DenseMap<Value*, ValDependancy> m_deps;

  /// @brief maps values to their constant stride.
  DenseMap<Value*, Constant *> m_constStrides;

  /// minimal bit width to keep their dependency when trucating.
  static const unsigned int MinIndexBitwidthToPreserve;

  /// @brief computes the stride dependency of the loop header phi nodes.
  void getHeaderPHiStride();

  /// @brief computes the stride dependency of instruction in the basic block
  ///        represented by the node, and recursively calls his children.
  /// @param N - dominator tree node of the current processed basic block.
  void ScanLoop(DomTreeNode *N);

  /// @brief returns the dependency of the current value.
  /// @param val - value to query.
  ValDependancy getDependency(Value *val);

  /// @brief calculates the dependency of I.
  /// @param I - instruction to query.
  void calculate_dep(Instruction *I);

  /// @brief calculates the dependency of BO.
  /// @param BO - binary operator to query.
  /// returns BO depedency.
  ValDependancy calculate_dep(BinaryOperator *BO);

  /// @brief calculates the dependency of CI.
  /// @param CI - cast instruction to query.
  /// returns CI depedency.
  ValDependancy calculate_dep(CastInst *CI);

  /// @brief calculates the dependency of EEI.
  /// @param EEI - extractelement operator to query.
  /// return EEI depedency.
  ValDependancy calculate_dep(ExtractElementInst *EEI);

  /// @brief Checks if I is generetion of sequential ids according to the
  ///        vectorizer pattern.
  /// @param I - instruction to check
  /// @returns true if I is sequential vector.
  bool isSequentialVector(Instruction *I);

  /// @brief Checks is v is constant vector of the from <0, 1, 2, ...>
  /// @param v - value to query.
  /// true if v is consant vector of the above form.
  bool isConsecutiveConstVector(Value *v);

  /// @brief checks if SVI is a vector whose elements are the same.
  /// @param SVI - shuffle to query.
  /// return true if SVI is a broadcast.
  bool isBroadcast(ShuffleVectorInst *SVI);

  /// @brief updates the const stride of toUpdate according to the stride
  ///        of updateBy.
  /// @param toUpdate - value to update stride for.
  /// @param updateBy - value to update the stride by.
  /// @param negate - Indicator whether need to negate the constant value.
  void updateConstStride(Value *toUpadte, Value *updateBy, bool negate=false);

};

}// namespace intel

#endif //define __LOOP_WI_ANALYSIS_H_
