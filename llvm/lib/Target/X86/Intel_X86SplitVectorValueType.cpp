//====-- Intel_X86SplitVectorValueType.cpp ----------------====
//
//      Copyright (c) 2020 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines pass which split some vector instructions to prevent k-reg
// from being unpacked and split repeatedly so that ISel could generate more
// efficient instructions.
//
// e.g. In this simple case. 'for loop' will be vectorized with width=32.
//
// #define BLOCK_SIZE 32
// int foo() {
//   int result = 0;
// #pragma omp simd simdlen(BLOCK_SIZE)
//   for (int v = 0; v < BLOCK_SIZE; v++) {
//     int m = 512;
//     float x = v + 1.0, y = 0.0;
//     while (m > 0 && x + y < 4.0) {
//       x = x * x + y;
//       y = y * y - x;
//       m--;
//     }
//     result += m;
//   }
//   return result;
// }
//
// Before CodeGen, IR of while loop would probably be:
//
// VPlannedBB:
//   ......
//   %vec.phi39 = phi <32 x i1> [ ...... ], [ %predphi44, %VPlannedBB ]
//   ......
//   %1 = fcmp fast olt <32 x float> ......
//   %2 = and <32 x i1> %vec.phi39, %1
//   ......
//   %8 = icmp sgt <32 x i32> ......
//   %predphi42 = select <32 x i1> %2, <32 x i32> ......
//   %predphi43 = select <32 x i1> %2, <32 x i32> ......
//   %predphi44 = and <32 x i1> %2, %8
//   %9 = select <32 x i1> %vec.phi39, <32 x i32> ......
//   %10 = bitcast <32 x i1> %predphi44 to i32
//   %11 = icmp eq i32 %10, 0
//   br i1 %11, label %VPlannedBB47, label %VPlannedBB
//
// If the target machine supports avx512, <32 x float> would be held in two
// zmm registers but <32 x i1> could be held in one k register. In this case,
// the value of %1 instruction is supported by target machine, but its operands
// are not supported. These operands must be split in lowering step so
// there will be two cmp instructions in machine IR. The result of 2 cmp
// instructions are held in two K registers and they could be unpacked into
// one k register. If there are other instructions which use %1 to do some
// bitwise binary operations like and/or, ISel prefer unpacking these two k
// registers into one k register and using unpacked k register to do this
// bitwise binary operation. If there are some select instructions (or other
// instructions with similar behavior) based on cond %1/%vec.phi39 after these
// bitwise binary operations and values to be selected isn't supported by target
// machine, then the unpacked k register may be split into two parts to do
// select operation. In some cases, k registers may be unpacked and split many
// times which may generate more instructions than when k registers weren't
// unpacked. It is difficult to precisely predict which solution will generate
// less instructions. From experience, unpacking k register has a greater
// probability to generate more instructions.
//
// ISel may generate unpacking k instruction when this kind of instruction
// exist: (Assuming the target machine supports avx512.)
// 1. The value of instruction is a vector condition. e.g. <32 x i1>
// 2. The operands of this instruction are not supported by target machine. e.g.
// <32 x float>, <32 x double>.
// 3. This instruction is used for BitwiseLogicOp or SelectInst.
//
// This kind of instruction could be split into two parts before ISel so that
// ISel won't generate unpack instruction. Currently, This pass would find such
// kind of cmp instructions and split it transitively.
//
// Currently, the strategy of this pass is:
// 1. Find such instruction we describe above.
//
// 2. Split this instruction transitively. That is, split its operands first,
// then split this instruction. The new instructions will be insert before
// original instructions. The original instruction will be erased after all
// instructions are handled successfully by this pass. If there is an
// instruction which isn't supported by this pass, the value of this instruction
// will be split into two parts with shuffleVector instructions.
//
// 3. For those original instructions that will be erased, split its users
// transitively. If there are some users can't be split, then generate a
// shuffleVector instruction to 'unpack' split instructions and this
// shuffleVector instruction will be inserted before the user (also supports
// after definition). the user's operands will be updated after all instructions
// are handled successfully.
//
// 4. If all instructions are handled successfully, then erase the original
// instructions which were split. Then update some original instruction's
// operands if necessary. If not, erase all new instructions.
//
// 5. All instructions can only be split once. If original instruction is split
// into new instructions, then new instructions can't be split even if it could
// be split. This is a heuristic method, if vector condition value is split many
// times, bitwise binary operation (and/or) may generate more instructions than
// we saved from unpack and split k-reg.
// FIXME: Need a cost model.
//
// If the dependence graph of some instructions forms a cycle. (There is a
// strong connected component.) This pass handles it in this way:
// 1. This cycle must contain at least one PHINode.
//
// 2. If a PHINode has to be split, it's operands which are instructions may
// depend on this PHINode. This pass will presplit this PHINode, but not split
// its operands which are instructions. The corresponding operands of new
// PHINodes will be set to undef and the relationship of this PHINode and its
// operands will be recorded.
//
// 3. If some of its operands depend on this PHINode, for this PHINode has been
// presplit, these operands will be split when the pass split affected users.
// The new PHINode could be updated when these operands update it's users.
//
// 4. For those PHINode's operands haven't been split, they must not depend
// on this PHINode. This pass will split those operands transitively and update
// the new PHINode.

#include "X86.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Utils/Local.h"
#include <algorithm>
#include <numeric>
#include <queue>

#define DEBUG_TYPE "x86-split-vector-value-type"

using namespace llvm;
using namespace PatternMatch;

static cl::opt<bool>
    DisableSplitVector("disable-x86-split-vector-value-type", cl::Hidden,
                       cl::init(false),
                       cl::desc("Disable x86-split-vector-value-type pass"));
static cl::opt<unsigned> DFSDepthThreshold(
    "x86-split-vector-value-type-max-dfs-depth", cl::Hidden, cl::init(1024),
    cl::desc("Specify maximum DFS depth to split instruction"));

namespace {
// This class is used to operate instruction conveniently. e.g. Insert A
// instruction before/after B instruction. update some of B's operands to A.
// We could abstract these operations as Instruction Actions and save it. In
// this way, these Instruction Actions could be executed later.
// e.g. We want to update some instructions' operands only after we make sure
// all instructions are handled successfully.
class InstAction {
public:
  template <typename InstActionT>
  InstAction(InstActionT &&Action)
      : Action(std::make_unique<InstActionModel<InstActionT>>(
            std::forward<InstActionT>(Action))) {}

  void run() { Action->run(); }

private:
  // Instruction Action concept. Refer to concept based polymorphism.
  class InstActionConcept {
  public:
    virtual void run() = 0;
    virtual ~InstActionConcept() = default;
  };

  // Instruction Action model
  template <typename InstActionT>
  class InstActionModel : public InstActionConcept {
  public:
    InstActionModel(InstActionT &&Action)
        : Action(std::forward<InstActionT>(Action)) {}

