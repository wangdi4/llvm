//===- LoopStridedCodeMotion.cpp - Hoist strided value ----------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LoopStridedCodeMotion.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LoopWIAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/VectorizerUtils.h"

#define DEBUG_TYPE "dpcpp-kernel-loop-strided-code-motion"

using namespace llvm;

namespace {

class LoopStridedCodeMotionImpl {
public:
  LoopStridedCodeMotionImpl(Loop &L, LoopInfo &LI, DominatorTree &DT,
                            LoopWIInfo &WIInfo)
      : L(L), LI(LI), DT(DT), WIInfo(WIInfo), Preheader(L.getLoopPreheader()),
        Header(L.getHeader()), Latch(L.getLoopLatch()) {}

  /// Run on the loop.
  /// \return true if the loop is modified.
  bool run();

private:
  /// The current loop being processed.
  Loop &L;

  LoopInfo &LI;

  DominatorTree &DT;

  /// Result of LoopWIAnalysis.
  LoopWIInfo &WIInfo;

  /// Preheader of the current loop.
  BasicBlock *Preheader;

  /// @brief header of the current loop.
  BasicBlock *Header;

  /// Latch of the current loop.
  BasicBlock *Latch;

  IRBuilder<> *Builder;

  /// i32 zero constant.
  Constant *Zero;

  /// i32 one constant.
  Constant *One;

  /// PHI nodes in loop header.
  SmallPtrSet<Value *, 4> HeaderPhi;

  /// Latch entries of PHI nodes in loop header.
  SmallPtrSet<Value *, 4> HeaderPhiLatchEntries;

  /// Strided instructions to be moved according to dominance order.
  SmallVector<Instruction *, 16> OrderedCandidates;

  /// Strided instruction to be moved.
  SmallPtrSet<Value *, 16> InstToMoveSet;

  /// Obtains loop header PHI nodes and their latch entries.
  void getHeaderPhi();

  /// Scan the loop for strided values to be moved to the preheader.
  /// \param N dominator tree node of currently processed basic block.
  void scanLoop(DomTreeNode *N);

  /// Move strided instruction to loop preheader, and create PHI node to replace
  /// them if needed.
  void hoistInstructions();

  /// Fix strided instruction that uses header phi nodes to use their preheader
  /// entries instead.
  void fixHeaderPhiOps(Instruction *I);

  /// Create PHI nodes in loop header for the moved strided instruction if
  /// needed.
  void createPhiIncrementors(Instruction *I);

  /// Return true if the instruction can be hoisted.
  bool canHoistInstruction(Instruction *I);

  /// Get stride of the instruction between loop iterations.
  Value *getStrideForInst(Instruction *I);

  /// If one of the instruction's operand is strided FMul, get stride from FMul
  /// instead in order to minimize floating-point accuracy loss.
  /// \param I the instruction.
  /// \param Width vector length.
  /// \return the stride, or nullptr if FMul is not found.
  Value *getStrideForInstFMul(Instruction *I, Value *Width);

  /// Get vector version of stride for the instruction if it is a vector.
  /// \param I the instruction.
  /// \param Stride scalar stride of the instruction.
  /// \return vector stride if it is a vector, stride if it is a scalar.
  Value *getVectorStrideIfNeeded(Instruction *I, Value *Stride);

  /// Screen from InstToMoveSet values that are likely to cause performance
  /// degradation.
  void screenNonProfitableValues();

  /// Fill \p Vec with users of \p V that are out InstToMoveSet.
  /// \param V the value whose users are checked.
  /// \param Vec vector to fill.
  void obtainNonHoistedUsers(Value *V, SmallVectorImpl<User *> &Vec);
};

} // namespace

