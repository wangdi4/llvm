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
//

#include "X86.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include <algorithm>
#include <numeric>
#include <queue>

#define DEBUG_TYPE "x86-split-vector-value-type"

using namespace llvm;

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

// Position of shufflevector instructions.
// shufflevector instruction is used to split not supported value or fuse two
// split values to one value. Currently these shufflevector instructions is
// inserted after defination of original instruction or before use of split
// instruction.
// FIXME:ShufflePos should be removed if we make sure we never fuse split
// instruction after definition of original instruction.
enum ShufflePos { SP_AFTER_DEFINE = 0, SP_BEFORE_USE };

class X86SplitVectorValueType : public FunctionPass {
public:
  static char ID;

  X86SplitVectorValueType() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

private:
  // Split an instruction transitively.
  bool splitInstChainBottomUp(Instruction *I, unsigned Depth);

  // Update affected instructions transitively.
  bool updateInstChain();

  // Split I into two new instructions and insert it before I. I won't be
  // erased until all instructions are handled successfully.
  void createSplitInst(Instruction *I);

  // Split Constant into two parts. Currently, only constant is supported.
  void createSplitConstant(Constant *Val);

  // Create shufflevector instructions to split value of instruction that we
  // can't handle.
  void createShufVecInstToSplit(Instruction *I);

  // Create shuffleVector instruction to fuse two new instructions' value into
  // one. I is original instruction which has been split. UI is one of users of
  // I.
  void createShufVecInstToFuse(Instruction *I, Instruction *UI, ShufflePos Pos);

  // Call run method of all Instruction Actions.
  void takeAllInstAction();

  // Erase all instructions in this set from it's parent. Affected instructions
  // must also be in this set.
  void eraseInstSet(const DenseSet<Instruction *> &InstSet);

  // Erase cache based on split status.
  void cleanUpCache(bool SplitSucc);

  // Get TargetTransformationInfo and X86Subtarget.
  bool getTargetInfo(const Function &F) {
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

  // Judge if I need to be split.
  bool isCandidate(const Instruction *I) const;

  // Judge if I is supported to be split by our pass.
  bool isSupportedOp(const Instruction *I) const;

  // Map the original constant to new constant.
  DenseMap<Value *, SmallVector<Constant *, 2>> ConstantMap;

  // Map the original instruction to new instructions.
  // The first two elements of this SmallVector is the new instrctions split
  // from original instruction. The third element is the shufflevector
  // instruction to fuse the values of first two elements. The third element
  // exist only when we need to fuse first two elements into one and insert it
  // after the second element.
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

  // For naming instructions.
  unsigned NameIdx = 0;

  const TargetTransformInfo *TTI = nullptr;
  const X86Subtarget *ST = nullptr;
};
} // end anonymous namespace

#ifndef NDEBUG
// Only for debuging.
static raw_ostream &indentedDbgs(unsigned Depth) {
  return dbgs().indent(Depth * 2);
}
#endif

void X86SplitVectorValueType::setInstName(const Instruction *I,
                                          Instruction *NI0,
                                          Instruction *NI1) const {
  if (I->hasName()) {
    NI0->setName(I->getName().str() + ".l");
    NI1->setName(I->getName().str() + ".h");
  }
}

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
    LLVM_DEBUG(dbgs() << "Erasing Instruction: "; I->dump());
    I->eraseFromParent();
  }
}