    void run() override { Action.run(); }

  private:
    typename std::remove_reference<InstActionT>::type Action;
  };

  std::unique_ptr<InstActionConcept> Action;
};

class UnaryInstAction {
public:
  UnaryInstAction(Instruction *I) : I(I) {}

protected:
  Instruction *I;
};

class BinaryInstAction : public UnaryInstAction {
public:
  BinaryInstAction(Instruction *I, Instruction *NI)
      : UnaryInstAction(I), NI(NI) {}

protected:
  Instruction *NI;
};

// Insert NI before I.
struct InsertInst : public BinaryInstAction {
  InsertInst(Instruction *I, Instruction *NI) : BinaryInstAction(I, NI) {}
  void run() { I->getParent()->getInstList().insert(I->getIterator(), NI); }
};

// Insert NI after I.
struct InsertInstAfter : public BinaryInstAction {
  InsertInstAfter(Instruction *I, Instruction *NI) : BinaryInstAction(I, NI) {}
  void run() {
    I->getParent()->getInstList().insertAfter(I->getIterator(), NI);
  }
};

// Update I's operands to NI according to OperandList.
class UpdateInstOperand : public BinaryInstAction {
public:
  UpdateInstOperand(Instruction *I, Instruction *NI,
                    SmallVector<unsigned, 2> &&OperandList)
      : BinaryInstAction(I, NI), OperandList(std::move(OperandList)) {}

  UpdateInstOperand(const UpdateInstOperand &&UIO)
      : BinaryInstAction(std::move(UIO)),
        OperandList(std::move(UIO.OperandList)) {}

  UpdateInstOperand(const UpdateInstOperand &UIO)
      : BinaryInstAction(UIO), OperandList(UIO.OperandList) {}

  void run() {
    for (unsigned OpI : OperandList)
      I->setOperand(OpI, NI);
  }

private:
  SmallVector<unsigned, 2> OperandList;
};

class X86SplitVectorValueType : public FunctionPass {
public:
  static char ID;

  X86SplitVectorValueType() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

private:
  // Split an instruction transitively.
  bool splitInstChainBottomUp(Instruction *I, unsigned Depth);

  // Split a value/instruction transitively.
  bool splitValueChainBottomUp(Value *Val, unsigned Depth);

  // Update affected instructions transitively.
  bool updateInstChain();

  // Split Constant into two parts.
  bool createSplitConstant(Constant *Val, unsigned Depth);

  // Split I to new instruction and insert it before I. I won't be
  // erased until all instructions are handled successfully.
  void createSplitInst(Instruction *I, unsigned Depth);

  // Split I to two new instructions.
  void createSplitNormalInst(Instruction *I, unsigned Depth);

  // Split ShuffleVectorInst to new instruction.
  void createSplitShuffleVectorInst(ShuffleVectorInst *I, unsigned Depth);

  // Split InsertElementInst to new instruction.
  void createSplitInsertElementInst(InsertElementInst *I, unsigned Depth);

  // Split SelectInst to new instruction.
  void createSplitSelectInst(SelectInst *I, unsigned Depth);

  // Create shufflevector instructions to split value of instruction that we
  // can't handle.
  void createShufVecInstToSplit(Instruction *I, unsigned Depth);

  // Create shuffleVector instruction to fuse two new instructions' value into
  // one. I is original instruction which has been split. UI is one of users of
  // I.
  void createShufVecInstToFuse(Instruction *I, Instruction *UI, unsigned Depth);

  // Call run method of all Instruction Actions.
  void takeAllInstAction();

  // Erase all instructions in this set from it's parent. Affected instructions
  // must also be in this set.
  void eraseInstSet(const DenseSet<Instruction *> &InstSet);

  // Erase cache based on split status.
  void cleanUpCache(bool SplitSucc);

  // Get TargetTransformationInfo and X86Subtarget.
  bool getTargetInfo(const Function &F) {
    TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
    if (!TPC)
      return false;

    auto &TM = TPC->getTM<X86TargetMachine>();
    ST = TM.getSubtargetImpl(F);
    return true;
  }

  // Set name of NI0 and NI1 to [name of I].l|h if name of I exist.
  void setInstName(const Instruction *I, Instruction *NI0,
                   Instruction *NI1) const;

  // Set name of NI to [name of I].l|h according to Idx.
  void setInstName(const Instruction *I, Instruction *NI, unsigned Idx) const;

  // Set operand of split instruction NI. The operand of NI has been split in to
  // two parts. Idx indicates which part we should get.
  void setOperandOfSplitInst(Instruction *NI, Instruction *I, unsigned OpI,
                             unsigned Idx);

  // Return true if I need to be split.
  bool isCandidate(const Instruction *I) const;

  // Return true if I is supported to be split by our pass.
  bool isSupportedOp(const Instruction *I) const;

  // Map the original constant to new constant.
  DenseMap<Value *, SmallVector<Constant *, 2>> ConstantMap;

  // Map the original instruction to new instructions. The first two elements of
  // this SmallVector is the new instrctions split from original instruction.
  DenseMap<Instruction *, SmallVector<Instruction *, 2>> InstMap;

  // Instruction actions that need to be run sequentially after all instructions
  // are handled successfully.
  SmallVector<InstAction, 16> InstActions;

  // Mark old instructions to be erased.
  DenseSet<Instruction *> OInstSet;

  // Mark new instructions when split an instruction transitively. those new
  // instructions will be erased if split failed.
  DenseSet<Instruction *> NInstSet;

  // Mark all new instructions that have been "settled". This set is used to
  // make sure all instructions will be split only once.
  DenseSet<Instruction *> SettledNInstSet;

  // Map [pair<'PHINode', 'PHINode operand which is instruction'>] to ['This
  // operand indexes' ]. This is useful when PHINode have operands which is an
  // instruction. In this scenario the dependency graph may be a cycle so we
  // need to presplit PHINode first. Then update PHINode transitively (May
  // update some of PHINode operands). Finally split the left PHINode operands
  // that haven't been split.
  DenseMap<std::pair<Instruction *, Instruction *>, SmallVector<unsigned, 1>>
      UnsvdPHIOpdMap;

  // WorkList save instructions which users must be updated because this
  // instruction will be replaced by split instructions.
  std::queue<Instruction *> WorkList;

  const TargetLibraryInfo *TLI = nullptr;
  const TargetTransformInfo *TTI = nullptr;
  const X86Subtarget *ST = nullptr;
};

class SimpleInstCombinerWorkList;

// This class do some optimization for split instructions.
class SimpleInstCombiner {
public:
  using BuilderTy = IRBuilder<ConstantFolder, IRBuilderCallbackInserter>;