bool LoopStridedCodeMotionImpl::run() {
  if (!L.isLoopSimplifyForm())
    return false;

  assert(Preheader && Header && Latch && "loop should be LoopSimplify form");

  Function *F = Header->getParent();
  if (F->hasOptNone())
    return false;

  LLVM_DEBUG(dbgs() << "Run LoopStridedCodeMotion on loop " << Header->getName()
                    << "\n");
  LLVM_DEBUG(dbgs().indent(2));

  IRBuilder<> IRB(Header->getContext());
  this->Builder = &IRB;

  auto *I32Ty = Type::getInt32Ty(F->getContext());
  Zero = ConstantInt::get(I32Ty, 0);
  One = ConstantInt::get(I32Ty, 1);

  // Obtain loop header PHI nodes and their latch entries for later use.
  getHeaderPhi();

  // Scans the loop for strided values to be moved to preheader.
  DomTreeNode *N = DT.getNode(Header);
  assert(N && "Could not get DT node for header");
  scanLoop(N);

  // Screens instruction that are expected to create performance degradation
  // from the hoisted instructions set.
  screenNonProfitableValues();

  // Hoist instructions out of the loop. Create PHI node to replace them if
  // needed.
  hoistInstructions();

  return !InstToMoveSet.empty();
}

void LoopStridedCodeMotionImpl::getHeaderPhi() {
  for (Instruction &I : *Header) {
    auto *PN = dyn_cast<PHINode>(&I);
    if (!PN)
      break;
    HeaderPhi.insert(PN);
    HeaderPhiLatchEntries.insert(PN->getIncomingValueForBlock(Latch));
  }
  LLVM_DEBUG(dbgs() << "HeaderPhi size " << HeaderPhi.size() << "\n");
  LLVM_DEBUG(dbgs() << "HeaderPhiLatchEntries size "
                    << HeaderPhiLatchEntries.size() << "\n");
}

void LoopStridedCodeMotionImpl::scanLoop(DomTreeNode *N) {
  BasicBlock *BB = N->getBlock();
  // Exit if this subregion is outside the current loop.
  if (!L.contains(BB))
    return;

  // Don't process instruction in sub loops.
  if (!LoopUtils::inSubLoop(BB, &L, &LI)) {
    for (auto &I : *BB) {
      // Avoid moving original header phi nodes or latch entries.
      if (HeaderPhi.contains(&I) || HeaderPhiLatchEntries.contains(&I)) {
        continue;
      }
      // Update if we can move the value.
      if (canHoistInstruction(&I)) {
        OrderedCandidates.push_back(&I);
        InstToMoveSet.insert(&I);
      }
    }
  }

  // Go over blocks recursively accoding to dominator tree.
  for (DomTreeNode *Child : N->children())
    scanLoop(Child);
}

void LoopStridedCodeMotionImpl::hoistInstructions() {
  Instruction *Loc = Preheader->getTerminator();
  // Since the instructions were inserted to OrderedCandidates according to the
  // dominator tree order, we know that we move def before its use.
  for (auto *I : OrderedCandidates) {
    if (!InstToMoveSet.contains(I))
      continue;
    LLVM_DEBUG(dbgs() << "hoist " << *I << "\n");
    // Fix phi node operands if exists.
    fixHeaderPhiOps(I);
    // Move to preheader.
    I->moveBefore(Loc);
    // Create phi that increments the strided and its users that were not moved
    // outside the loop.
    createPhiIncrementors(I);
  }
}

void LoopStridedCodeMotionImpl::fixHeaderPhiOps(Instruction *I) {
  // If an instruction users a header phi, use preheader entry instead.
  for (unsigned Idx = 0, E = I->getNumOperands(); Idx != E; ++Idx) {
    Value *Op = I->getOperand(Idx);
    if (!HeaderPhi.contains(Op))
      continue;
    auto *PH = cast<PHINode>(Op);
    I->setOperand(Idx, PH->getIncomingValueForBlock(Preheader));
  }
}