bool X86SplitVectorValueType::isSupportedOp(const Instruction *I) const {
  if (I->isBinaryOp())
    return true;

  if (I->getOpcode() == Instruction::Select) {
    // Avoid to split select instruction if it's value won't be split in ISel.
    // (except vector condition)
    if (!I->getType()->getScalarType()->isIntegerTy(1) &&
        TTI->getNumberOfParts(I->getType()) < 2)
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

void X86SplitVectorValueType::createShufVecInstToSplit(Instruction *I) {
  // Skip if I has already been split.
  if (InstMap.count(I))
    return;

  Type *VTy = cast<VectorType>(I->getType());
  unsigned NumElmts = VTy->getVectorNumElements();
  SmallVector<uint32_t, 16> MaskVec(NumElmts / 2);

  std::iota(MaskVec.begin(), MaskVec.end(), 0);
  Instruction *NI0 = new ShuffleVectorInst(
      I, UndefValue::get(VTy),
      ConstantDataVector::get(VTy->getContext(), MaskVec));

  std::iota(MaskVec.begin(), MaskVec.end(), NumElmts / 2);
  Instruction *NI1 = new ShuffleVectorInst(
      I, UndefValue::get(VTy),
      ConstantDataVector::get(VTy->getContext(), MaskVec));

  // Record new instructions.
  NInstSet.insert(NI0);
  NInstSet.insert(NI1);

  // Map original instruction to new instructions.
  InstMap[I].push_back(NI0);
  InstMap[I].push_back(NI1);

  // Insert NI0 after I, NI1 after NI0.
  InsertInstAfter(I, NI0).run();
  InsertInstAfter(NI0, NI1).run();
}

// FIXME: Currently, Pos == SP_AFTER_DEFINE has never been used. Remove Pos if
// we make sure we never fuse split instruction after definition of original
// instruction.
void X86SplitVectorValueType::createShufVecInstToFuse(Instruction *I,
                                                      Instruction *UI,
                                                      ShufflePos Pos) {
  assert(InstMap.count(I) && "I must be split!");

  // Get operands of UI that must be updated if I is replaced by new
  // instructions.
  SmallVector<unsigned, 2> OperandList;
  for (unsigned OpI = 0; OpI < UI->getNumOperands(); OpI++) {
    if (UI->getOperand(OpI) == I)
      OperandList.push_back(OpI);
  }

  Instruction *NI = nullptr;

  if (InstMap[I].size() == 2) {
    VectorType *VTy = cast<VectorType>(I->getType());
    unsigned NumElmts = VTy->getVectorNumElements();
    SmallVector<uint32_t, 32> MaskVec(NumElmts);

    std::iota(MaskVec.begin(), MaskVec.end(), 0);
    NI = new ShuffleVectorInst(
        InstMap[I][0], InstMap[I][1],
        ConstantDataVector::get(VTy->getContext(), MaskVec));
    NI->setName(std::string("fused") + std::to_string(NameIdx++));

    switch (Pos) {
    default:
      llvm_unreachable("Unsupported position");
    case SP_BEFORE_USE:
      InsertInst(UI, NI).run();
      break;
    case SP_AFTER_DEFINE:
      InstMap[I].push_back(NI);
      InsertInstAfter(I, NI).run();
      break;
    }

  } else if (InstMap[I].size() == 3) {
    switch (Pos) {
    default:
      llvm_unreachable("Unsupported position!");
    case SP_BEFORE_USE:
      NI = InstMap[I][2]->clone();
      NI->setName(std::string("fused") + std::to_string(NameIdx++));
      InsertInst(UI, NI).run();
      break;
    case SP_AFTER_DEFINE:
      NI = InstMap[I][2];
      break;
    }

  } else {
    llvm_unreachable("Something bad happend!");
  }

  NInstSet.insert(NI);

  // Operands of UI will be updated after all instructions are handled
  // successfully.
  InstActions.push_back(UpdateInstOperand{UI, NI, std::move(OperandList)});
}

void X86SplitVectorValueType::createSplitConstant(Constant *CV) {
  // Skip Constant which has been split.
  if (ConstantMap.count(CV))
    return;

  VectorType *VTy = cast<VectorType>(CV->getType());
  unsigned NumElmts = VTy->getVectorNumElements();
  SmallVector<Constant *, 32> ElmtsVec;
  for (unsigned I = 0; I < NumElmts; I++)
    ElmtsVec.push_back(CV->getAggregateElement(I));

  auto Elmts0 = makeArrayRef(ElmtsVec).drop_back(NumElmts / 2);
  auto Elmts1 = makeArrayRef(ElmtsVec).drop_front(NumElmts / 2);
  ConstantMap[CV].push_back(ConstantVector::get(Elmts0));
  ConstantMap[CV].push_back(ConstantVector::get(Elmts1));
}

void X86SplitVectorValueType::createSplitInst(Instruction *I) {
  VectorType *VTy = cast<VectorType>(I->getType());
  VectorType *HVTy = VTy->getHalfElementsVectorType(VTy);
  Instruction *NI0 = I->clone();
  Instruction *NI1 = I->clone();
  NI0->mutateType(HVTy);
  NI1->mutateType(HVTy);

  for (unsigned OpI = 0; OpI < I->getNumOperands(); OpI++) {
    Value *OpdVal = I->getOperand(OpI);
    if (Constant *CV = dyn_cast<Constant>(OpdVal)) {
      NI0->setOperand(OpI, ConstantMap[CV][0]);
      NI1->setOperand(OpI, ConstantMap[CV][1]);
    } else if (Instruction *Opd = dyn_cast<Instruction>(OpdVal)) {
      // Temporarily set new PHINode's operands which havn't been split to
      // undef. This step is very important or the new PHINode may cause
      // unnecessary bugs dues to its operand.
      // FIXME: add a bug example.
      if (UnsvdPHIOpdMap.count(std::make_pair(I, Opd))) {
        NI0->setOperand(OpI, UndefValue::get(HVTy));
        NI1->setOperand(OpI, UndefValue::get(HVTy));
        continue;
      }

      NI0->setOperand(OpI, InstMap[Opd][0]);
      NI1->setOperand(OpI, InstMap[Opd][1]);
    } else {
      // We have checked all operands of I is Constant or Instruction before
      // split I.
      llvm_unreachable("Something bad happend. An instruction can be split "
                       "only if all of its operands can be split!");
    }
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
        LLVM_DEBUG(indentedDbgs(1) << "Unknow user type: "; U->getType());
        return false;
      }

      LLVM_DEBUG(indentedDbgs(1) << "User Instruction: "; UI->dump());

      // Prevent an instruction from being split more than once.
      if (SettledNInstSet.count(UI)) {
        LLVM_DEBUG(indentedDbgs(1)
                       << "This instruction has already been splited: ";
                   UI->dump());
        return false;
      }

      if (!isSupportedOp(UI)) {
        createShufVecInstToFuse(I, UI, SP_BEFORE_USE);
        LLVM_DEBUG(indentedDbgs(1)
                   << "Create shufflevector instruction to fuse split "
                      "instructions before this user.\n");
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

        LLVM_DEBUG(indentedDbgs(1) << "Update PHINode: "; UI->dump();
                   indentedDbgs(1); InstMap[UI][0]->dump(); indentedDbgs(1);
                   InstMap[UI][1]->dump());
        UnsvdPHIOpdMap.erase(std::make_pair(UI, I));
        continue;
      }

      // Now we can make sure all UI in InstMap have been split completely.
      if (InstMap.count(UI))
        continue;

      // Split operands of UI.
      for (unsigned OpI = 0; OpI < UI->getNumOperands(); OpI++) {
        Value *OpdVal = UI->getOperand(OpI);
        bool SplitSucc = true;
        if (Instruction *Opd = dyn_cast<Instruction>(OpdVal)) {
          if (Opd == I)
            continue;

          SplitSucc = splitInstChainBottomUp(Opd, 2);

        } else if (Constant *CV = dyn_cast<Constant>(OpdVal)) {
          createSplitConstant(CV);
          LLVM_DEBUG(indentedDbgs(2) << "Create split Constant to replace: ";
                     OpdVal->dump(); indentedDbgs(2);
                     ConstantMap[CV][0]->dump(); indentedDbgs(2);
                     ConstantMap[CV][1]->dump());
        } else {
          SplitSucc = false;
          LLVM_DEBUG(indentedDbgs(2) << "Unsupported value type to split: ";
                     OpdVal->getType()->dump());
        }

        if (!SplitSucc)
          return false;
      }

      // Up to now, all operands have been splited successfully.
      createSplitInst(UI);
      LLVM_DEBUG(indentedDbgs(1) << "Create split instructions to replace: ";
                 UI->dump(); indentedDbgs(1);
                 UI->getPrevNode()->getPrevNode()->dump(); indentedDbgs(1);
                 UI->getPrevNode()->dump());

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
    LLVM_DEBUG(indentedDbgs(1) << "Split left operands of PHI Node: ";
               PI->dump());

    if (!splitInstChainBottomUp(OI, 2))
      return false;

    // Be careful! MI may be invalidated because DenseMap may invalidate
    // iterator when insert occur.
    MI = UnsvdPHIOpdMap.find(std::make_pair(PI, OI));
    assert(MI != UnsvdPHIOpdMap.end() && "Something bad happend!");

    for (unsigned OpI : MI->second) {
      InstMap[PI][0]->setOperand(OpI, InstMap[OI][0]);
      InstMap[PI][1]->setOperand(OpI, InstMap[OI][1]);
    }

    LLVM_DEBUG(indentedDbgs(1) << "Update PHINode: "; PI->dump();
               indentedDbgs(1); InstMap[PI][0]->dump(); indentedDbgs(1);
               InstMap[PI][1]->dump());
    UnsvdPHIOpdMap.erase(MI);
  }

  // Last step may generate new split instructions.
  if (!WorkList.empty())
    return updateInstChain();

  return true;
}

bool X86SplitVectorValueType::splitInstChainBottomUp(Instruction *I,
                                                     unsigned Depth) {
  LLVM_DEBUG(indentedDbgs(Depth) << "Visit Inst: "; I->dump());

  // Prevent stack overflow.
  if (Depth > DFSDepthThreshold) {
    LLVM_DEBUG(dbgs() << "Depth reach threshold. Depth: " << Depth << "\n");
    return false;
  }

  // Prevent an instruction from being split more than once.
  if (SettledNInstSet.count(I)) {
    LLVM_DEBUG(dbgs() << "This instruction has already been splited. ";
               I->dump());
    return false;
  }

  if (!isSupportedOp(I)) {
    createShufVecInstToSplit(I);
    LLVM_DEBUG(indentedDbgs(Depth)
                   << "Create shufflevector instruction to split: \n";
               indentedDbgs(Depth); I->getNextNode()->dump();
               indentedDbgs(Depth); I->getNextNode()->getNextNode()->dump());
    return true;
  }

  // Skip instruction that has already been split.
  if (InstMap.count(I))
    return true;

  // Step1: Recursively split operands until all operands have been split.
  for (unsigned OpI = 0; OpI < I->getNumOperands(); OpI++) {
    Value *OpdVal = I->getOperand(OpI);
    bool SplitSucc = true;

    if (Instruction *Opd = dyn_cast<Instruction>(OpdVal)) {
      // Some of PHINode's operands may rely on this PHINode. This may cause a
      // cycle reliance so we need to presplit the PHINode. Those operands will
      // be updated later.
      if (isa<PHINode>(I)) {
        UnsvdPHIOpdMap[std::make_pair(I, Opd)].push_back(OpI);
        continue;
      }

      SplitSucc = splitInstChainBottomUp(Opd, Depth + 1);

    } else if (Constant *CV = dyn_cast<Constant>(OpdVal)) {
      createSplitConstant(CV);
      LLVM_DEBUG(indentedDbgs(Depth) << "Create split Constant to replace: ";
                 OpdVal->dump(); indentedDbgs(Depth + 1);
                 ConstantMap[CV][0]->dump(); indentedDbgs(Depth + 1);
                 ConstantMap[CV][1]->dump());
    } else {
      SplitSucc = false;
      LLVM_DEBUG(indentedDbgs(Depth) << "Unsupported value type to split: ";
                 OpdVal->getType()->dump());
    }

    if (!SplitSucc)
      return false;
  }

  // Step2: After all operands have been split, split this instruction.
  // Then add it into WorkList.
  createSplitInst(I);
  LLVM_DEBUG(indentedDbgs(Depth) << "Create split instructions to replace: ";
             I->dump(); indentedDbgs(Depth);
             I->getPrevNode()->getPrevNode()->dump(); indentedDbgs(Depth);
             I->getPrevNode()->dump());

  WorkList.push(I);

  return true;
}

// Return true if I meets all of the following conditions.
// a) I is cmp instruction.
// b) The number of elements of I is power of 2.
// c) All operands of I will be split in lowering step.
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

  return true;
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

        // Record split instructions to make sure an instruction can only be
        // split once.
        for (auto &MI : InstMap) {
          SettledNInstSet.insert(MI.second[0]);
          SettledNInstSet.insert(MI.second[1]);
        }

        // Erase old instructions.
        eraseInstSet(OInstSet);

        // Rescan this BB to see if there are any left candidates.
        II = BB.getFirstNonPHI()->getIterator();

        LLVM_DEBUG(dbgs() << "\nIR Dump after split\n"; F.dump());

      } else {
        LLVM_DEBUG(dbgs() << "\nSplit BB failed. Erasing split "
                          << "instructions: \n");

        // Erase new instructions.
        eraseInstSet(NInstSet);

        // Add I to bad candidates.
        BadCandidatesSet.insert(I);

        LLVM_DEBUG(dbgs() << "\n\n");
      }

      // Some cache need to be cleaned based on split status.
      cleanUpCache(SplitSucc);
    }

    Changed |= SplitSucc;
  }
  return Changed;
}

char X86SplitVectorValueType::ID = 0;

INITIALIZE_PASS_BEGIN(X86SplitVectorValueType, "x86-split-vector-value-type",
                      "Split vector value type to prevent k-reg from being "
                      "unpacked and split repeatedly",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(X86SplitVectorValueType, "x86-split-vector-value-type",
                    "Split vector value type to prevent k-reg from being "
                    "unpacked and split repeatedly",
                    false, false)

FunctionPass *llvm::createX86SplitVectorValueTypePass() {
  return new X86SplitVectorValueType();
}