  SimpleInstCombiner(SimpleInstCombinerWorkList &SICWorkList,
                     BuilderTy &Builder, const TargetLibraryInfo *TLI,
                     const TargetTransformInfo *TTI)
      : SICWorkList(SICWorkList), Builder(Builder), TLI(TLI), TTI(TTI) {}

  // This method will visit all instruction in SICWorkList and remove dead
  // instructions.
  void run();

private:
  // Try to do Instcombine for I. Return the new instruction if I is modified.
  // Otherwise return nullptr;
  Instruction *visit(Instruction *I);

  Instruction *visitShuffleVectorInst(ShuffleVectorInst *SV);
  Instruction *visitExtractElementInst(ExtractElementInst *I);

  // This method is used to replace all uses of I to V and add all users of I to
  // SICWorkList. It is very useful when we find I is dead.
  Instruction *replaceInstUsesWith(Instruction *I, Value *V);

  // Return true is there are some dead instructions is removed.
  // Dead instructions will also be removed from SICWorkList.
  bool recursivelyEraseDeadInst(Instruction *I);

  SimpleInstCombinerWorkList &SICWorkList;
  BuilderTy &Builder;
  const TargetLibraryInfo *TLI;
  const TargetTransformInfo *TTI;
};

// A worklist for SimpleInstCombiner.
class SimpleInstCombinerWorkList {
public:
  // Add unique instruction to worklist.
  void push(Instruction *I) {
    if (WorkListSet.insert(I).second)
      WorkList.push(I);
  }

  // Add users of I to worklist.
  void pushUsers(Instruction *I) {
    for (auto *U : I->users()) {
      if (Instruction *UI = dyn_cast<Instruction>(U))
        push(UI);
    }
  }

  void erase(Instruction *I) { WorkListSet.erase(I); }

  // Pop an instruction from worklist. Return nullptr if worklist is empty.
  Instruction *getNextEntry() {
    Instruction *I = nullptr;
    while (!WorkList.empty()) {
      Instruction *CI = WorkList.front();
      WorkList.pop();
      if (WorkListSet.erase(CI)) {
        I = CI;
        break;
      }
    }
    return I;
  }

private:
  std::queue<Instruction *> WorkList;
  DenseSet<Instruction *> WorkListSet;
};
} // end anonymous namespace

#ifndef NDEBUG
// Only for debuging.
static raw_ostream &indentedDbgs(unsigned Depth) {
  return dbgs().indent(Depth * 2);
}
#endif

void X86SplitVectorValueType::cleanUpCache(bool SplitSucc) {
  if (SplitSucc) {
    assert(WorkList.empty() && "WorkList must be empty!");
    assert(UnsvdPHIOpdMap.empty() && "UnsvdPHIOpdMap must be empty!");

    // InstMap contains instructions which value is split with shuffervector.
    // This shuffervector Instruction should be kept if SplitSucc. It may be
    // reused again.
    for (auto *II : OInstSet)
      InstMap.erase(II);

  } else {
    InstMap.clear();
    UnsvdPHIOpdMap.clear();
    while (!WorkList.empty())
      WorkList.pop();
  }

  ConstantMap.clear();
  InstActions.clear();
  NInstSet.clear();
  OInstSet.clear();
}

void X86SplitVectorValueType::takeAllInstAction() {
  for (auto &IA : InstActions)
    IA.run();
}

void X86SplitVectorValueType::eraseInstSet(
    const DenseSet<Instruction *> &InstSet) {
  for (auto *I : InstSet) {
    for (auto *U : I->users()) {
      auto UI = cast<Instruction>(U);
      assert(InstSet.count(UI) && "User must be also in InstSet!");
      (void)UI;
    }
    I->replaceAllUsesWith(UndefValue::get(I->getType()));
    LLVM_DEBUG(dbgs() << "Erasing Instruction: " << *I << "\n");
    I->eraseFromParent();
  }
}

bool X86SplitVectorValueType::isSupportedOp(const Instruction *I) const {
  if (I->isBinaryOp())
    return true;

  if (isa<SelectInst>(I)) {
    // Avoid to split select instruction if it's value won't be split in ISel.
    // (except vector condition)
    if (!I->getType()->getScalarType()->isIntegerTy(1) &&
        TTI->getNumberOfParts(I->getType()) < 2)
      return false;

    return true;
  }

  // Split special ShuffleVectorInst to expose more opportunities for
  // SimpleInstCombiner.
  // Pattern: shufflevector(Op0, undef, M). M is Constant and it could be
  // splatted and this shufflevector won't change length.
  if (const ShuffleVectorInst *SV = dyn_cast<ShuffleVectorInst>(I)) {
    Value *Op0;
    ArrayRef<int> M;
    if (!match(SV, m_ShuffleVector(m_Value(Op0), m_Undef(), m_Mask(M))))
      return false;

    if (SV->changesLength())
      return false;

    if (!is_splat(M))
      return false;

    unsigned NumElmts = Op0->getType()->getVectorNumElements();
    int64_t Index = M[0];

    if (Index < 0 || Index >= NumElmts)
      return false;

    return true;
  }

  // InsertElementInst with constant index could be split.
  if (isa<InsertElementInst>(I)) {
    ConstantInt *Op2 = dyn_cast<ConstantInt>(I->getOperand(2));
    unsigned NumElmts = I->getType()->getVectorNumElements();
    int64_t Index = Op2->getSExtValue();

    if (Index < 0 || Index >= NumElmts)
      return false;

    return true;
  }

  switch (I->getOpcode()) {
  default:
    return false;

  // Trunc and Ext shouldn't be supported for it's value or operands may not
  // be split in ISel.
  case Instruction::FCmp:
  case Instruction::ICmp:
  case Instruction::PHI:
    return true;
  }
}

void X86SplitVectorValueType::createShufVecInstToSplit(Instruction *I,
                                                       unsigned Depth) {
  // Skip if I has already been split.
  if (InstMap.count(I))
    return;

  VectorType *VTy = cast<VectorType>(I->getType());
  unsigned NumElmts = VTy->getNumElements();
  SmallVector<uint32_t, 16> MaskVec(NumElmts / 2);

  std::iota(MaskVec.begin(), MaskVec.end(), 0);
  Instruction *NI0 = new ShuffleVectorInst(
      I, UndefValue::get(VTy),
      ConstantDataVector::get(VTy->getContext(), MaskVec));

  std::iota(MaskVec.begin(), MaskVec.end(), NumElmts / 2);
  Instruction *NI1 = new ShuffleVectorInst(
      I, UndefValue::get(VTy),
      ConstantDataVector::get(VTy->getContext(), MaskVec));

  setInstName(I, NI0, NI1);

  // Record new instructions.
  NInstSet.insert(NI0);
  NInstSet.insert(NI1);

  // Map original instruction to new instructions.
  InstMap[I].push_back(NI0);
  InstMap[I].push_back(NI1);

  // Insert NI0 after I, NI1 after NI0.
  InsertInstAfter(I, NI0).run();
  InsertInstAfter(NI0, NI1).run();

  LLVM_DEBUG(
      indentedDbgs(Depth) << "Create shufflevector instruction to split: \n";
      indentedDbgs(Depth) << *NI0 << "\n"; indentedDbgs(Depth) << *NI1 << "\n");
}

