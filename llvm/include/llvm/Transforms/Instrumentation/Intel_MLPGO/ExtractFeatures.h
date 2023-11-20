//===- ExtractFeatures.h - Extract Features ---------------------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef __LLVM_TRANSFORMS_INSTRUMENTATION_MLPGO_INTEL_EXTRACTFEATRUES_H__
#define __LLVM_TRANSFORMS_INSTRUMENTATION_MLPGO_INTEL_EXTRACTFEATRUES_H__

#include "FeatureVecEnum.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Intel_MLPGO/Model.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.h"

#include <array>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace llvm;

namespace llvm {
namespace mlpgo {
struct Parameters {
  Parameters(const Module &M);

  bool DumpFeatures{false}; // need dump raw features
  bool RemoveNonRun{false}; // remove non-Run feature vector
  bool DumpJSON{false};     // dump features in JSON format
  bool DumpFeaturesWithInfRes{
      false}; // whether to dump raw features with inference results or not
  bool DumpFeaturesWithDebugInfo{
      false}; // whether to dump raw featuers with debug info or not
  bool DumpUnknownFeatures{
      false}; // whether to dump the unknown features or not

  std::unique_ptr<raw_fd_ostream> OS;
  std::unique_ptr<raw_fd_ostream> UnknownFeaturesOS;
};

using InstFeaturesMapTy = DenseMap<const Instruction *, mlpgo::MLBrFeatureVec>;

// wrapper for our targeted Instruction( which must be BranchInst or SwitchInst)
struct TerminatorInst {
  TerminatorInst(const Instruction *Inst) : Inst(Inst) {}

  static bool isSupportedBrInst(const Instruction *Inst) {
    if (const auto *BRI = dyn_cast<BranchInst>(Inst))
      return BRI->isConditional();
    return false;
    // enable the switch
    // return isa<SwitchInst>(Inst);
  }

  bool isBranch() const { return isa<BranchInst>(Inst); }

  const BasicBlock *getParent() const { return Inst->getParent(); }

  const BasicBlock *getSuccessor(unsigned i) const {
    return Inst->getSuccessor(i);
  }

  const Value *getCondition() const {
    return isBranch() ? cast<BranchInst>(Inst)->getCondition()
                      : cast<SwitchInst>(Inst)->getCondition();
  }

  unsigned getNumSuccessors() const { return Inst->getNumSuccessors(); }

  operator const Instruction *() const { return Inst; }

  const Instruction *Inst;
};

/**
 * @brief this function return if a function belongs to non-leaf, leaf or
 * call-it-self
 * @param F current function
 * @param CG call graph
 */
ProcedureType GetProcedureType(const Function &F, CallGraph &CG);

/**
 * @brief transform ProcedureType into readable string. Debug use
 */
StringRef GetProcedureType(ProcedureType ProcType);

/**
 * @brief calculate edges in Function
 */
void CalcEdgesInFunction(const Function &F, unsigned int &EdgesCountInCFG);

/**
 * @brief check if source basic block and its two successors will form a
 * triangle
 */
bool GetTriangle(const Instruction *Terminator);

/**
 * @brief judge whether a BB and its successors forms a diamond shape.
 */
bool DiamondHeuristic(const TerminatorInst BRI);

/**
 * @brief judge whether function has an entry block has a ret successor
 */
bool GetFunctionStartWithRet(const Function *F);

/**
 * @brief check if the successor has calling events
 */
bool ExtractSuccessorCall(const BasicBlock *BB);

/**
 * @brief check whether the successor uses the register which was once used by
 * the terminator branch of source basic block
 */
bool ExtractSuccessorUseDef(TerminatorInst BRI, const BasicBlock *BB);

/**
 * @brief check whether the successor has store instruction
 */
bool ExtractSuccessorStore(const BasicBlock *BB);

/**
 * @brief judge if Src to Dst is loop entry
 */
bool isLoopEntering(const LoopInfo &LI,
                    const BranchProbabilityInfo::SccInfo &Scc,
                    const BasicBlock *Src, const BasicBlock *Dst);

/**
 * @brief the subroutine called by ExtractInstrFeatures to get the features of
 * successor part
 */
void ExtractSuccessorFeatures(
    const BranchProbabilityInfo *BPI, const BranchProbabilityInfo *OldBPI,
    const BasicBlock *Src, const BasicBlock *Dst, const LoopInfo &LI,
    const DominatorTree &DT, const PostDominatorTree &PostDT,
    const BranchProbabilityInfo::SccInfo &Scc,
    SmallPtrSet<const BasicBlock *, 8> &UnlikelyBlocks,
    BrSuccFeaturesT &SuccFeatures);

/**
 * @brief Firstly extract specific features which require a lot of extra
 * information such as SCC, LoopInfo
 */
std::optional<mlpgo::MLBrFeatureVec> ExtractInstFeatures(
    const Instruction *Terminator, const Function &F, ProcedureType ProcType,
    const LoopInfo &LI, const DominatorTree &DT,
    const PostDominatorTree &PostDT, const BranchProbabilityInfo::SccInfo &Scc,
    std::set<std::pair<const BasicBlock *, const BasicBlock *>> BackEdgesSet,
    mlpgo::Parameters &Parameter, const BranchProbabilityInfo &OldBPI,
    const BranchProbabilityInfo &BPI, bool IsInference = false);

bool ValidateFeatureVec(MLBrFeatureVec &FeatureVec,
                        mlpgo::Parameters &Parameter);

void ExtractFeatures(Function &F, mlpgo::Parameters &Parameter,
                     const BranchProbabilityInfo &OldBPI, CallGraph &CG,
                     InstFeaturesMapTy &Inst2FeaturesMap,
                     std::map<const BasicBlock *, uint64_t> &BBCountValueMap);

/**
 * @brief transform data type value to raw feature value
 */
int32_t GetTypeDesc(const Type *Ty);

/**
 * @brief get operand function as its raw feature value
 */
int32_t GetOperandFunctionAsInt(const Instruction *Inst);

std::vector<unsigned>
GenFeatureVecWithInferenceRes(const Instruction *Inst,
                              const MLBrFeatureVec &FeatureVec);

void FreeInferenceModel();

void DumpTrainingSet(
    Function &F, const InstFeaturesMapTy &Inst2FeaturesMap,
    mlpgo::Parameters &Parameter,
    std::map<const BasicBlock *, uint64_t> &BBCountValueMap,
    std::map<std::pair<const BasicBlock *, const BasicBlock *>, uint64_t>
        EdgeCountValueMap);

} // end namespace mlpgo
} // end namespace llvm

#endif //__LLVM_TRANSFORMS_INSTRUMENTATION_MLPGO_INTEL_EXTRACTFEATRUES_H__