void LoopStridedCodeMotionImpl::createPhiIncrementors(Instruction *I) {
  // Get all users that are not moved outside of the loop.
  SmallVector<User *, 4> UsersToFix;
  obtainNonHoistedUsers(I, UsersToFix);

  if (UsersToFix.empty()) {
    LLVM_DEBUG(dbgs() << *I
                      << ": all users were hoisted, no need to create phi\n");
    return;
  }

  // Create phi node in header.
  auto *PN = PHINode::Create(I->getType(), 2, "", Header->getFirstNonPHI());
  PN->addIncoming(I, Preheader);

  // Create increment for the phi node.
  Value *Stride = getStrideForInst(I);
  Value *StrideToAdd = getVectorStrideIfNeeded(I, Stride);
  Builder->SetInsertPoint(Latch->getTerminator());
  Value *IncrementedVal;
  if (PN->getType()->isFPOrFPVectorTy())
    IncrementedVal = Builder->CreateFAdd(PN, StrideToAdd, "strided.add");
  else
    IncrementedVal = Builder->CreateAdd(PN, StrideToAdd, "strided.add");

  // Set nsw/nuw on the incremented value if the hoisted value has these flags.
  // The reason for checking the instruction type is that some of the binary
  // operations does not support wrap and assert in debug mode.
  // FIXME check if we covered all supported instructions.
  unsigned Opcode = I->getOpcode();
  if (Opcode == Instruction::Add || Opcode == Instruction::Sub ||
      Opcode == Instruction::Mul) {
    auto *BO = cast<BinaryOperator>(I);
    auto *IncrementedValInst = cast<Instruction>(IncrementedVal);
    if (BO->hasNoSignedWrap())
      IncrementedValInst->setHasNoSignedWrap();
    if (BO->hasNoUnsignedWrap())
      IncrementedValInst->setHasNoUnsignedWrap();
  }

  PN->addIncoming(IncrementedVal, Latch);

  // Replace users of the phi.
  for (User *U : UsersToFix)
    U->replaceUsesOfWith(I, PN);

  // Update LoopWIAnalysis about the new phi node.
  WIInfo.setValStrided(PN, dyn_cast<Constant>(Stride));
}

/// Return true if all users of I are ICmp/ExtractElement/Assume instructions.
static bool IsVectorAssume(Instruction *I) {
  ICmpInst *CmpUser;
  if (!I->hasOneUse() || !(CmpUser = dyn_cast<ICmpInst>(*I->user_begin())))
    return false;
  using namespace llvm::PatternMatch;
  CmpInst::Predicate Pred;
  Constant *C;
  if (!match(CmpUser, m_ICmp(Pred, m_Specific(I), m_Constant(C))) ||
      Pred != ICmpInst::ICMP_ULT)
    return false;
  if (!dyn_cast<ConstantDataVector>(C) ||
      C->getUniqueInteger() !=
          ((uint64_t)std::numeric_limits<int32_t>::max() + 1ULL))
    return false;

  return llvm::all_of(CmpUser->users(), [](User *U) {
    auto *EEI = dyn_cast<ExtractElementInst>(U);
    if (!EEI || !EEI->hasOneUse())
      return false;
    return match(*EEI->user_begin(),
                 m_Intrinsic<Intrinsic::assume>(m_Specific(EEI)));
  });
}