void X86SplitVectorValueType::createShufVecInstToFuse(Instruction *I,
                                                      Instruction *UI,
                                                      unsigned Depth) {
  assert(InstMap.count(I) && "I must be split!");

  // Get operands of UI that must be updated if I is replaced by new
  // instructions.
  SmallVector<unsigned, 2> OperandList;
  for (unsigned OpI = 0; OpI < UI->getNumOperands(); OpI++) {
    Instruction *Opd = dyn_cast<Instruction>(UI->getOperand(OpI));
    if (Opd == I)
      OperandList.push_back(OpI);
  }

  Instruction *NI = nullptr;
  VectorType *VTy = cast<VectorType>(I->getType());
  unsigned NumElmts = VTy->getNumElements();
  SmallVector<uint32_t, 32> MaskVec(NumElmts);
  std::iota(MaskVec.begin(), MaskVec.end(), 0);
  NI = new ShuffleVectorInst(
      InstMap[I][0], InstMap[I][1],
      ConstantDataVector::get(VTy->getContext(), MaskVec));
  NI->setName("fused");
  InsertInst(UI, NI).run();

  // Record new instruction.
  NInstSet.insert(NI);

  // Operands of UI will be updated after all instructions are handled
  // successfully.
  InstActions.push_back(UpdateInstOperand{UI, NI, std::move(OperandList)});

  LLVM_DEBUG(indentedDbgs(Depth)
                 << "Create shufflevector instruction to fuse split "
                    "instructions.\n";
             indentedDbgs(Depth) << *NI << "\n");
}

bool X86SplitVectorValueType::createSplitConstant(Constant *C, unsigned Depth) {
  if (!isa<ConstantVector>(C) && !isa<ConstantData>(C)) {
    LLVM_DEBUG(indentedDbgs(Depth)
               << "Unsupported Constant to split: " << *C << "\n");
    return false;
  }

  // Skip Constant which has been split.
  if (ConstantMap.count(C))
    return true;

  VectorType *VTy = cast<VectorType>(C->getType());
  unsigned NumElmts = VTy->getNumElements();
  SmallVector<Constant *, 32> ElmtsVec;
  for (unsigned I = 0; I < NumElmts; I++)
    ElmtsVec.push_back(C->getAggregateElement(I));

  auto Elmts0 = makeArrayRef(ElmtsVec).drop_back(NumElmts / 2);
  auto Elmts1 = makeArrayRef(ElmtsVec).drop_front(NumElmts / 2);
  ConstantMap[C].push_back(ConstantVector::get(Elmts0));
  ConstantMap[C].push_back(ConstantVector::get(Elmts1));

  LLVM_DEBUG(indentedDbgs(Depth)
                 << "Create split Constant to replace: " << *C << "\n";
             indentedDbgs(Depth + 1) << *ConstantMap[C][0] << "\n";
             indentedDbgs(Depth + 1) << *ConstantMap[C][1] << "\n");
  return true;
}

void X86SplitVectorValueType::setInstName(const Instruction *I,
                                          Instruction *NI0,
                                          Instruction *NI1) const {
  if (!I->hasName())
    return;

  NI0->setName(I->getName().str() + ".l");
  NI1->setName(I->getName().str() + ".h");
}

void X86SplitVectorValueType::setInstName(const Instruction *I, Instruction *NI,
                                          unsigned Idx) const {
  if (!I->hasName())
    return;

  auto *Postfix = Idx == 0 ? ".l" : ".h";
  NI->setName(I->getName().str() + Postfix);
}

void X86SplitVectorValueType::setOperandOfSplitInst(Instruction *I,
                                                    Instruction *NI,
                                                    unsigned OpI,
                                                    unsigned Idx) {
  assert(!(Idx >> 1) && "Idx must equal to 0|1!");
  Value *OpdVal = I->getOperand(OpI);
  if (Constant *C = dyn_cast<Constant>(OpdVal)) {
    NI->setOperand(OpI, ConstantMap[C][Idx]);
  } else if (Instruction *Opd = dyn_cast<Instruction>(OpdVal)) {
    // Temporarily set new PHINode's operands which havn't been split into
    // undef. This step is very important or the new PHINode may cause
    // unnecessary bugs dues to its operand.
    // FIXME: add a bug example.
    if (UnsvdPHIOpdMap.count(std::make_pair(I, Opd))) {
      VectorType *VTy = cast<VectorType>(I->getType());
      VectorType *HVTy = VTy->getHalfElementsVectorType(VTy);
      NI->setOperand(OpI, UndefValue::get(HVTy));
      return;
    }

    NI->setOperand(OpI, InstMap[Opd][Idx]);
  } else {
    // We have checked all operands of I is Constant or Instruction before
    // split I.
    llvm_unreachable("Something bad happend. An instruction can be split "
                     "only if all of its operands can be split!");
  }
}

void X86SplitVectorValueType::createSplitInsertElementInst(InsertElementInst *I,
                                                           unsigned Depth) {
  VectorType *VTy = cast<VectorType>(I->getType());
  VectorType *HVTy = VTy->getHalfElementsVectorType(VTy);
  ConstantInt *Op2 = cast<ConstantInt>(I->getOperand(2));

  // We have checked index of insertelement point to a valid element.
  unsigned IEIndex = Op2->getZExtValue();
  unsigned NumElmts = VTy->getNumElements();

  // Idx indicates which part of split operands we should get.
  unsigned Idx = IEIndex * 2 / NumElmts;
  unsigned NewIEIndex = IEIndex % (NumElmts / 2);
  Instruction *NI, *NI0, *NI1;

  NI = I->clone();
  NI->mutateType(HVTy);
  setOperandOfSplitInst(I, NI, 0, Idx);
  NI->setOperand(1, I->getOperand(1));
  NI->setOperand(2, ConstantInt::get(Op2->getType(), NewIEIndex));
  setInstName(I, NI, Idx);

  // The first operand of original insertelement will be split into two parts.
  // insertelement will only modify one part (Indexed by Idx). We just need
  // create an insertelement to modify this part. The other part remains intact.
  // This operand may be a Constant. InstMap will map an instruction into two
  // instruction, so we need to create a "bridge" instruction to connect this
  // part to the user of this part. SimpleInstCombiner will remove this
  // "bridge".
  SmallVector<unsigned, 16> MaskVec(NumElmts / 2);
  std::iota(MaskVec.begin(), MaskVec.end(), 0);
  NI1 = new ShuffleVectorInst(
      UndefValue::get(HVTy), UndefValue::get(HVTy),
      ConstantDataVector::get(VTy->getContext(), MaskVec));
  NI1->setName("bridge");
  setOperandOfSplitInst(I, NI1, 0, 1 - Idx);

  NI0 = Idx == 0 ? NI : NI1;
  NI1 = Idx == 0 ? NI1 : NI;

  // Insert NI1 before I. Insert NI0 before NI1.
  InsertInst(I, NI1).run();
  InsertInst(NI1, NI0).run();

  InstMap[I].push_back(NI0);
  InstMap[I].push_back(NI1);

  // Mark new instructions.
  NInstSet.insert(NI0);
  NInstSet.insert(NI1);

  // Mark I to be erased.
  OInstSet.insert(I);

  LLVM_DEBUG(indentedDbgs(Depth)
                 << "Create split instructions to replace: " << *I << "\n";
             indentedDbgs(Depth) << *NI0 << "\n";
             indentedDbgs(Depth) << *NI1 << "\n");
}

