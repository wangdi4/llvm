// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "LoopStridedCodeMotion.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "VectorizerUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"

#define DEBUG_TYPE "cl-loop-stride"

namespace intel {


char LoopStridedCodeMotion::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(LoopStridedCodeMotion, DEBUG_TYPE,
                          "move strided values out of loops", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopWIAnalysisLegacy)
OCL_INITIALIZE_PASS_END(LoopStridedCodeMotion, DEBUG_TYPE,
                        "move strided values out of loops", false, false)

LoopStridedCodeMotion::LoopStridedCodeMotion() : LoopPass(ID) {
  initializeLoopStridedCodeMotionPass(*PassRegistry::getPassRegistry());
}

// this pass assumes LICM ran before it, moved loop invariant values
// outside of the loop.
bool LoopStridedCodeMotion::runOnLoop(Loop *L, LPPassManager & /*LPM*/) {
  const Function *F = L->getHeader()->getParent();
  if (F && F->hasOptNone())
    return false;
  //if (counter++ != getIntEnvVarVal("STRIDED_COUNTER")) return false;
  //errs() << "\n\nLoopStridedCodeMotion on " << L->getHeader()->getNameStr() << "\n";
  //errs() << "input: " << *(L->getHeader()->getParent());
  if (!L->isLoopSimplifyForm()) return false;


  m_DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  m_LoopWIInfo = &getAnalysis<LoopWIAnalysisLegacy>().getResult();
  m_curLoop = L;
  m_header = m_curLoop->getHeader();
  m_preHeader = m_curLoop->getLoopPreheader();
  m_latch = m_curLoop->getLoopLatch();
  assert(m_latch && m_preHeader && "should have latch and pre header");

  // Some usefull constants.
  m_i32Ty  = IntegerType::get(m_header->getContext(), 32);
  m_one = ConstantInt::get(m_i32Ty, 1);
  m_zero = ConstantInt::get(m_i32Ty, 0);

  // Clear data structures.
  m_orderedCandidates.clear();
  m_instToMoveSet.clear();
  m_headerPhi.clear();
  m_headerPhiLatchEntries.clear();

  /// Obtain the loop header phi nodes and their latch entries for later use.
  getHeaderPHi();

  // Scans the loop for strided values to be moved to the pre-header.
  DomTreeNode* DTNode = m_DT->getNode(m_header);
  assert(DTNode && "Could not get DT node for header");
  ScanLoop(DTNode);

  // Screens instruction that are expected to create performance degradation
  // from the hoisted instructions set.
  screenNonProfitableValues();

  // Move the marked instructions outside the loop, create Phi node to replace
  // them if needed.
  HoistMarkedInstructions();

  // clear WI Analysis data since they are not in the loop anymore.
  for (Value *V : m_instToMoveSet)
    m_LoopWIInfo->clearValDep(V);

  // changed if any instruction was moved.

  //errs() << "output: " << *(L->getHeader()->getParent());
  return m_instToMoveSet.size();
}

void LoopStridedCodeMotion::getHeaderPHi() {
  assert(m_header && m_latch && "loop should be simple form");
  for (BasicBlock::iterator I = m_header->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    m_headerPhi.insert(PN);
    m_headerPhiLatchEntries.insert(PN->getIncomingValueForBlock(m_latch));
  }
}

void LoopStridedCodeMotion::ScanLoop(DomTreeNode *N) {
  assert(N != 0 && "Null dominator tree node?");
  BasicBlock *BB = N->getBlock();

  // If this subregion is outside the current loop exit.
  if (!m_curLoop->contains(BB)) return;

  // We don't process instruction in sub-loops.
  if (!LoopUtils::inSubLoop(m_curLoop, BB)) {
    for (BasicBlock::iterator IIt = BB->begin(), EIt = BB->end(); IIt != EIt; IIt++) {
      Instruction* I = &*IIt;
      // Avoid moving original header phi nodes or latch entries.
      if (m_headerPhiLatchEntries.count(I) || m_headerPhi.count(I)) continue;

      // If we can move the value update data structures.
      if (canHoistInstruction(I)) {
        m_orderedCandidates.push_back(I);
        m_instToMoveSet.insert(I);
      }
    }
  }

  // Go over blocks recursively according to Dominator tree.
  for (DomTreeNode *Children : N->children()) ScanLoop(Children);
}



bool LoopStridedCodeMotion::canHoistInstruction(Instruction *I) {
  // Can move strided values or their intermediates.
  if (!m_LoopWIInfo->isStrided(I) && !m_LoopWIInfo->isStridedIntermediate(I)){
    return false;
  }
  // Currently support move strided scalars only if their stride is constant.
  // Strided vector stride can be computed by substracting vector elements.
  if (!I->getType()->isVectorTy() && !m_LoopWIInfo->getConstStride(I)) {
    return false;
  }

  // To avoid accuracy loss, don't hoist fmul instruction without fast flag.
  if (I->getOpcode() == Instruction::FMul && !I->isFast())
    return false;

  // I can be moved to the pre-header only if all it's operands will be valid
  // in the pre-header. This means that they are either invariant, or they are
  // strided values that were marked for removal before, or they are header phi
  // node for which we can use the pre-header entry.
  for (unsigned j=0; j<I->getNumOperands(); ++j) {
    Value *op = I->getOperand(j);
    if (!m_instToMoveSet.count(op) &&
        !m_curLoop->isLoopInvariant(op) &&
        !m_headerPhi.count(op) ) {
      return false;
    }
  }
  LLVM_DEBUG(dbgs() << "can be moved: " << I << "   " << *I << "\n");
  return true;
}

void LoopStridedCodeMotion::HoistMarkedInstructions() {
  Instruction *loc = m_preHeader->getTerminator();
  // Note that since the instructions were inserted to m_orderedCandidates
  // according to the dominator tree order we know that we move def before
  // it's use.
  for (unsigned i=0; i<m_orderedCandidates.size(); ++i) {
    Instruction *I = m_orderedCandidates[i];
    if (!m_instToMoveSet.count(I)) continue;
    // Fix header phi operands if exists.
    fixHeaderPhiOps(I);
    // Move to pre header.
    I->moveBefore(loc);
    // Create Phi that increments the strided and it's users that were
    // not moved outside the loop.
    createPhiIncrementors(I);
  }
}

void LoopStridedCodeMotion::fixHeaderPhiOps(Instruction *I) {
// If an instruction uses an header phi use the preheader entry instead.
  for (unsigned j=0; j<I->getNumOperands(); ++j) {
    Value *op = I->getOperand(j);
    if (m_headerPhi.count(op)) {
      assert(isa<PHINode>(op) && "set contains non phi");
      PHINode *PN = cast<PHINode>(op);
      Value *preHeaderOp = PN->getIncomingValueForBlock(m_preHeader);
      I->setOperand(j, preHeaderOp);
    }
  }
}

void LoopStridedCodeMotion::createPhiIncrementors(Instruction *I) {
  // Get all users that were not moved outside the loop.
  SmallVector<User *, 4> usersToFix;
  ObtainNonHoistedUsers(I, usersToFix);
  // If all users were moved outside there is no need to create phi.
  //errs() << "moving " << *I;
  if (!usersToFix.size()) {
    //errs() << "  all users were moved, no need for phi\n";
    return;
  }
  //errs() << "  will make phi node\n";
  // Create phi node in the loop header.
  PHINode *PN =
      PHINode::Create(I->getType(), 2, "", m_header->getFirstNonPHI());
  PN->addIncoming(I, m_preHeader);

  // Create increment for the phi node.
  Value *stride = getStrideForInst(I);
  Value *strideToAdd = getVectorStrideIfNeeded(I, stride);
  IRBuilder<> Builder(m_latch->getTerminator());
  Value *incrementedVal;
  if (PN->getType()->isFPOrFPVectorTy())
    incrementedVal = Builder.CreateFAdd(PN, strideToAdd, "strided.add");
  else
    incrementedVal = Builder.CreateAdd(PN, strideToAdd, "strided.add");

  // Set nsw, nuw on the incremented phi incase the hoisted value has these
  // flags.
  // The reason for checking the instruction type is that some of the binary
  // operations does not support wrap and assert in debug mode.
  // TODO:: check if we covered all supported instructions.
  unsigned opcode  = I->getOpcode();
  if (opcode == Instruction::Add || opcode == Instruction::Sub ||
      opcode == Instruction::Mul) {
    BinaryOperator *BO = cast<BinaryOperator>(I);
    Instruction *IncrementedValInst = cast<Instruction>(incrementedVal);
    if (BO->hasNoSignedWrap())
      IncrementedValInst->setHasNoSignedWrap();
    if (BO->hasNoUnsignedWrap())
      IncrementedValInst->setHasNoUnsignedWrap();
  }

  PN->addIncoming(incrementedVal, m_latch);

  // Replace the users with the phi.
  for (unsigned i=0; i<usersToFix.size(); ++i) {
    usersToFix[i]->replaceUsesOfWith(I, PN);
  }

  // Update the analysis about the new phi node val.
  m_LoopWIInfo->setValStrided(PN, dyn_cast<Constant>(stride));
}

Value *LoopStridedCodeMotion::getStrideForInst(Instruction *I) {
  Constant *constStride = m_LoopWIInfo->getConstStride(I);
  if (constStride) return constStride;
  assert(m_LoopWIInfo->isStrided(I) &&
         "strided intermediate must have constant stride");

  // Non constant strides must be vectors.
  FixedVectorType *vTy = dyn_cast<FixedVectorType>(I->getType());
  assert(vTy && "input should be a vector");
  unsigned nElts = vTy->getNumElements();
  Type *baseType = vTy->getElementType();

  // Calculate the stride as the substraction of the first two elements.
  Value *Width;
  if (baseType->isFloatingPointTy()) {
    Width = ConstantFP::get(baseType, static_cast<double>(nElts));
    if (Value *S = getStrideForInstFMul(I, Width))
      return S;
  } else {
    Width = ConstantInt::get(baseType, nElts);
  }
  IRBuilder<> Builder(m_preHeader->getTerminator());
  Builder.SetCurrentDebugLocation(I->getDebugLoc());
  Value *Elt0 = Builder.CreateExtractElement(I, m_zero, "extract.0");
  Value *Elt1 = Builder.CreateExtractElement(I, m_one, "extract.1");
  if (baseType->isFloatingPointTy()) {
    Value *SubVals = Builder.CreateFSub(Elt1, Elt0, "sub.delta");
    return Builder.CreateFMul(SubVals, Width, "mul.delta");
  } else {
    Value *SubVals = Builder.CreateSub(Elt1, Elt0, "sub.delta");
    return Builder.CreateMul(SubVals, Width, "mul.delta");
  }
}

// If FMul instruction is strided, avoid succeeding add/subtract which may bring
// additional accuracy loss.
Value *LoopStridedCodeMotion::getStrideForInstFMul(Instruction *I,
                                                   Value *Width) {
  // Ignore all add/substract instructions which have an strided operand and an
  // uniform operand. Find the nearest preceding strided FMul.
  while (I) {
    unsigned Opcode = I->getOpcode();
    if (Opcode != Instruction::FAdd && Opcode != Instruction::FSub)
      break;
    Value *Op0 = I->getOperand(0);
    Value *Op1 = I->getOperand(1);
    if (m_LoopWIInfo->isUniform(Op0))
      I = dyn_cast<Instruction>(Op1);
    else if (m_LoopWIInfo->isUniform(Op1))
      I = dyn_cast<Instruction>(Op0);
    else
      break;
  }

  if (!I || I->getOpcode() != Instruction::FMul)
    return nullptr;

  assert(m_LoopWIInfo->isStrided(I) && "FMul should be strided");

  // FMul is strided. Obtain its strided operand's stride, multiply with its
  // uniform operand, and then multiply with vector length.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);
  bool IsOp0Uniform = m_LoopWIInfo->isUniform(Op0);
  Value *OpUniform = IsOp0Uniform ? Op0 : Op1;
  Value *OpStrided = IsOp0Uniform ? Op1 : Op0;

  IRBuilder<> Builder(m_preHeader->getTerminator());
  Builder.SetCurrentDebugLocation(I->getDebugLoc());
  Value *Elt0 = Builder.CreateExtractElement(OpStrided, m_zero, "extract.0");
  Value *Elt1 = Builder.CreateExtractElement(OpStrided, m_one, "extract.1");
  Value *SubV = Builder.CreateFSub(Elt1, Elt0, "sub.delta");
  Value *EltUni =
      Builder.CreateExtractElement(OpUniform, m_zero, "extract.uniform");
  Value *MulV = Builder.CreateFMul(EltUni, Width, "mul.uniform.width");
  return Builder.CreateFMul(SubV, MulV, "mul.delta");
}

Value *LoopStridedCodeMotion::getVectorStrideIfNeeded(Instruction *I,
                                                      Value *stride) {
  // For scalars just return the stride.
  FixedVectorType *vTy = dyn_cast<FixedVectorType>(I->getType());
  if (!vTy) return stride;

  unsigned nElts = vTy->getNumElements();
  // For constant stride just generate constant vector.
  if (Constant *C = dyn_cast<Constant>(stride)) {
    return ConstantDataVector::getSplat(nElts, C);
  }

  // For non constant broadcast the stride.
  Instruction *loc = m_preHeader->getTerminator();
  Instruction *SVI = VectorizerUtils::createBroadcast(stride, nElts, loc);
  return SVI;
}


bool hasUserInSet(Value *v, SmallPtrSet<Value *, 16>& s) {
  for (Value::user_iterator useIt = v->user_begin(), useE = v->user_end();
      useIt != useE; ++useIt) {
    if (s.count(*useIt)) return true;
  }
  return false;
}


void LoopStridedCodeMotion::ObtainNonHoistedUsers(Value *v,
                                                SmallVectorImpl<User *> &vec) {
  for (Value::user_iterator useIt = v->user_begin(), useE = v->user_end();
      useIt != useE; ++useIt) {
    if (!m_instToMoveSet.count(*useIt)) vec.push_back(*useIt);
  }
}


// When hoisting an instruction if all it's users are also hoisted than
// the instruction can be just hoisted amnd this is a clear gain. However
// when if some users are not we need to make phi that is incremented at
// each iteration. This is not a clear gain since the phi is alive
// throughout the entire loop. Thus, we screen some hoisted values that
// which are not creating at least some values to be hoisted with clear
// gain. Also we aviod using phi for scalars, and shl instruction
// (this was tested empirically).
void LoopStridedCodeMotion::screenNonProfitableValues() {
  // Scan the instruction marked to move in reverse order meaning use
  // before def.
  for (int i=m_orderedCandidates.size()-1; i>=0; --i) {
    Instruction *I = m_orderedCandidates[i];
    // If a user of this intruction was marked to be hoisted than we must hoist
    // this instruction also.
    if (hasUserInSet(I, m_instToMoveSet)) continue;


    // We avoid hoisting an instruction if none of it's operands will be
    // hoisted freely (with no phi generated)
    bool isProfitable = false;
    for (unsigned i=0, e=I->getNumOperands(); i < e && !isProfitable; ++i) {
      Value *op = I->getOperand(i);
      // Obtain the non hoisted users of the current operand.
      SmallVector<User *, 4> NonHoistedUSers;
      ObtainNonHoistedUsers(op, NonHoistedUSers);
      // If op is marked for moving and all it's users (including I) are also marked
      // then I makes op hoisted with no need for phi node.
      isProfitable |= (NonHoistedUSers.size() == 0 && m_instToMoveSet.count(op) );
      // Also if op is a header phi node whose only non hoisted user is it's increment,
      // then by hoisting I the op will become dead and removed by ADCE.
      isProfitable |= (NonHoistedUSers.size() == 1 && m_headerPhi.count(op) &&
           cast<PHINode>(op)->getIncomingValueForBlock(m_latch) == NonHoistedUSers[0]);
    }

    // Can not have strided intermediate as pivot strided value.
    // Empirically, creating phi nodes for scalar values is non profitable.
    bool isScalar = !I->getType()->isVectorTy();
    if (!isProfitable || m_LoopWIInfo->isStridedIntermediate(I) || isScalar) {
      m_instToMoveSet.erase(I);
      //errs() << "will not move - not profitable: " << *I << "\n";
    }
  }
}

}// namespace intel

extern "C" {
  Pass* createLoopStridedCodeMotionPass() {
    return new intel::LoopStridedCodeMotion();
  }
}