// Check if I is critical for floating point accuracy.
// We assume that I is a strided value or a user of strided value.
static bool isFPAccuracyCritical(Instruction *I) {
  // If I operates on fp values under non-fast mode, then we do care about the
  // accuracy.
  if (isa<FPMathOperator>(I) && !I->isFast()) {
    LLVM_DEBUG(dbgs().indent(2)
               << "FPMathOperator " << *I << " is critical for accuracy\n");
    return true;
  }

  // If I may write fp values to memory, then it should be
  // accuracy critical.
  if (I->mayWriteToMemory()) {
    LLVM_DEBUG(dbgs().indent(2) << "Memory writing instruction " << *I
                                << " is critical for accuracy\n");
    assert(
        any_of(I->operands(),
               [](Value *Op) { return Op->getType()->isFPOrFPVectorTy(); }) &&
        "I should have fp value operands");
    return true;
  }

  // If any user of a fp-cast instruction is accuracy critical, then the cast
  // instruction itself is critical too.
  bool IsCritical = false;
  switch (I->getOpcode()) {
  default:
    break;
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::FPExt:
  case Instruction::FPTrunc:
    IsCritical = any_of(I->users(), [](User *U) {
      auto *IU = dyn_cast<Instruction>(U);
      return IU && isFPAccuracyCritical(IU);
    });
    break;
  }
  if (IsCritical)
    LLVM_DEBUG(dbgs().indent(2) << "FP casting instruction " << *I
                                << " is critical for accuracy\n");

  return IsCritical;
}

bool LoopStridedCodeMotionImpl::canHoistInstruction(Instruction *I) {
  // Can move strided values or their intermediates.
  if (!WIInfo.isStrided(I) && !WIInfo.isStridedIntermediate(I)) {
    LLVM_DEBUG(dbgs() << "Can't hoist not strided: " << *I << "\n");
    return false;
  }

  // Currently support moving strided scalars only if their stride is constant.
  // Stride of strided vector can be computed by subtracting vector elements.
  if (!I->getType()->isVectorTy() && !WIInfo.getConstStride(I)) {
    LLVM_DEBUG(dbgs() << "Can't hoist nonconst strided scalar: " << *I << "\n");
    return false;
  }

  // To avoid accuracy loss, don't hoist accuracy critical instructions.
  if (isFPAccuracyCritical(I)) {
    LLVM_DEBUG(dbgs() << "Can't hoist fp accuracy critical instruction: " << *I
                      << '\n');
    return false;
  }

  // Instruction can be hoisted to preheader only if all of its operands will be
  // valid in preheader. This means that they are one of following:
  //   * invariant value.
  //   * strided values that were marked for removal before.
  //   * header phi node for which we can use its preheader entry.
  for (Value *Op : I->operands()) {
    if (!L.isLoopInvariant(Op) && !InstToMoveSet.contains(Op) &&
        !HeaderPhi.contains(Op)) {
      LLVM_DEBUG(dbgs() << "Can't hoist having unsupported operands: " << *I
                        << "\n");
      return false;
    }
  }

  // Don't hoise If I is for llvm.assume usage.
  if (IsVectorAssume(I))
    return false;

  LLVM_DEBUG(dbgs() << "Can hoist " << *I << "\n");

  return true;
}

Value *LoopStridedCodeMotionImpl::getStrideForInst(Instruction *I) {
  if (Constant *ConstStride = WIInfo.getConstStride(I))
    return ConstStride;

  assert(WIInfo.isStrided(I) && "strided immediate must have const stride");

  // Non constant stride must be vectors.
  auto *VTy = dyn_cast<FixedVectorType>(I->getType());
  assert(VTy && "I should be a vector");
  unsigned NumElts = VTy->getNumElements();
  Type *BaseTy = VTy->getElementType();

  // Calculate the stride as subtraction of the first two elements.
  Value *Width;
  if (BaseTy->isFloatingPointTy()) {
    Width = ConstantFP::get(BaseTy, (double)NumElts);
    if (Value *S = getStrideForInstFMul(I, Width))
      return S;
  } else {
    Width = ConstantInt::get(BaseTy, NumElts);
  }
  Builder->SetInsertPoint(Preheader->getTerminator());
  Builder->SetCurrentDebugLocation(I->getDebugLoc());
  Value *Elt0 = Builder->CreateExtractElement(I, Zero, "extract.0");
  Value *Elt1 = Builder->CreateExtractElement(I, One, "extract.1");
  if (BaseTy->isFloatingPointTy()) {
    Value *SubVal = Builder->CreateFSub(Elt1, Elt0, "sub.delta");
    return Builder->CreateFMul(SubVal, Width, "mul.delta");
  }
  Value *SubVal = Builder->CreateSub(Elt1, Elt0, "sub.delta");
  return Builder->CreateMul(SubVal, Width, "mul.delta");
}