void X86SplitVectorValueType::createSplitShuffleVectorInst(ShuffleVectorInst *I,
                                                           unsigned Depth) {
  VectorType *VTy = cast<VectorType>(I->getType());
  VectorType *HVTy = VTy->getHalfElementsVectorType(VTy);
  ArrayRef<int> M = I->getShuffleMask();
  assert(is_splat(M));

  unsigned NumElmts = VTy->getNumElements();
  unsigned SVIndex = M[0];
  unsigned Idx = SVIndex * 2 / NumElmts;
  unsigned NewSVIndex = SVIndex % (NumElmts / 2);

  Instruction *NI = I->clone();
  NI->mutateType(HVTy);
  setOperandOfSplitInst(I, NI, 0, Idx);
  setOperandOfSplitInst(I, NI, 1, Idx);
  SmallVector<int, 16> NewMask(NumElmts / 2, static_cast<int>(NewSVIndex));
  cast<ShuffleVectorInst>(NI)->setShuffleMask(NewMask);
  setInstName(I, NI, Idx);

  // Insert NI before I.
  InsertInst(I, NI).run();

  // Two new shufflevector is same.
  InstMap[I].push_back(NI);
  InstMap[I].push_back(NI);

  // Mark new instructions.
  NInstSet.insert(NI);

  // Mark I to be erased.
  OInstSet.insert(I);

  LLVM_DEBUG(indentedDbgs(Depth)
                 << "Create split instructions to replace: " << *I << "\n";
             indentedDbgs(Depth) << *NI << "\n");
}

void X86SplitVectorValueType::createSplitSelectInst(SelectInst *I,
                                                    unsigned Depth) {
  if (isa<VectorType>(I->getCondition()->getType())) {
    createSplitNormalInst(I, Depth);
    return;
  }

  VectorType *VTy = cast<VectorType>(I->getType());
  VectorType *HVTy = VTy->getHalfElementsVectorType(VTy);
  Instruction *NI0 = I->clone();
  Instruction *NI1 = I->clone();
  NI0->mutateType(HVTy);
  NI1->mutateType(HVTy);

  for (unsigned OpI = 1; OpI < I->getNumOperands(); OpI++) {
    setOperandOfSplitInst(I, NI0, OpI, 0);
    setOperandOfSplitInst(I, NI1, OpI, 1);
  }

  // Condition of split SelectInst is same as original.
  cast<SelectInst>(NI0)->setCondition(I->getCondition());
  cast<SelectInst>(NI1)->setCondition(I->getCondition());

  setInstName(I, NI0, NI1);

  // Insert NI1 before I. Insert NI0 before NI1.
  InsertInst(I, NI1).run();
  InsertInst(NI1, NI0).run();

  InstMap[I].push_back(NI0);
  InstMap[I].push_back(NI1);

  // Mark new instructions.
  NInstSet.insert(NI0);
  NInstSet.insert(NI1);

  // Mark I to be erased.
  OInstSet.insert(I);

  LLVM_DEBUG(indentedDbgs(Depth)
                 << "Create split instructions to replace: " << *I << "\n";
             indentedDbgs(Depth) << *NI0 << "\n";
             indentedDbgs(Depth) << *NI1 << "\n");
}

void X86SplitVectorValueType::createSplitNormalInst(Instruction *I,
                                                    unsigned Depth) {
  VectorType *VTy = cast<VectorType>(I->getType());
  VectorType *HVTy = VTy->getHalfElementsVectorType(VTy);
  Instruction *NI0 = I->clone();
  Instruction *NI1 = I->clone();
  NI0->mutateType(HVTy);
  NI1->mutateType(HVTy);

  for (unsigned OpI = 0; OpI < I->getNumOperands(); OpI++) {
    setOperandOfSplitInst(I, NI0, OpI, 0);
    setOperandOfSplitInst(I, NI1, OpI, 1);
  }

  setInstName(I, NI0, NI1);

  // Insert NI1 before I. Insert NI0 before NI1.
  InsertInst(I, NI1).run();
  InsertInst(NI1, NI0).run();

  InstMap[I].push_back(NI0);
  InstMap[I].push_back(NI1);

  // Mark new instructions.
  NInstSet.insert(NI0);
  NInstSet.insert(NI1);

  // Mark I to be erased.
  OInstSet.insert(I);

  LLVM_DEBUG(indentedDbgs(Depth)
                 << "Create split instructions to replace: " << *I << "\n";
             indentedDbgs(Depth) << *NI0 << "\n";
             indentedDbgs(Depth) << *NI1 << "\n");
}

void X86SplitVectorValueType::createSplitInst(Instruction *I, unsigned Depth) {
  switch (I->getOpcode()) {
  default:
    createSplitNormalInst(I, Depth);
    break;
  case Instruction::ShuffleVector:
    createSplitShuffleVectorInst(cast<ShuffleVectorInst>(I), Depth);
    break;
  case Instruction::InsertElement:
    createSplitInsertElementInst(cast<InsertElementInst>(I), Depth);
    break;
  case Instruction::Select:
    createSplitSelectInst(cast<SelectInst>(I), Depth);
  }
}

