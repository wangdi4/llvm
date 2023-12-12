//===- ExtractFeatures.cpp - Extract Features -------------------*- C++ -*-===//
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

#include "llvm/Transforms/Instrumentation/Intel_MLPGO/ExtractFeatures.h"

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureValidation.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

using namespace llvm;
using namespace mlpgo;

#define DEBUG_TYPE "mlpgo"

namespace llvm {
namespace mlpgo {

Parameters::Parameters(const Module &M) {

#ifndef INTEL_PRODUCT_RELEASE
  std::optional<std::string> OutputFile = sys::Process::GetEnv("MLPGO_OUTPUT");
  std::optional<std::string> NeedRemoveNonRun =
      sys::Process::GetEnv("MLPGO_REMOVE_NON_RUN");
  std::optional<std::string> NeedDumpFeaturesWithInfRes =
      sys::Process::GetEnv("MLPGO_DUMP_WITH_INF");
  std::optional<std::string> NeedDumpWithDebugInfo =
      sys::Process::GetEnv("MLPGO_DUMP_WITH_DEBUG_INFO");
  DumpJSON = !sys::Process::GetEnv("MLPGO_DUMP_PLAIN_FORMAT").has_value();
  std::optional<std::string> UnknownFeaturesOutput =
      sys::Process::GetEnv("MLPGO_DUMP_UNKNOWN_FEATURES");

  std::thread::id tid = std::this_thread::get_id();
  std::stringstream ss;
  ss << tid;

  if (OutputFile) {
    std::error_code ECOS;
    OS = std::make_unique<raw_fd_ostream>(
        OutputFile.value() + "-pid" + ss.str(), ECOS, sys::fs::CD_OpenAlways,
        sys::fs::FA_Write, sys::fs::OF_Append);

    if (ECOS) {
      WithColor::error() << "Cannot open output file" << OutputFile.value()
                         << " for writing.\n";
    } else {
      DumpFeatures = true;
    }
  }

  if (UnknownFeaturesOutput) {
    std::error_code ECOS;
    UnknownFeaturesOS = std::make_unique<raw_fd_ostream>(
        UnknownFeaturesOutput.value() + "-pid" + ss.str(), ECOS,
        sys::fs::CD_OpenAlways, sys::fs::FA_Write, sys::fs::OF_Append);

    if (ECOS) {
      WithColor::error() << "Cannot open unknown feature output file"
                         << UnknownFeaturesOutput.value() << " for writing.\n";
    } else {
      DumpUnknownFeatures = true;
    }
  }

  if (NeedRemoveNonRun) {
    RemoveNonRun = true;
  }

  if (NeedDumpFeaturesWithInfRes) {
    DumpFeaturesWithInfRes = true;
  }

  if (NeedDumpWithDebugInfo) {
    DumpFeaturesWithDebugInfo = true;
  }
#endif // INTEL_PRODUCT_RELEASE
}

// Maps the name to a number provided by a file in JSON format
static uint64_t mapName2NumHelper(const StringRef &Name,
                                  const StringRef &JSONEntryName) {

  std::optional<std::string> FileName;
#ifndef INTEL_PRODUCT_RELEASE
  FileName = sys::Process::GetEnv("MLPGO_NAME_MAPPING_FILE");
#endif // INTEL_PRODUCT_RELEASE

  if (!FileName)
    return 0;

  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> Buffer =
      llvm::MemoryBuffer::getFile(FileName.value(), true);

  if (!Buffer)
    report_fatal_error(Twine("Error opening JSON file"),
                       /*no crash dump*/ false);

  Expected<json::Value> Value = json::parse(Buffer.get()->getBuffer());

  if (!Value)
    report_fatal_error(Twine("Cannot parse JSON"), /*no crash dump*/ false);

  if (json::Object *Obj = Value->getAsObject())
    if (json::Object *NameArray = Obj->getObject(JSONEntryName))
      if (std::optional<int64_t> Num = NameArray->getInteger(Name))
        return Num.value();

  return 1;
}

ProcedureType GetProcedureType(const Function &F, CallGraph &CG) {

  // get the Node of current Function in calling graph
  const auto *Node = CG.getOrInsertFunction(&F);
  if (Node->size() == 0)
    return ProcedureType::Leaf;

  // detect whether the function is recursive or will form a loop
  scc_iterator<CallGraph *> CGI = scc_begin(&CG);
  while (!CGI.isAtEnd()) {
    const auto &CGs = *CGI;
    assert(CGs.size() >= 1);

    // in a non SCC graph node find the current function
    if (CGs.size() > 1) {
      if (std::find(CGs.begin(), CGs.end(), Node) != CGs.end())
        return ProcedureType::CallSelf;
    } else if (CGs.front() == Node) { // recursive
      if (CGI.hasCycle())
        return ProcedureType::CallSelf;
      else
        break;
    }
    ++CGI;
  }

  for (const auto &record : *Node) {

    // reference edge
    if (!record.first.has_value())
      return ProcedureType::NonLeaf;

    // indirect call or the destination function is not intrinsic
    const auto *Callee = record.second->getFunction();
    if (!Callee || !Callee->isIntrinsic())
      return ProcedureType::NonLeaf;
  }

  // intrinsic by llvm
  return ProcedureType::OnlyIntrinsic;
}

StringRef GetProcedureType(ProcedureType ProcType) {
  switch (ProcType) {
  case ProcedureType::Leaf:
    return "Leaf";
  case ProcedureType::NonLeaf:
    return "NonLeaf";
  case ProcedureType::CallSelf:
    return "CallSelf";
  default: // OnlyIntrinsic:
    return "OnlyIntrinsic";
  }
}

void CalcEdgesInFunction(const Function &F, unsigned int &EdgesCountInCFG) {
  for (const auto &BB : F) {
    EdgesCountInCFG += BB.getTerminator()->getNumSuccessors();
  }
}

bool GetTriangle(const Instruction *Terminator) {
  int NumOfSuccessors = Terminator->getNumSuccessors();
  std::set<const BasicBlock *> SuccessorSet;

  // ignore switch
  if (NumOfSuccessors > 2) {
    return false;
  }

  for (int i = 0; i < NumOfSuccessors; ++i) {
    SuccessorSet.insert(Terminator->getSuccessor(i));
  }

  // not as strict as diamond heuristic
  // as long as it happens a trangle type then it will be recorded
  // even though the successors may have branch edges out of the three edges of
  // triangle
  for (int i = 0; i < NumOfSuccessors; ++i) {
    const BasicBlock *Succ = Terminator->getSuccessor(i);
    const Instruction *GrandTerminator = Succ->getTerminator();
    int NumOfGrandSuccessors = GrandTerminator->getNumSuccessors();

    for (int j = 0; j < NumOfGrandSuccessors; ++j) {
      const BasicBlock *GrandSucc = GrandTerminator->getSuccessor(j);

      // ensure the grand succ not succ itself
      if (SuccessorSet.find(GrandSucc) != SuccessorSet.end() &&
          GrandSucc != Succ) {
        return true;
      }
    }
  }

  return false;
}

bool DiamondHeuristic(const TerminatorInst BRI) {
  int NumOfSuccessors = BRI.getNumSuccessors();
  if (NumOfSuccessors != 2) {
    return false;
  }

  const auto *SuccessorATerminator = BRI.getSuccessor(0)->getTerminator();
  const auto *SuccessorBTerminator = BRI.getSuccessor(1)->getTerminator();

  if (SuccessorATerminator->getNumSuccessors() != 1 ||
      SuccessorBTerminator->getNumSuccessors() != 1) {
    return false;
  }

  if (SuccessorATerminator->getSuccessor(0) !=
      SuccessorBTerminator->getSuccessor(0)) {
    return false;
  }

  // we just make sure the block in the third level isn't the source block
  if (SuccessorATerminator->getSuccessor(0) == BRI.getParent()) {
    assert(false);
    return false;
  }

  return true;
}

/**
 * @brief get the type of terminator of successor
 * @param BB here the current basic block is actually successor
 */
static SuccessorEndKind ExtractSuccessorEnds(const BasicBlock *BB) {
  const auto *Terminator = BB->getTerminator();

  // it the terminator is branch instruction, decide it's CBR, FT or UBR
  if (const auto *BI = dyn_cast<BranchInst>(Terminator)) {
    if (BI->isConditional())
      return SuccessorEndKind::CBR;
    const auto *Succ = BI->getSuccessor(0);
    return BB->getNextNode() == Succ ? SuccessorEndKind::FT
                                     : SuccessorEndKind::UBR;
  }

  // switch
  if (isa<SwitchInst>(Terminator))
    return SuccessorEndKind::SWITCH;

  // invoke
  if (isa<InvokeInst>(Terminator))
    return SuccessorEndKind::IVK;

  // return
  if (isa<ReturnInst>(Terminator))
    return SuccessorEndKind::Ret;

  // resume
  if (isa<ResumeInst>(Terminator))
    return SuccessorEndKind::Resume;

  // indirect branch
  if (isa<IndirectBrInst>(Terminator))
    return SuccessorEndKind::IJUMP;

  // call branch
  if (isa<CallBrInst>(Terminator))
    return SuccessorEndKind::IJSR;

  // call return
  if (isa<CatchReturnInst>(Terminator))
    return SuccessorEndKind::UBR;

  // catch switch
  if (const auto *CSWT = dyn_cast<CatchSwitchInst>(Terminator)) {
    return CSWT->unwindsToCaller() ? SuccessorEndKind::Ret
                                   : SuccessorEndKind::UBR;
  }

  // clean up return
  if (const auto *CRI = dyn_cast<CleanupReturnInst>(Terminator)) {
    return CRI->unwindsToCaller() ? SuccessorEndKind::Ret
                                  : SuccessorEndKind::UBR;
  }

  assert(isa<UnreachableInst>(Terminator));
  return SuccessorEndKind::Nothing; // Unreacheable Inst
}

bool GetFunctionStartWithRet(const Function *F) {
  const BasicBlock &BB = F->getEntryBlock();
  const Instruction *Terminator = BB.getTerminator();

  // check direct successors of entry block
  int NumOfSuccessors = Terminator->getNumSuccessors();
  for (int i = 0; i < NumOfSuccessors; ++i) {
    const BasicBlock *Dst = Terminator->getSuccessor(i);
    if (ExtractSuccessorEnds(Dst) == SuccessorEndKind::Ret) {
      return true;
    }
  }

  return false;
}

void getSubLoopFeatures(Loop *L, BrSrcBBFeturesT &SrcBBFeatures) {
  const std::vector<Loop *> &SubLoops = L->getSubLoops();
  SrcBBFeatures.srcTotalSubLoopSize = SubLoops.size();

  unsigned int sum = 0;

  for (auto *SubLoop : SubLoops) {
    sum += SubLoop->getNumBlocks();
  }

  SrcBBFeatures.srcTotalSubLoopBlockSize = sum;
}

void ValidfyLoopExitSize(BrSrcBBFeturesT &SrcBBFeatures,
                         SmallVector<BasicBlock *, 8> &ExitBlocks) {

  DenseMap<BasicBlock *, int> ExitBlocksMap;

  for (auto *EB : ExitBlocks) {
    ExitBlocksMap.insert(std::pair<BasicBlock *, int>(EB, 1));
  }

  /*
  considering this case
  for.cond:                                         ; preds = %for.inc, %entry
    ...
    br i1 %cmp, label %for.body, label %for.end, !prof !34

  for.body:                                         ; preds = %for.cond
    ...
    br i1 %cmp3, label %if.then, label %if.end, !prof !35

  if.then:                                          ; preds = %for.body
    br label %for.end

  for.end:                                          ; preds = %if.then,
  %for.cond
    ...
    ret i32 %18
  In llvm, if.then and for.end are both classified as exit blocks. However,
  if.then to us seems to be a duplicated one which is spawned potentially for
  the purpose of Instrumentation.
  */
  for (auto *EB : ExitBlocks) {
    if (EB->sizeWithoutDebug() == 1) {
      auto *EBTerminator = EB->getTerminator();

      if (EBTerminator->getNumSuccessors() == 1) {
        auto *EBUnqiueDirectSucc = EBTerminator->getSuccessor(0);

        if (ExitBlocksMap.count(EBUnqiueDirectSucc)) {
          SrcBBFeatures.srcLoopExitSize -= 1;
        }
      }
    }
  }
}

bool ExtractSuccessorCall(const BasicBlock *BB) {
  auto HasCall = [](const BasicBlock *BB) {
    for (const auto &Inst : *BB) {
      if (const auto *CI = dyn_cast<CallInst>(&Inst)) {
        const auto *Func = CI->getCalledFunction();
        if (Func)
          return true;
      }
    }
    return false;
  };

  // it has call
  if (HasCall(BB))
    return true;

  // its unconditional successor has call
  const auto *Terminator = BB->getTerminator();
  if (const auto *BI = dyn_cast<BranchInst>(Terminator)) {
    if (!BI->isUnconditional())
      return false;
    return HasCall(BI->getSuccessor(0));
  }

  return false;
}

bool ExtractSuccessorUseDef(TerminatorInst BRI, const BasicBlock *BB) {
  const auto *Condition = BRI.getCondition();
  const auto *Inst = dyn_cast<Instruction>(Condition);

  // no condition in branch
  if (!Inst)
    return false;

  // check whether the successor uses the register in terminator
  for (const auto &operand : Inst->operands()) {
    const auto *InstOp = dyn_cast<Instruction>(&operand);
    if (!InstOp)
      continue;

    for (const auto *user : InstOp->users()) {
      if (const auto *InstUser = dyn_cast<Instruction>(user)) {
        if (InstUser->getParent() == BB)
          return true;
      }
    }
  }
  return false;
}

bool ExtractSuccessorStore(const BasicBlock *BB) {
  // maybe we should evaluate some implicit store instruction
  auto HasStore = [](const BasicBlock *BB) {
    for (const auto &Inst : *BB) {
      if (isa<StoreInst>(&Inst)) {
        return true;
      }
    }
    return false;
  };

  // it has store
  if (HasStore(BB)) {
    return true;
  }

  return false;
}

bool isLoopEntering(const LoopInfo &LI,
                    const BranchProbabilityInfo::SccInfo &Scc,
                    const BasicBlock *Src, const BasicBlock *Dst) {
  int SrcSccNum = !LI.getLoopFor(Src) ? Scc.getSCCNum(Src) : -1;
  int DstSccNum = !LI.getLoopFor(Dst) ? Scc.getSCCNum(Dst) : -1;

  const auto *Loop = LI.getLoopFor(Dst);

  return (Loop && !Loop->contains(LI.getLoopFor(Src))) ||
         (DstSccNum != -1 && SrcSccNum != DstSccNum);
}

void ExtractSuccessorFeatures(
    const BranchProbabilityInfo &BPI, const BranchProbabilityInfo &OldBPI,
    const BasicBlock *Src, const BasicBlock *Dst, const LoopInfo &LI,
    const DominatorTree &DT, const PostDominatorTree &PostDT,
    const BranchProbabilityInfo::SccInfo &Scc,
    SmallPtrSet<const BasicBlock *, 8> &UnlikelyBlocks,
    BrSuccFeaturesT &SuccFeatures) {

  SuccFeatures.SuccessorBranchDominate = DT.dominates(Src, Dst);
  SuccFeatures.SuccessorsBranchPostDominate = PostDT.dominates(Dst, Src);

  // whether the successor is a loop header, it even needs to check whether the
  // unconditional successor of the successor is a loop header
  SuccFeatures.SuccessorLoopHeader = false;

  if (const auto *Loop = LI.getLoopFor(Dst)) {
    if (Dst == Loop->getHeader()) {
      SuccFeatures.SuccessorLoopHeader = true;
    }
  }
  if (!SuccFeatures.SuccessorLoopHeader) {
    if (auto *BI = dyn_cast<BranchInst>(Dst->getTerminator())) {
      if (BI->isUnconditional()) {
        auto *SSB = BI->getSuccessor(0);
        if (const auto *Loop = LI.getLoopFor(SSB)) {
          SuccFeatures.SuccessorLoopHeader = Loop->getHeader() == SSB;
        }
      }
    }
  }

  // whether the successor is an exit or loop back edge
  SuccFeatures.SuccessorExitEdge = SuccessorExitEdgeType::NLE;
  SuccFeatures.SuccesorLoopBack = false;
  if (const auto *Loop = LI.getLoopFor(Src)) {
    SuccFeatures.SuccessorExitEdge = !Loop->contains(Dst)
                                         ? SuccessorExitEdgeType::LE
                                         : SuccessorExitEdgeType::NLE;
    SuccFeatures.SuccesorLoopBack = Loop->isLoopLatch(Src);
    if (SuccFeatures.SuccesorLoopBack) {
      SuccFeatures.SuccesorLoopBack = Loop->getHeader() == Dst;
    }
  }

  /* additional judge on Successor Exit through SCC infoscc
    this is intended to detect some potential loop
    like that caused by jumping between label */
  int SrcSccNum = !LI.getLoopFor(Src) ? Scc.getSCCNum(Src) : -1;
  int DstSccNum = !LI.getLoopFor(Dst) ? Scc.getSCCNum(Dst) : -1;
  if (SrcSccNum != -1 && SrcSccNum != DstSccNum) {
    SuccFeatures.SuccessorExitEdge = SuccessorExitEdgeType::LE;
  }

  SuccFeatures.SuccessorsEnd = ExtractSuccessorEnds(Dst);
  SuccFeatures.SuccessorsCall = ExtractSuccessorCall(Dst);
  SuccFeatures.SuccessorsUseDef =
      ExtractSuccessorUseDef(Src->getTerminator(), Dst);

  // Store instruction in Successor
  SuccFeatures.SuccessorStore = ExtractSuccessorStore(Dst);

  // Unlikely Branch detection
  if (LI.getLoopFor(Src) && UnlikelyBlocks.contains(Dst)) {
    SuccFeatures.SuccessorUnlikely = SuccessorUnlikelyType::Unlikely;
  } else {
    SuccFeatures.SuccessorUnlikely = SuccessorUnlikelyType::Normallikely;
  }

  // Estimated Weight Feature detection

  std::optional<uint32_t> Weight;
  BPI.getLLVMEstimatedWeight(Src, Dst, isLoopEntering(LI, Scc, Src, Dst));

  SuccFeatures.SuccessorEstimatedWeight =
      Weight.value_or(static_cast<uint32_t>(0x3));

  // if unwind 0xffff, transfer it to 0x2 to scale in a small number
  if (SuccFeatures.SuccessorEstimatedWeight ==
      (unsigned int)llvm::BlockExecWeight::COLD) {
    SuccFeatures.SuccessorEstimatedWeight = 0x2;
  }

  SuccFeatures.SuccessorInstructionSize = Dst->sizeWithoutDebug();
}

// mainly refer to calcZeroHeuristics in BranchProbabilityInfo.cpp
void GenConstantFuncFeatureValue(const Instruction *Terminator,
                                 unsigned &FuncIdx) {
  /// to check whether it satisfy the condition of triggering Zero Heuristics in
  /// LLVM
  bool ZeroHeuristicSatisfy = true;

  const BranchInst *BI = dyn_cast<BranchInst>(Terminator);
  if (!BI || !BI->isConditional())
    return;

  Value *Cond = BI->getCondition();
  ICmpInst *CI = dyn_cast<ICmpInst>(Cond);
  if (!CI)
    return;

  auto GetConstantInt = [](Value *V) {
    if (auto *I = dyn_cast<BitCastInst>(V))
      return dyn_cast<ConstantInt>(I->getOperand(0));
    return dyn_cast<ConstantInt>(V);
  };

  if (Instruction *LHS = dyn_cast<Instruction>(CI->getOperand(0))) {
    if (LHS->getOpcode() == Instruction::And) {
      if (ConstantInt *AndRHS = GetConstantInt(LHS->getOperand(1))) {
        if (AndRHS->getValue().isPowerOf2()) {
          ZeroHeuristicSatisfy = false;
        }
      }
    }
  }

  Value *RHS = CI->getOperand(1);
  ConstantInt *CV = GetConstantInt(RHS);
  if (!CV) {
    FuncIdx = ConstantOpFunc;
    return;
  }

  if (CV->isOne() && CI->getPredicate() == CmpInst::ICMP_SLT)
    FuncIdx = ZeroHeuristicSatisfy ? ConstantOneFunc : ConstantOnlyOneFunc;
  else if (CV->isMinusOne())
    FuncIdx =
        ZeroHeuristicSatisfy ? ConstantMinusOneFunc : ConstantOnlyMinusOneFunc;
  else if (CV->isZero())
    FuncIdx = ZeroHeuristicSatisfy ? ConstantZeroOpFunc : ConstantOnlyZeroFunc;
}

std::optional<mlpgo::MLBrFeatureVec> ExtractInstFeatures(
    const Instruction *Terminator, const Function &F, ProcedureType ProcType,
    const LoopInfo &LI, const DominatorTree &DT,
    const PostDominatorTree &PostDT, const BranchProbabilityInfo::SccInfo &Scc,
    std::set<std::pair<const BasicBlock *, const BasicBlock *>> BackEdgesSet,
    mlpgo::Parameters &Parameter, const BranchProbabilityInfo &OldBPI,
    const BranchProbabilityInfo &BPI, bool IsInference) {

  TerminatorInst BRI(Terminator);
  const auto *BB = Terminator->getParent();

  // check whether the branch instruction compare predicate is valid
  const auto *Inst = dyn_cast<Instruction>(BRI.getCondition());
  if (Inst && Inst->getParent() == BB) {
    if (const auto *Cmp = dyn_cast<CmpInst>(Inst)) {
      auto predicate = Cmp->getPredicate();
      if (predicate == CmpInst::BAD_ICMP_PREDICATE ||
          predicate == CmpInst::BAD_FCMP_PREDICATE) {
        LLVM_DEBUG(dbgs() << "Bad cmp terminator in Function: " << F.getName()
                          << " Basic Block: " << BB->getName());
        return {};
      }
    }
  }

  size_t NumOfSuccessors = BRI.getNumSuccessors();

  mlpgo::MLBrFeatureVec FeaturesVec = MLBrFeatureVec(NumOfSuccessors);

  BrSrcBBFeturesT &SrcBBFeatures = FeaturesVec.getSrcBBFeatures();

  SrcBBFeatures.srcLoopHeader = LI.isLoopHeader(BB);
  SrcBBFeatures.srcProcedureType = ProcType;
  SrcBBFeatures.srcLoopDepth = LI.getLoopDepth(BB);
  SrcBBFeatures.srcNumberOfSuccessors = NumOfSuccessors;
  SrcBBFeatures.srcFunctionInstructionSize = F.getInstructionCount();

  SrcBBFeatures.srcFunctionNameHash =
      mapName2NumHelper(BB->getParent()->getName(), "function_name");

  const auto *Cond = BRI.getCondition();
  if (const auto *Cmp = dyn_cast<CmpInst>(Cond))
    SrcBBFeatures.srcBranchPredicate = Cmp->getPredicate();
  else
    SrcBBFeatures.srcBranchPredicate = CmpInst::Predicate::BAD_ICMP_PREDICATE;

  SrcBBFeatures.srcRAFunc = SrcBBFeatures.srcRBFunc = BadOpFunc;
  SrcBBFeatures.srcRAOpCode = SrcBBFeatures.srcRBOpCode = BadOpFunc;
  SrcBBFeatures.srcRAType = SrcBBFeatures.srcRBType = GetTypeDesc(nullptr);

  if (const auto *Inst = dyn_cast<Instruction>(Cond)) {
    SrcBBFeatures.srcBranchOperandOpcode = Inst->getOpcode();
    SrcBBFeatures.srcBranchOperandFunc = GetOperandFunctionAsInt(Inst);

    int NumOfOperands = Inst->getNumOperands();
    // Some instructions, like Invoke, can have multiple operands, and we just
    // pick first two operands.
    NumOfOperands = NumOfOperands > 2 ? 2 : NumOfOperands;
    for (int i = 0; i < NumOfOperands; ++i) {
      const auto *Operand = Inst->getOperand(i);
      unsigned &OpCodeIdx =
          i != 0 ? SrcBBFeatures.srcRBOpCode : SrcBBFeatures.srcRAOpCode;
      unsigned &FuncIdx =
          i != 0 ? SrcBBFeatures.srcRBFunc : SrcBBFeatures.srcRAFunc;
      unsigned &TypeIdx =
          i != 0 ? SrcBBFeatures.srcRBType : SrcBBFeatures.srcRAType;
      if (const auto *InstOperand = dyn_cast<Instruction>(Operand)) {
        OpCodeIdx = InstOperand->getOpcode();
        FuncIdx = GetOperandFunctionAsInt(InstOperand);
      } else {
        OpCodeIdx = BadOpFunc;
        if (const auto *Const = dyn_cast<Constant>(Operand)) {
          FuncIdx = Const->isNullValue() ? ConstantZeroOpFunc : ConstantOpFunc;

          // if it's compared with a immediate number, then do one step
          const ICmpInst *CI = dyn_cast<ICmpInst>(Cond);
          if ((CI) && i == 1) {
            if (!CI->getOperand(0)->getType()->isPointerTy()) {
              GenConstantFuncFeatureValue(BRI.Inst, FuncIdx);
            }
          }
        } else {
          FuncIdx = BadOpFunc;
        }
      }
      TypeIdx = GetTypeDesc(Operand->getType());
    }
  } else {
    SrcBBFeatures.srcBranchOperandOpcode = BadOpFunc;
    SrcBBFeatures.srcBranchOperandFunc = BadOpFunc;
  }

  SrcBBFeatures.srcBranchOperandType = GetTypeDesc(Cond->getType());
  SrcBBFeatures.srcFunctionBlockSize = F.size();
  SrcBBFeatures.srcTriangle = GetTriangle(Terminator);
  SrcBBFeatures.srcDiamond = DiamondHeuristic(Terminator);
  SrcBBFeatures.srcFunctionStartWithRet = GetFunctionStartWithRet(&F);

  auto *L = LI.getLoopFor(BB);
  SmallPtrSet<const BasicBlock *, 8> UnlikelyBlocks;
  if (L) {
    BPI.computeUnlikelySuccessorsWrapper(BB, L, UnlikelyBlocks);

    SrcBBFeatures.srcLoopBlockSize = L->getNumBlocks();

    getSubLoopFeatures(L, SrcBBFeatures);

    SmallVector<BasicBlock *, 8> ExitingBlocks;
    L->getExitingBlocks(ExitingBlocks);
    SrcBBFeatures.srcLoopExitingSize = ExitingBlocks.size();

    assert(SrcBBFeatures.srcLoopExitingSize > 0);

    SmallVector<BasicBlock *, 8> ExitBlocks;
    L->getExitBlocks(ExitBlocks);
    SrcBBFeatures.srcLoopExitSize = ExitBlocks.size();

    ValidfyLoopExitSize(SrcBBFeatures, ExitBlocks);
    assert(SrcBBFeatures.srcLoopExitSize > 0);

    SmallVector<std::pair<BasicBlock *, BasicBlock *>, 4> ExitEdges;
    L->getExitEdges(ExitEdges);
    SrcBBFeatures.srcLoopExitEdgesSize = ExitEdges.size();
  }

  bool HasOneExitBranch =
      false; // detect whether there will be an loop exit among branches
  bool HasUnlikelyBranch = false; // detect whether there will be an unlikely
                                  // branch among all branches
  int countForExit =
      0; // counter to record how many loop exits for these branch instruction
  uint32_t totalWeight =
      0; // counter to sum up all the estimated weight of successor

  for (unsigned Succ = 0; Succ < NumOfSuccessors; ++Succ) {

    BrSuccFeaturesT &SuccFeatures = FeaturesVec.getSuccFeatures(Succ);
    SuccFeatures.SuccessorsRank = Succ;

    const BasicBlock *Dst = BRI.getSuccessor(Succ);
    ExtractSuccessorFeatures(BPI, OldBPI, BB, Dst, LI, DT, PostDT, Scc,
                             UnlikelyBlocks, SuccFeatures);

    SuccFeatures.SuccessorNameHash = mapName2NumHelper(
        Dst->getName(), Succ ? "successor_right_name" : "successor_left_name");

#if MLPGO_FEATURE_DEBUG_LEVEL > 0
    FeaturesVec.getSuccDebugFeatures(Succ).BBName = Dst->getName();
#endif // MLPGO_FEATURE_DEBUG_LEVEL

    // information like branch probability and dominate relationship, conducted
    // by Dominator tree or etc.
    auto BP = BPI.getEdgeProbability(BB, Dst);

    SuccFeatures.SuccessorPGOProb = BP.getNumerator();

    auto EdgeProb = OldBPI.getEdgeProbability(BB, Dst);

    SuccFeatures.SuccessorLLVMHeuristicProb = EdgeProb.getNumerator();

    if (BackEdgesSet.find(std::pair<const BasicBlock *, const BasicBlock *>(
            BB, Dst)) != BackEdgesSet.end()) {
      SuccFeatures.SuccessorBranchDirection = BranchDirection::Backward;
    }

    SuccFeatures.SuccessorNumberOfSiblingExitSuccessors = 0;

    if (static_cast<bool>(SuccFeatures.SuccessorExitEdge)) {
      HasOneExitBranch = true;
      countForExit++;
    }

    if (static_cast<bool>(SuccFeatures.SuccessorUnlikely)) {
      HasUnlikelyBranch = true;
    }

    totalWeight += SuccFeatures.SuccessorEstimatedWeight;
  }

  // when there is unlike branch, set all other non unlikely branch to be
  // sibling unlikely branch
  if (HasUnlikelyBranch) {
    for (unsigned Succ = 0; Succ < NumOfSuccessors; ++Succ) {
      BrSuccFeaturesT &SuccFeatures = FeaturesVec.getSuccFeatures(Succ);
      if (!static_cast<bool>(SuccFeatures.SuccessorUnlikely)) {
        SuccFeatures.SuccessorUnlikely = SuccessorUnlikelyType::SiblingUnlikely;
      }
    }
  }

  // when there is exit branch, set all other non exit branch to be sibling exit
  // branch
  if (HasOneExitBranch) {
    for (unsigned Succ = 0; Succ < NumOfSuccessors; ++Succ) {
      BrSuccFeaturesT &SuccFeatures = FeaturesVec.getSuccFeatures(Succ);
      if (!static_cast<bool>(SuccFeatures.SuccessorExitEdge)) {
        SuccFeatures.SuccessorExitEdge = SuccessorExitEdgeType::SiblingLE;
        SuccFeatures.SuccessorNumberOfSiblingExitSuccessors = countForExit;
      } else {
        SuccFeatures.SuccessorNumberOfSiblingExitSuccessors = countForExit - 1;
      }
    }
  }

  // set total weight
  for (unsigned Succ = 0; Succ < NumOfSuccessors; ++Succ) {
    BrSuccFeaturesT &SuccFeatures = FeaturesVec.getSuccFeatures(Succ);
    SuccFeatures.SuccessorTotalWeight = totalWeight;
  }

#ifndef INTEL_PRODUCT_RELEASE
  bool CheckRes = ValidateFeatureVec(FeaturesVec, Parameter);
  if (!CheckRes && IsInference) {
    return {};
  }
#endif

  return {std::move(FeaturesVec)};
}

static bool ValidateFeatureVecHelper(int *Vec, int VecSize,
                                     bool IsCommonFeature,
                                     mlpgo::Parameters &Parameter) {
  using FeatValueTy = pair<unsigned, pair<unsigned, ArrayRef<unsigned>>>;
  bool ExistUnknown = false;
  auto ValuesMap = IsCommonFeature
                       ? ArrayRef<FeatValueTy>(&*SrcBBFeaturesValuesMap.begin(),
                                               SrcBBFeaturesValuesMap.size())
                       : ArrayRef<FeatValueTy>(&*SuccFeaturesValuesMap.begin(),
                                               SuccFeaturesValuesMap.size());
  for (int I = 0; I < VecSize; ++I) {
    auto FeatureValuesIt =
        std::find_if(ValuesMap.begin(), ValuesMap.end(),
                     [I](auto Pair) { return (int)Pair.first == I; });
    if (FeatureValuesIt == ValuesMap.end())
      continue;
    unsigned V = *(Vec + I);
    auto ValuesPair = (*FeatureValuesIt).second;
    auto Values = ValuesPair.second;
    if ((ValuesPair.first == static_cast<unsigned>(ValidationType::RANGE) &&
         (V > Values[1] || V < Values[0])) ||
        (ValuesPair.first == static_cast<unsigned>(ValidationType::ENUM) &&
         std::find(Values.begin(), Values.end(), (unsigned)V) ==
             Values.end())) {
      if (!ExistUnknown)
        ExistUnknown = true;
      if (Parameter.DumpUnknownFeatures)
        *(Parameter.UnknownFeaturesOS)
            << "Unknow " << (IsCommonFeature ? "Common" : "Successcor")
            << " Feature Warning: <value> " << V << " at <idx> " << I << "\n";
    }
  }
  return !ExistUnknown;
}

bool ValidateFeatureVec(MLBrFeatureVec &FeatureVec,
                        mlpgo::Parameters &Parameter) {
  // check the common feature
  int *CommonFeatureVec =
      reinterpret_cast<int *>(&FeatureVec.getSrcBBFeatures());
  bool Res = ValidateFeatureVecHelper(
      CommonFeatureVec, static_cast<int>(SrcBBFeatures::SrcBBFeaturesSize),
      true, Parameter);

  // check the successor feature
  size_t NumOfSuccessors = FeatureVec.getNumOfSucc();
  for (unsigned Succ = 0; Succ < NumOfSuccessors; ++Succ) {
    int *SuccessorFeatureVec =
        reinterpret_cast<int *>(&FeatureVec.getSuccFeatures(Succ));
    Res &= ValidateFeatureVecHelper(
        SuccessorFeatureVec, static_cast<int>(SuccFeatures::SuccFeaturesSize),
        false, Parameter);
  }
  return Res;
}

void ExtractFeatures(Function &F, mlpgo::Parameters &Parameter,
                     const BranchProbabilityInfo &OldBPI, CallGraph &CG,
                     InstFeaturesMapTy &Inst2FeaturesMap,
                     std::map<const BasicBlock *, uint64_t> &BBCountValueMap) {

  // preparation
  DominatorTree DT{F};
  PostDominatorTree PostDT{F};
  LoopInfo LI{DT};
  BranchProbabilityInfo BPI(F, LI, nullptr, &DT, &PostDT);
  BranchProbabilityInfo::SccInfo Scc = BranchProbabilityInfo::SccInfo(F);
  SmallVector<std::pair<const BasicBlock *, const BasicBlock *>> BackEdges;
  std::set<std::pair<const BasicBlock *, const BasicBlock *>> BackEdgesSet;

  FindFunctionBackedges(F, BackEdges);
  for (auto &BackEdge : BackEdges)
    BackEdgesSet.insert(BackEdge);

  unsigned int EdgesCountInCFG = 0;

  // get type of function
  auto ProcType = GetProcedureType(F, CG);

  CalcEdgesInFunction(F, EdgesCountInCFG);

  for (const auto &BB : F) {
    const auto *Terminator = BB.getTerminator();
    // check if its conditional branch or switch
    if (!TerminatorInst::isSupportedBrInst(Terminator))
      continue;

    // remove branch which has not been executed
    if (Parameter.RemoveNonRun &&
        BBCountValueMap.find(&BB) != BBCountValueMap.end() &&
        BBCountValueMap[&BB] == 0) {
      continue;
    }

    // extract other features like branch direction ...
    std::optional<mlpgo::MLBrFeatureVec> IF =
        ExtractInstFeatures(Terminator, F, ProcType, LI, DT, PostDT, Scc,
                            BackEdgesSet, Parameter, OldBPI, BPI);
    if (IF) {
      IF->getSrcBBFeatures().srcFunctionEdgesSize = EdgesCountInCFG;
      IF->getSrcBBFeatures().srcBBCount = BBCountValueMap[&BB];
      Inst2FeaturesMap.insert({Terminator, IF.value()});
    }
  }
}

int32_t GetTypeDesc(const Type *Ty = nullptr) {
  [[maybe_unused]] auto IsSupportedType = [](Type::TypeID TyID) {
    switch (TyID) {
    case Type::VoidTyID:
    // case Type::LabelTyID:
    case Type::MetadataTyID:
    case Type::TokenTyID:
    case Type::FunctionTyID:
      return false;
    default:
      return true;
    }
  };

  if (!Ty)
    return EmptyTy;

  int32_t Factor;

  auto TyID = Ty->getTypeID();
  switch (TyID) {
  case Type::PointerTyID:
    Factor = 0x1;
    break;
  case Type::ArrayTyID:
    Ty = Ty->getArrayElementType();
    Factor = 0x2;
    break;
  case Type::FixedVectorTyID:
  case Type::ScalableVectorTyID:
    Ty = cast<VectorType>(Ty)->getElementType();
    Factor = TyID == Type::FixedVectorTyID ? 0x3 : 0x4;
    break;
  case Type::LabelTyID:
    return LabelTy; // directly return since typically we don't need to process
                    // Label, currently this is a temporary solution for Invoke
  default:
    assert(IsSupportedType(TyID));
    Factor = 0x0;
  }

  int32_t Idx = 0;
  switch (Ty->getTypeID()) {
  case Type::HalfTyID:
    Idx = HalfTy;
    break;
  case Type::BFloatTyID:
    Idx = BFloatTy;
    break;
  case Type::FloatTyID:
    Idx = FloatTy;
    break;
  case Type::DoubleTyID:
    Idx = DoubleTy;
    break;
  case Type::X86_FP80TyID:
    Idx = X86_FP80Ty;
    break;
  case Type::FP128TyID:
    Idx = FP128Ty;
    break;
  case Type::PPC_FP128TyID:
    Idx = PPC_FP128Ty;
    break;
  case Type::X86_MMXTyID:
    Idx = X86_MMXTy;
    break;
  case Type::X86_AMXTyID:
    Idx = X86_AMXTy;
    break;
  case Type::IntegerTyID: {
    int BitWidth = cast<IntegerType>(Ty)->getBitWidth();
    if (BitWidth > 1) {
      int Offset = 0, Base = 8;
      while (Base < BitWidth && Offset < 5) {
        Base <<= 1;
        ++Offset;
      }
      Idx = Int8Ty + Offset;
    } else
      Idx = Int1Ty;
    break;
  }
  case Type::PointerTyID:
    Idx = PointerTy;
    break;
  default:
    Idx = VariableSizeTy;
  }
  return Factor * PrimTySize + Idx;
}

// Previously it mainly served as a decision tree, since Opcode can have too
// many values, we try to classify them into smaller sets. e.g. Icmp, Fcmp ->
// Icmp, Now with deep learning model, it's not necessary. It is mainly used for
// debugging and loadGlobal value extraction
int32_t GetOperandFunctionAsInt(const Instruction *Inst) {
  if (auto *LI = dyn_cast<LoadInst>(Inst)) {
    const auto *Pointer = LI->getPointerOperand();
    if (const auto *GEP = dyn_cast<GetElementPtrInst>(Pointer))
      Pointer = GEP->getPointerOperand();
    return isa<GlobalVariable>(Pointer) ? (int32_t)LoadGlobalOpFunc
                                        : (int32_t)Instruction::Load;
  }

  switch (Inst->getOpcode()) {
  case Instruction::ICmp:
  case Instruction::FCmp:
    return Instruction::ICmp;
  case Instruction::FSub:
    return Instruction::Sub;
  case Instruction::FAdd:
    return Instruction::Add;
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::FRem:
    return Instruction::URem;
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
    return Instruction::UDiv;
  default:
    break;
  }
  return Inst->getOpcode();
}

static StringRef GetOperandFunction(const Instruction *Inst) {
  if (auto *LI = dyn_cast<LoadInst>(Inst)) {
    const auto *Pointer = LI->getPointerOperand();
    if (const auto *GEP = dyn_cast<GetElementPtrInst>(Pointer))
      Pointer = GEP->getPointerOperand();
    return isa<GlobalVariable>(Pointer) ? "loadglobal" : "load";
  }

  switch (Inst->getOpcode()) {
  case Instruction::ICmp:
  case Instruction::FCmp:
    return "cmp";
  case Instruction::FSub:
    return "sub";
  case Instruction::FAdd:
    return "add";
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::FRem:
    return "rem";
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
    return "div";
  }
  return Inst->getOpcodeName();
}

static inline void PrintType(raw_ostream &OS, const Type *type) {
  type->print(OS, false, true);
}

StringRef GetSuccessorEnds(SuccessorEndKind Kind) {
  switch (Kind) {
  case SuccessorEndKind::FT:
    return "FT";
  case SuccessorEndKind::CBR:
    return "CBR";
  case SuccessorEndKind::SWITCH:
    return "SWITCH";
  case SuccessorEndKind::IVK:
    return "IVK";
  case SuccessorEndKind::UBR:
    return "UBR";
  case SuccessorEndKind::IJUMP:
    return "IJUMP";
  case SuccessorEndKind::IJSR:
    return "IJSR";
  case SuccessorEndKind::Ret:
    return "RETURN";
  case SuccessorEndKind::Resume:
    return "RESUME";
  case SuccessorEndKind::Nothing:
    return "UNREACHABLE";
  }
  llvm_unreachable("Invalid Successor Ends!");
  return "";
}

static void PrintSuccessorFeatures(const BrSuccFeaturesT &SuccFeatures,
                                   raw_ostream &OS) {
  OS << (SuccFeatures.SuccessorBranchDirection ? "Backward" : "Forward") << "|";
  OS << (SuccFeatures.SuccessorLoopHeader ? "LH" : "NLH") << "|";
  OS << (SuccFeatures.SuccesorLoopBack ? "LB" : "NLB") << "|";
  switch (SuccFeatures.SuccessorExitEdge) {
  case SuccessorExitEdgeType::NLE:
    OS << "NLE"
       << "|";
    break;

  case SuccessorExitEdgeType::LE:
    OS << "LE"
       << "|";
    break;

  case SuccessorExitEdgeType::SiblingLE:
    OS << "SiblingLE"
       << "|";
    break;
  }
  OS << (SuccFeatures.SuccessorsCall ? "PC" : "NPC") << "|";
  OS << GetSuccessorEnds((SuccessorEndKind)SuccFeatures.SuccessorsEnd) << "|";
  OS << (SuccFeatures.SuccessorsUseDef ? "UBD" : "NU") << "|";
  OS << (SuccFeatures.SuccessorBranchDominate ? "D" : "ND") << "|";
  OS << (SuccFeatures.SuccessorsBranchPostDominate ? "PD" : "NPD") << "|";

  switch (SuccFeatures.SuccessorUnlikely) {
  case SuccessorUnlikelyType::Normallikely:
    OS << "Normallikely|";
    break;

  case SuccessorUnlikelyType::Unlikely:
    OS << "Unlikely|";
    break;

  case SuccessorUnlikelyType::SiblingUnlikely:
    OS << "SiblingUnlikely|";
    break;

  default:
    OS << "?|";
    break;
  }

  OS << SuccFeatures.SuccessorNumberOfSiblingExitSuccessors << "|";
  OS << SuccFeatures.SuccessorEstimatedWeight << "|";
  OS << SuccFeatures.SuccessorTotalWeight << "|";

  /// add due to 8.16 issue
  OS << SuccFeatures.SuccessorInstructionSize << "|";

  /// add due to 8.24 issue
  OS << (SuccFeatures.SuccessorStore ? "Store" : "NoStore") << "|";
}

static void DumpFeatureVec(
    const Instruction *Terminator, const MLBrFeatureVec &FeatureVec,
    raw_fd_ostream &OS, std::map<const BasicBlock *, uint64_t> &BBCountValueMap,
    std::map<std::pair<const BasicBlock *, const BasicBlock *>, uint64_t>
        EdgeCountValueMap) {
  TerminatorInst BRI(Terminator);
  const auto *Cond = BRI.getCondition();
  if (const auto *Cmp = dyn_cast<CmpInst>(Cond)) {
    auto Predicate = Cmp->getPredicate();
    OS << CmpInst::getPredicateName(Predicate) << "|";
  } else
    OS << "?|";

  if (const auto *Inst = dyn_cast<Instruction>(Cond)) {
    OS << Inst->getOpcodeName() << "|" << GetOperandFunction(Inst) << "|";
  } else {
    OS << "?|?|";
  }
  PrintType(OS, Cond->getType());
  OS << "|";

  const BrSrcBBFeturesT &SrcBBFeatures = FeatureVec.getSrcBBFeatures();

  if (const auto *Inst = dyn_cast<Instruction>(Cond)) {
    int NumOfOperands = Inst->getNumOperands();
    NumOfOperands = NumOfOperands > 2 ? 2 : NumOfOperands;
    for (int i = 0; i < NumOfOperands; ++i) {
      const auto *Operand = Inst->getOperand(i);
      if (const auto *InstOperand = dyn_cast<Instruction>(Operand)) {
        OS << InstOperand->getOpcodeName() << "|"
           << GetOperandFunction(InstOperand) << "|";
      } else {
        OS << "?|";
        if (dyn_cast<Constant>(Operand)) {
          const int &FuncIdx =
              i == 0 ? SrcBBFeatures.srcRAFunc : SrcBBFeatures.srcRBFunc;
          switch (FuncIdx) {
          case ConstantZeroOpFunc:
            OS << "constantZero|";
            break;

          case ConstantOneFunc:
            OS << "constantOne|";
            break;

          case ConstantMinusOneFunc:
            OS << "constantMinusOne|";
            break;

          case ConstantOpFunc:
            OS << "constant|";
            break;

          case ConstantOnlyZeroFunc:
            OS << "constantOnlyZero|";
            break;

          case ConstantOnlyMinusOneFunc:
            OS << "constantOnlyMinusOne|";
            break;

          case ConstantOnlyOneFunc:
            OS << "constantOnlyOne|";
            break;

          default:
            OS << "?|";
            break;
          }
          // OS << (Const->isNullValue() ? "constantZero" : "constant") << "|";
        } else
          OS << "?|";
      }
      PrintType(OS, Operand->getType());
      OS << "|";
    }

    for (int i = NumOfOperands; i < 2; ++i)
      OS << "?|?|?|";
  } else
    OS << "?|?|?|?|?|?|";

  OS << (SrcBBFeatures.srcLoopHeader ? "LH" : "NLH") << "|";
  OS << GetProcedureType((ProcedureType)SrcBBFeatures.srcProcedureType) << "|";
  OS << SrcBBFeatures.srcLoopDepth << "|";
  OS << SrcBBFeatures.srcLoopBlockSize << "|";
  OS << SrcBBFeatures.srcTotalSubLoopSize << "|";
  OS << SrcBBFeatures.srcTotalSubLoopBlockSize << "|";
  OS << SrcBBFeatures.srcLoopExitingSize << "|";
  OS << SrcBBFeatures.srcLoopExitSize << "|";
  OS << SrcBBFeatures.srcLoopExitEdgesSize << "|";
  OS << (SrcBBFeatures.srcTriangle ? "SSD" : "NSD") << "|";
  OS << (DiamondHeuristic(BRI) ? "DD" : "NDD") << "|";
  OS << (SrcBBFeatures.srcFunctionStartWithRet ? "HeadRet" : "NoHeadRet")
     << "|";
  OS << SrcBBFeatures.srcFunctionInstructionSize << "|";
  OS << SrcBBFeatures.srcFunctionBlockSize << "|";
  OS << SrcBBFeatures.srcFunctionEdgesSize << "|";
  OS << BRI.getNumSuccessors() << "|";
  OS << FeatureVec.getSrcBBFeatures().srcBBCount << "|";
  const auto *Func = Terminator->getFunction();
  if (const auto &DebugLoc = Terminator->getDebugLoc()) {
    OS << DebugLoc->getDirectory() << "|" << DebugLoc->getFilename() << ":"
       << DebugLoc->getScope()->getSubprogram()->getName() << ":"
       << DebugLoc.getLine() << ":" << DebugLoc.getCol() << ":"
       << (DebugLoc.isImplicitCode() ? "Implicit" : "explicit");
  } else
    OS << Func->getName();
  OS << "\n";

  // additional Information
  const auto *BB = BRI.getParent();
  const auto *F = BB->getParent();

  OS << BBCountValueMap[BB] << "|";
  OS << Terminator->getOpcodeName() << " | ";
  OS << BB->getName() << " | ";
  for (unsigned int i = 0; i < Terminator->getNumSuccessors(); ++i) {
    OS << Terminator->getSuccessor(i)->getName() << " | ";
  }

  for (size_t Idx = 0; Idx < FeatureVec.getNumOfSucc(); ++Idx) {
    const BrSuccFeaturesT &SuccFeatures = FeatureVec.getSuccFeatures(Idx);
    PrintSuccessorFeatures(SuccFeatures, OS);

    OS << SuccFeatures.SuccessorLLVMHeuristicProb << "|";
    OS << SuccFeatures.SuccessorPGOProb << "|";

    const auto *SuccBB = BRI.getSuccessor(Idx);
    OS << "Function size: " << F->size() << " | "
       << EdgeCountValueMap[std::pair<const BasicBlock *, const BasicBlock *>(
              BB, SuccBB)]
       << "\n";
  }
}

void DumpTrainingSet(
    Function &F, const InstFeaturesMapTy &Inst2FeaturesMap,
    mlpgo::Parameters &Parameter,
    std::map<const BasicBlock *, uint64_t> &BBCountValueMap,
    std::map<std::pair<const BasicBlock *, const BasicBlock *>, uint64_t>
        EdgeCountValueMap) {
  raw_fd_ostream &OS = *Parameter.OS;

  if (Parameter.DumpJSON) {
    json::OStream J(OS, 2);
    J.arrayBegin();
    J.objectBegin();
    for (const auto &BB : F) {
      const auto *Terminator = BB.getTerminator();

      auto res = Inst2FeaturesMap.find(Terminator);
      if (res != Inst2FeaturesMap.end())
        res->second.dumpJSON(J);
    }
    J.objectEnd();
    J.arrayEnd();
    return;
  }

  for (const auto &BB : F) {
    const auto *Terminator = BB.getTerminator();

    auto res = Inst2FeaturesMap.find(Terminator);
    if (res != Inst2FeaturesMap.end()) {

      if (Parameter.DumpFeaturesWithDebugInfo)
        DumpFeatureVec(res->first, res->second, OS, BBCountValueMap,
                       EdgeCountValueMap);
      else
        res->second.dump(OS);
    }
  }
}

} // end namespace mlpgo
} // end namespace llvm