// If FMul instruction is strided, avoid succeeding add/subtract which may bring
// additional accuracy loss.
Value *LoopStridedCodeMotionImpl::getStrideForInstFMul(Instruction *I,
                                                       Value *Width) {
  // Ignore all add/subtract instructions which have a strided operand and an
  // uniform operand. Find the nearest preceding strided FMul.
  while (I) {
    unsigned Opcode = I->getOpcode();
    if (Opcode != Instruction::FAdd && Opcode != Instruction::FSub)
      break;
    Value *Op0 = I->getOperand(0);
    Value *Op1 = I->getOperand(1);
    if (WIInfo.isUniform(Op0))
      I = dyn_cast<Instruction>(Op1);
    else if (WIInfo.isUniform(Op1))
      I = dyn_cast<Instruction>(Op0);
    else
      break;
  }

  if (!I || I->getOpcode() != Instruction::FMul)
    return nullptr;

  assert(WIInfo.isStrided(I) && "FMul should be strided");

  // FMul is strided. Obtain its strided operand's stride, multiple with its
  // uniform operand, and then multiply with vector length.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);
  bool IsOp0Uniform = WIInfo.isUniform(Op0);
  Value *OpUniform = IsOp0Uniform ? Op0 : Op1;
  Value *OpStrided = IsOp0Uniform ? Op1 : Op0;

  Builder->SetInsertPoint(Preheader->getTerminator());
  Builder->SetCurrentDebugLocation(I->getDebugLoc());
  Value *Elt0 = Builder->CreateExtractElement(OpStrided, Zero, "extract.0");
  Value *Elt1 = Builder->CreateExtractElement(OpStrided, One, "extract.1");
  Value *SubV = Builder->CreateFSub(Elt1, Elt0, "sub.delta");
  Value *EltUni =
      Builder->CreateExtractElement(OpUniform, Zero, "extract.uniform");
  Value *MulV = Builder->CreateFMul(EltUni, Width, "mul.uniform.width");
  return Builder->CreateFMul(SubV, MulV, "mul.delta");
}

Value *LoopStridedCodeMotionImpl::getVectorStrideIfNeeded(Instruction *I,
                                                          Value *Stride) {
  // For scalars, just return stride.
  auto *VTy = dyn_cast<FixedVectorType>(I->getType());
  if (!VTy)
    return Stride;

  unsigned NumElts = VTy->getNumElements();
  // For constant stride, just generate constant vector.
  if (Constant *C = dyn_cast<Constant>(Stride))
    return ConstantDataVector::getSplat(NumElts, C);

  // For non-constant, broadcast the stride.
  return VectorizerUtils::createBroadcast(Stride, NumElts,
                                          Preheader->getTerminator());
}