bool X86SplitVectorValueType::updateInstChain() {
  while (!WorkList.empty()) {
    Instruction *I = WorkList.front();
    WorkList.pop();
    LLVM_DEBUG(dbgs() << "Update usage of Value: "; I->printAsOperand(dbgs());
               dbgs() << "\n");

    // I may be used many times in UI. The update proceduce will update all
    // value I in UI, so it is unnecessary to visit it again.
    DenseSet<const User *> VisitedUserSet;

    for (auto *U : I->users()) {
      // Skip user which has already been visited.
      if (VisitedUserSet.count(U))
        continue;

      VisitedUserSet.insert(U);

      auto UI = dyn_cast<Instruction>(U);
      if (!UI) {
        LLVM_DEBUG(indentedDbgs(1)
                   << "Unknow user type: " << *U->getType() << "\n");
        return false;
      }

      LLVM_DEBUG(indentedDbgs(1) << "User Instruction: " << *UI << "\n");

      assert(!SettledNInstSet.count(I) &&
             "Candidate must be split previously!");

      if (!isSupportedOp(UI)) {
        createShufVecInstToFuse(I, UI, 1);
        continue;
      }

      // User will now try to be split and replaced by new instructions && it
      // is supported.
      //
      // Some PHINodes have been presplit but it's operands haven't been
      // split yet. This step will update some of those operands that cause a
      // cycle reliance. The left operands will be updated later.
      if (isa<PHINode>(UI) && UnsvdPHIOpdMap.count(std::make_pair(UI, I))) {
        // UI => PHINode, I => PHINode's operand that cause cycle reliance.
        assert(InstMap.count(UI) && "This PHINode Must has been presplit!");

        for (unsigned OpI : UnsvdPHIOpdMap[std::make_pair(UI, I)]) {
          InstMap[UI][0]->setOperand(OpI, InstMap[I][0]);
          InstMap[UI][1]->setOperand(OpI, InstMap[I][1]);
        }

        LLVM_DEBUG(indentedDbgs(1) << "Update PHINode: " << *UI << "\n";
                   indentedDbgs(1) << *InstMap[UI][0] << "\n";
                   indentedDbgs(1) << *InstMap[UI][1] << "\n");
        UnsvdPHIOpdMap.erase(std::make_pair(UI, I));
        continue;
      }

      // Now we can make sure all UI in InstMap have been split completely.
      if (InstMap.count(UI))
        continue;

      // Split operands of UI.
      for (unsigned OpI = 0; OpI < UI->getNumOperands(); OpI++) {
        // I is the first operand of UI and it has been split.
        if (isa<InsertElementInst>(UI))
          break;

        // If cond of SelectInst isn't VectorType, just skip this operand.
        if (isa<SelectInst>(UI) && OpI == 0 &&
            !isa<VectorType>(UI->getOperand(0)->getType()))
          continue;

        Value *OpdVal = UI->getOperand(OpI);
        if (!splitValueChainBottomUp(OpdVal, 2))
          return false;
      }

      // Up to now, all operands have been split successfully.
      createSplitInst(UI, 1);
      WorkList.push(UI);
    }
  }

  // Finally We need to split PHINode' operands that haven't been split yet.
  // In other word, those operands don't depend on this PHINode.
  // !Note: UnsvdPHIOpdMap may be updated.
  while (!UnsvdPHIOpdMap.empty()) {
    auto MI = UnsvdPHIOpdMap.begin();
    Instruction *PI = MI->first.first;  // PHINode
    Instruction *OI = MI->first.second; // Operand of PHINode
    assert(isa<PHINode>(PI) && "PI must be a PHINode!");
    assert(InstMap.count(PI) && "This PHINode Must have been presplit!");
    LLVM_DEBUG(indentedDbgs(1)
               << "Split left operands of PHI Node: " << *PI << "\n");

    if (!splitInstChainBottomUp(OI, 2))
      return false;

    // Be careful! MI may be invalidated because DenseMap may invalidate
    // iterator when insert element.
    MI = UnsvdPHIOpdMap.find(std::make_pair(PI, OI));
    assert(MI != UnsvdPHIOpdMap.end() && "Something bad happend!");

    for (unsigned OpI : MI->second) {
      InstMap[PI][0]->setOperand(OpI, InstMap[OI][0]);
      InstMap[PI][1]->setOperand(OpI, InstMap[OI][1]);
    }

    LLVM_DEBUG(indentedDbgs(1) << "Update PHINode: " << *PI << "\n";
               indentedDbgs(1) << *InstMap[PI][0] << "\n";
               indentedDbgs(1) << *InstMap[PI][1] << "\n");
    UnsvdPHIOpdMap.erase(MI);
  }

  // Last step may generate new split instructions.
  if (!WorkList.empty())
    return updateInstChain();

  return true;
}

bool X86SplitVectorValueType::splitInstChainBottomUp(Instruction *I,
                                                     unsigned Depth) {
  LLVM_DEBUG(indentedDbgs(Depth) << "Visiting Inst: " << *I << "\n");

  // Prevent stack overflow.
  if (Depth > DFSDepthThreshold) {
    LLVM_DEBUG(dbgs() << "Depth reach threshold. Depth: " << Depth << "\n");
    return false;
  }

  // If candidate A is split, supppose all of split instructions form a set CA.
  // If candidate B is split, supppose all of split instructions form a set CB.
  // if split B is not in CA, then CA intersect B equals null set.
  assert(!SettledNInstSet.count(I) && "Candidate must be split previously!");

  if (!isSupportedOp(I)) {
    createShufVecInstToSplit(I, Depth);
    return true;
  }

  // Skip instruction that has already been split.
  if (InstMap.count(I))
    return true;

  // Step1: Recursively split operands until all operands have been split.
  for (unsigned OpI = 0; OpI < I->getNumOperands(); OpI++) {

    // Only the first operand need to be split for InsertElementInst.
    if (isa<InsertElementInst>(I) && OpI > 0)
      break;

    // If cond of SelectInst isn't VectorType, just skip this operand.
    if (isa<SelectInst>(I) && OpI == 0 &&
        !isa<VectorType>(I->getOperand(0)->getType()))
      continue;

    Value *OpdVal = I->getOperand(OpI);

    // Some of PHINode's operands may rely on this PHINode. This may cause a
    // cycle reliance so we need to presplit the PHINode. Those operands will
    // be updated later.
    Instruction *Opd = dyn_cast<Instruction>(OpdVal);
    if (isa<PHINode>(I) && Opd) {
      UnsvdPHIOpdMap[std::make_pair(I, Opd)].push_back(OpI);
      continue;
    }

    if (!splitValueChainBottomUp(OpdVal, Depth + 1))
      return false;
  }

  // Step2: After all operands have been split, split this instruction.
  // Then add it into WorkList.
  createSplitInst(I, Depth);
  WorkList.push(I);

  return true;
}

bool X86SplitVectorValueType::splitValueChainBottomUp(Value *Val,
                                                      unsigned Depth) {

  bool SplitSucc = true;

  if (Instruction *Opd = dyn_cast<Instruction>(Val)) {
    SplitSucc = splitInstChainBottomUp(Opd, Depth);
  } else if (Constant *C = dyn_cast<Constant>(Val)) {
    SplitSucc = createSplitConstant(C, Depth);
  } else {
    SplitSucc = false;
    LLVM_DEBUG(indentedDbgs(Depth) << "Unsupported value type to split: "
                                   << *Val->getType() << "\n");
  }

  return SplitSucc;
}

static bool
recursivelyFindBitwiseOp(const Instruction *I,
                         DenseSet<const Instruction *> &VisitedUserSet) {
  if (VisitedUserSet.count(I))
    return false;

  if (I->isBitwiseLogicOp() || isa<SelectInst>(I))
    return true;

  VisitedUserSet.insert(I);

  for (auto *U : I->users()) {
    if (const Instruction *UI = dyn_cast<Instruction>(U)) {
      if (UI->getType() == I->getType() &&
          recursivelyFindBitwiseOp(UI, VisitedUserSet))
        return true;
    }
  }

  return false;
}

// Return true if I meets all of the following conditions.
// a) I is cmp instruction.
// b) The number of elements of I is power of 2.
// c) All operands of I will be split in lowering step.
// d) There is one user (direct or indirect) which is BitwiseLogicOp or
// SelectInst. All of instructions from I to that user must has same type as I.
// FIXME: Unpacking k-reg has better performance in mandelbrot with -O3.
// FIXME: Candidate may be fold to other instruction in ISel. Split in such
// situation is unnecessary and may degrade performance.
bool X86SplitVectorValueType::isCandidate(const Instruction *I) const {
  if (!isa<CmpInst>(I))
    return false;

  Type *Ty = I->getType();
  if (!Ty->isVectorTy() || !isPowerOf2_64(Ty->getVectorNumElements()))
    return false;

  if (TTI->getNumberOfParts(I->getOperand(0)->getType()) < 2)
    return false;

  DenseSet<const Instruction *> VisitedUserSet;

  // Return true if I meet condition d).
  return recursivelyFindBitwiseOp(I, VisitedUserSet);
}

bool X86SplitVectorValueType::runOnFunction(Function &F) {
  if (DisableSplitVector)
    return false;

  if (!getTargetInfo(F))
    return false;

  if (!ST->useAVX512Regs())
    return false;

  bool Changed = false;

  for (auto &BB : F) {
    bool SplitSucc = false;
    DenseSet<const Instruction *> BadCandidatesSet;

    for (auto II = BB.getFirstNonPHI()->getIterator(); II != BB.end(); ++II) {
      Instruction *I = &*II;

      // Find instruction may cause K-reg repeatedly unpacked and split.
      if (!isCandidate(I))
        continue;

      // Prevent an instruction from being split more than once.
      if (SettledNInstSet.count(I)) {
        LLVM_DEBUG(dbgs() << "This instruction has already been split: " << *I
                          << "\n");
        continue;
      }

      // Avoid try to split I if I can't be split compleletely in previous
      // iteration.
      if (BadCandidatesSet.count(I))
        continue;

      // First split the operands of this instruction, then split this
      // instruction. All users of split instructions will be added to
      // WorkList and updateInstChain will split them transitively.
      if (SplitSucc = splitInstChainBottomUp(I, 0))
        SplitSucc = updateInstChain();

      if (SplitSucc) {
        // Update original instructions.
        takeAllInstAction();

        // Record new instructions to make sure an instruction can only be
        // split once.
        SettledNInstSet.insert(NInstSet.begin(), NInstSet.end());

        // Erase old instructions.
        eraseInstSet(OInstSet);

        // Rescan this BB to see if there are any left candidates.
        II = BB.getFirstNonPHI()->getIterator();

        LLVM_DEBUG(dbgs() << "\nIR Dump after split:\n" << F << "\n");

      } else {
        LLVM_DEBUG(dbgs() << "\nSplit BB failed. Erasing split "
                          << "instructions: \n");

        // Erase new instructions.
        eraseInstSet(NInstSet);

        // Add I to bad candidates.
        BadCandidatesSet.insert(I);

        LLVM_DEBUG(dbgs() << "\n");
      }

      // Some cache need to be cleaned based on split status.
      cleanUpCache(SplitSucc);
    }

    Changed |= SplitSucc;
  }

  if (Changed) {
    SimpleInstCombinerWorkList SICWorkList;
    for (Instruction *I : SettledNInstSet) {
      SICWorkList.push(I);
      SICWorkList.pushUsers(I);
    }

    // Instruction created by Builder will be inserted and added to SICWorkList
    // automatically.
    IRBuilder<ConstantFolder, IRBuilderCallbackInserter> Builder(
        F.getContext(), ConstantFolder(),
        IRBuilderCallbackInserter(
            [&SICWorkList](Instruction *I) { SICWorkList.push(I); }));

    // Try to optimize split instructions.
    SimpleInstCombiner(SICWorkList, Builder, TLI, TTI).run();
  }

  // Elements in these two containers are valid in function scope. They need
  // to be cleanup for next function.
  SettledNInstSet.clear();
  InstMap.clear();

  return Changed;
}

Instruction *SimpleInstCombiner::replaceInstUsesWith(Instruction *I, Value *V) {
  assert(I != V && "Do not replace users of I to I!");
  if (I->use_empty())
    return nullptr;

  LLVM_DEBUG(indentedDbgs(1) << "Replacing " << *I << "\n";
             indentedDbgs(1) << "With      " << *V << "\n");

  SICWorkList.pushUsers(I);
  I->replaceAllUsesWith(V);
  return I;
}

bool SimpleInstCombiner::recursivelyEraseDeadInst(Instruction *I) {
  if (!isInstructionTriviallyDead(I, TLI))
    return false;

  SmallSetVector<Instruction *, 16> InstList;
  InstList.insert(I);

  do {
    Instruction *CI = InstList.pop_back_val();
    if (!isInstructionTriviallyDead(CI, TLI))
      continue;

    for (auto &OpdVal : CI->operands()) {
      if (Instruction *Opd = dyn_cast<Instruction>(OpdVal))
        InstList.insert(Opd);
    }

    SICWorkList.erase(CI);
    LLVM_DEBUG(dbgs() << "Erasing DC: " << *CI << "\n");
    CI->eraseFromParent();
  } while (!InstList.empty());

  return true;
}

Instruction *SimpleInstCombiner::visit(Instruction *I) {
  switch (I->getOpcode()) {
  default:
    return nullptr;
  case Instruction::ShuffleVector:
    return visitShuffleVectorInst(cast<ShuffleVectorInst>(I));
  case Instruction::ExtractElement:
    return visitExtractElementInst(cast<ExtractElementInst>(I));
  }
}