// When hoisting an instruction, if all its users are also hoisted, then the
// instruction can be hoisted and this is a clear gain.
// However, if some users are not hoisted, we need to make phi that is
// incremented at each iteration. This is not a clear gain since the phi is
// alive throughout the entire loop.
// Therefore, we screen some hoisted values that are not creating at least some
// values to be hoisted with clear gain. Also we avoid using phi for scalars,
// and shl instruction (this was tested empirically).
void LoopStridedCodeMotionImpl::screenNonProfitableValues() {
  auto HasUserInSet = [](Value *V, SmallPtrSetImpl<Value *> &S) {
    for (User *U : V->users())
      if (S.contains(U))
        return true;
    return false;
  };

  // Scan the instruction marked to move in reverse order, meaning use before
  // def.
  for (auto It = OrderedCandidates.rbegin(), E = OrderedCandidates.rend();
       It != E; ++It) {
    Instruction *I = *It;
    // If a user of this instruction was hoisted, we must hoist this instruction
    // as well.
    if (HasUserInSet(I, InstToMoveSet))
      continue;

    // We avoid hoisting an instruction if none of its operands will be hoisted
    // freely (with no phi generated).
    bool IsProfitable = false;
    for (unsigned Idx = 0, E = I->getNumOperands(); Idx != E && !IsProfitable;
         ++Idx) {
      Value *Op = I->getOperand(Idx);
      // Obtain non-hoisted users of this operand.
      SmallVector<User *, 4> NonHoistedUsers;
      obtainNonHoistedUsers(Op, NonHoistedUsers);
      // If Op is marked for moving and all its users (including I) are also
      // marked, then I makes Op hoisted with no need for phi node.
      IsProfitable |= NonHoistedUsers.empty() && InstToMoveSet.contains(Op);
      // Also if Op is a header phi node whose only non-hoisted user is its
      // increment, then by hoisting I the op will become dead and removed by
      // ADCE.
      IsProfitable |= NonHoistedUsers.size() == 1 && HeaderPhi.contains(Op) &&
                      cast<PHINode>(Op)->getIncomingValueForBlock(Latch) ==
                          NonHoistedUsers[0];
    }

    // Can not have strided intermediate as pivot strided value.
    // Empirically, creating phi nodes for scalar values is non profitable.
    bool IsScalar = !I->getType()->isVectorTy();
    if (!IsProfitable || WIInfo.isStridedIntermediate(I) || IsScalar) {
      InstToMoveSet.erase(I);
      LLVM_DEBUG(dbgs() << "Not hoist " << *I << ". Not profitable\n");
    }
  }
}

void LoopStridedCodeMotionImpl::obtainNonHoistedUsers(
    Value *V, SmallVectorImpl<User *> &Vec) {
  for (User *U : V->users())
    if (!InstToMoveSet.contains(U))
      Vec.push_back(U);
}

namespace {

class LoopStridedCodeMotionLegacy : public LoopPass {
public:
  static char ID;

  LoopStridedCodeMotionLegacy() : LoopPass(ID) {
    initializeLoopStridedCodeMotionLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnLoop(Loop *L, LPPassManager &) override {
    if (skipLoop(L))
      return false;
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopWIInfo &WIInfo = getAnalysis<LoopWIAnalysisLegacy>().getResult();
    LoopStridedCodeMotionImpl Impl(*L, LI, DT, WIInfo);
    return Impl.run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<LoopWIAnalysisLegacy>();
    AU.setPreservesCFG();
  }
};

} // namespace

INITIALIZE_PASS_BEGIN(LoopStridedCodeMotionLegacy, DEBUG_TYPE,
                      "Hoist strided values out of loops", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopWIAnalysisLegacy)
INITIALIZE_PASS_END(LoopStridedCodeMotionLegacy, DEBUG_TYPE,
                    "Hoist strided values out of loops", false, false)

char LoopStridedCodeMotionLegacy::ID = 0;

LoopPass *llvm::createLoopStridedCodeMotionLegacyPass() {
  return new LoopStridedCodeMotionLegacy();
}

PreservedAnalyses
LoopStridedCodeMotionPass::run(Loop &L, LoopAnalysisManager &AM,
                               LoopStandardAnalysisResults &AR, LPMUpdater &) {
  LoopWIInfo &WIInfo = AM.getResult<LoopWIAnalysis>(L, AR);
  LoopStridedCodeMotionImpl Impl(L, AR.LI, AR.DT, WIInfo);
  if (!Impl.run())
    return PreservedAnalyses::all();

  auto PA = getLoopPassPreservedAnalyses();
  if (AR.MSSA)
    PA.preserve<MemorySSAAnalysis>();

  return PA;
}