// Suppose C and M are Constant and they could be splatted and have same
// number of elements. Suppose LHS won't be split in ISel. This function fold
//     and(shufflevector(cmp(LHS, C), undef, M), B)
// --> and(cmp(shufflevector(LHS, undef, M), C), B)
// so that ISel could generate mask cmp instruction.
static Instruction *
foldSplattedCmpShuffleVector(ShuffleVectorInst *SV,
                             SimpleInstCombiner::BuilderTy &Builder,
                             const TargetTransformInfo *TTI) {
  Instruction *Cmp;
  ArrayRef<int> M;
  if (!match(SV, m_ShuffleVector(m_Instruction(Cmp), m_Undef(), m_Mask(M))))
    return nullptr;

  if (SV->changesLength())
    return nullptr;

  CmpInst::Predicate Pred;
  Value *LHS;
  Constant *C;

  // Check if Cmp is only used by ShuffleVector.
  if (!match(Cmp, m_OneUse(m_Cmp(Pred, m_Value(LHS), m_Constant(C)))))
    return nullptr;

  // Check if LHS won't be split in ISel.
  if (TTI->getNumberOfParts(LHS->getType()) > 1)
    return nullptr;

  bool HasAnd = false;
  for (auto *U : SV->users()) {
    Instruction *UI = dyn_cast<Instruction>(U);
    if (UI && UI->getOpcode() == Instruction::And) {
      HasAnd = true;
      break;
    }
  }

  Constant *ScalarC = C->getSplatValue(false);

  if (HasAnd && is_splat(M) && ScalarC) {
    Value *NewSV =
        Builder.CreateShuffleVector(LHS, UndefValue::get(LHS->getType()), M);

    bool IsFP = isa<FCmpInst>(Cmp);
    if (IsFP)
      return new FCmpInst(Pred, NewSV, C);
    else
      return new ICmpInst(Pred, NewSV, C);
  }

  return nullptr;
}

// Return true if SV is same as the first operand of SV.
static bool isBridgeShuffleVector(const ShuffleVectorInst *SV) {
  Value *Op0;
  ArrayRef<int> M;
  if (!match(SV, m_ShuffleVector(m_Value(Op0), m_Undef(), m_Mask(M))))
    return false;

  if (SV->changesLength())
    return false;

  // shufflevector(Undef, Undef, M) == Undef
  if (isa<UndefValue>(Op0))
    return true;

  // Check if SV chooses all elements from Op0.
  if (SV->isIdentity() && SV->getMaskValue(0) == 0)
    return true;

  return false;
}

Instruction *SimpleInstCombiner::visitShuffleVectorInst(ShuffleVectorInst *SV) {
  if (Instruction *Res = foldSplattedCmpShuffleVector(SV, Builder, TTI))
    return Res;

  if (isBridgeShuffleVector(SV))
    return replaceInstUsesWith(SV, SV->getOperand(0));

  return nullptr;
}

// Fold extractelement which VectorOp is fused shufflevector.
// e.g.
// %fused = shufflevector <16 x i32> %a, <16 x i32> %b, <32 x i32> <i32 1, ...>
// %tmp = extractelement <32 x i32> %fused, i32 0
// -->
// %fused = shufflevector <16 x i32> %a, <16 x i32> %b, <32 x i32> <i32 1, ...>
// %tmp = extractelement <16 x i32> %a, i32 0
static Instruction *
foldFusedShuffleVectorExtractElement(ExtractElementInst *I) {
  Value *VectorOp;
  ConstantInt *IndexOp;
  if (!match(I, m_ExtractElement(m_Value(VectorOp), m_ConstantInt(IndexOp))))
    return nullptr;

  unsigned NumElmts = VectorOp->getType()->getVectorNumElements();
  int64_t Index = IndexOp->getSExtValue();
  if (Index < 0 || Index >= NumElmts)
    return nullptr;

  Value *SVOp0, *SVOp1;
  ArrayRef<int> M;
  if (!match(VectorOp,
             m_ShuffleVector(m_Value(SVOp0), m_Value(SVOp1), m_Mask(M))))
    return nullptr;

  ShuffleVectorInst *SV = cast<ShuffleVectorInst>(VectorOp);
  if (!SV->isConcat())
    return nullptr;

  // Idx indicates which operand of shufflevector we shoud get.
  unsigned Idx = Index * 2 / NumElmts;
  unsigned NewIndex = Index % (NumElmts / 2);

  Value *NewVectorOp = Idx == 0 ? SVOp0 : SVOp1;
  ConstantInt *NewIndexOp = ConstantInt::get(IndexOp->getType(), NewIndex);

  return ExtractElementInst::Create(NewVectorOp, NewIndexOp,
                                    NewVectorOp->getName() + ".extract." +
                                        Twine(NewIndex) + ".");
}

Instruction *
SimpleInstCombiner::visitExtractElementInst(ExtractElementInst *I) {
  if (Instruction *Res = foldFusedShuffleVectorExtractElement(I))
    return Res;

  return nullptr;
}

void SimpleInstCombiner::run() {
  LLVM_DEBUG(dbgs() << "Simple Instruction Combiner:\n");

  while (Instruction *I = SICWorkList.getNextEntry()) {
    if (recursivelyEraseDeadInst(I))
      continue;

    LLVM_DEBUG(dbgs() << "Visiting: " << *I << "\n");

    // Instruction created by Builder will be inserted befor I.
    Builder.SetInsertPoint(I);
    Builder.SetCurrentDebugLocation(I->getDebugLoc());

    if (Instruction *Res = visit(I)) {
      if (Res != I) {
        LLVM_DEBUG(indentedDbgs(1) << "New Inst: " << *Res << "\n");
        if (I->getDebugLoc())
          Res->setDebugLoc(I->getDebugLoc());

        if (!Res->hasName())
          Res->takeName(I);

        I->replaceAllUsesWith(Res);

        // Insert Res before I.
        InsertInst(I, Res).run();

        // Add Res and its users to SICWorkList.
        SICWorkList.push(Res);
        SICWorkList.pushUsers(Res);
        recursivelyEraseDeadInst(I);
      } else {
        LLVM_DEBUG(indentedDbgs(1) << "Mod Inst: " << *Res << "\n");
        if (!recursivelyEraseDeadInst(I)) {
          SICWorkList.push(I);
          SICWorkList.pushUsers(I);
        }
      }
    }
  } // End of while

  LLVM_DEBUG(dbgs() << "\n--------- End ---------\n\n");
}

char X86SplitVectorValueType::ID = 0;

INITIALIZE_PASS_BEGIN(X86SplitVectorValueType, "x86-split-vector-value-type",
                      "Split vector value type to prevent k-reg from being "
                      "unpacked and split repeatedly",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(X86SplitVectorValueType, "x86-split-vector-value-type",
                    "Split vector value type to prevent k-reg from being "
                    "unpacked and split repeatedly",
                    false, false)

FunctionPass *llvm::createX86SplitVectorValueTypePass() {
  return new X86SplitVectorValueType();
}
