/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "LoopWIAnalysis.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"


namespace intel {

char intel::LoopWIAnalysis::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(LoopWIAnalysis, "LoopWIAnalysis", "LoopWIAnalysis provides work item dependency info for loops", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_END(LoopWIAnalysis, "LoopWIAnalysis", "LoopWIAnalysis provides work item dependency info for loops", false, false)

const unsigned int LoopWIAnalysis::MinIndexBitwidthToPreserve = 16;

/// Dependency maps (define output dependency according to 2 input deps)

#define UNI LoopWIAnalysis::UNIFORM
#define STR LoopWIAnalysis::STRIDED
#define RND LoopWIAnalysis::RANDOM

static const LoopWIAnalysis::ValDependancy
add_conversion[LoopWIAnalysis::NumDeps][LoopWIAnalysis::NumDeps] = {
  /*          UNI, STR, RND */
  /* UNI */  {UNI, STR, RND},
  /* STR */  {STR, STR, RND},
  /* RND */  {RND, RND, RND}
};

static const LoopWIAnalysis::ValDependancy
mul_conversion[LoopWIAnalysis::NumDeps][LoopWIAnalysis::NumDeps] = {
  /*          UNI, STR, RND */
  /* UNI */  {UNI, STR, RND},
  /* STR */  {STR, RND, RND},
  /* RND */  {RND, RND, RND}
};


LoopWIAnalysis::LoopWIAnalysis():LoopPass(ID){
  initializeLoopWIAnalysisPass(*PassRegistry::getPassRegistry());
}

LoopWIAnalysis::ValDependancy LoopWIAnalysis::getDependency(Value *val) {
  if (m_deps.count(val)) return m_deps[val];
  // We go throgh instruction inside the loops according to Dominator Tree
  // Thus the only value that were not encounterd before should be invariant
  // or instruction computed inside sub-loops.
  bool isInvariant = m_curLoop->isLoopInvariant(val);
  assert((isInvariant || (isa<Instruction>(val)
     && LoopUtils::inSubLoop(m_curLoop, cast<Instruction>(val))))
    && "non invariant value with no dep");

  // We dont assume anything on values computed in sub-loops.
  if (!isInvariant) return LoopWIAnalysis::RANDOM;

  // Non - vector invariants are considered uniform.
  if (!val->getType()->isVectorTy()) {
    m_deps[val] = LoopWIAnalysis::UNIFORM;
    return LoopWIAnalysis::UNIFORM;
  }

  // Vectors are considered uniform only if all the elements are the same.
  ValDependancy res = LoopWIAnalysis::RANDOM;
  if (ShuffleVectorInst *SVI = dyn_cast<ShuffleVectorInst>(val)) {
    if (isBroadcast(SVI)) res = LoopWIAnalysis::UNIFORM;
  } else if (ConstantVector *CV = dyn_cast<ConstantVector>(val)) {
    if (CV->getSplatValue()) res = LoopWIAnalysis::UNIFORM;
  } else if (ConstantDataVector *CDV = dyn_cast<ConstantDataVector>(val)) {
    if (CDV->getSplatValue()) res = LoopWIAnalysis::UNIFORM;
  } else if (isa<ConstantAggregateZero>(val)) {
    // all the elements of zeroinitializer are the same.
    res = LoopWIAnalysis::UNIFORM;
  }
  m_deps[val] = res;
  return res;
}

bool LoopWIAnalysis::runOnLoop(Loop *L, LPPassManager &LPM) {
  //errs() << "LoopWIAnalysis on " << L->getHeader()->getNameStr() << "\n";
  if (!L->isLoopSimplifyForm()) return false;
  m_DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  m_curLoop = L;
  m_header = m_curLoop->getHeader();
  m_preHeader = m_curLoop->getLoopPreheader();
  m_latch = m_curLoop->getLoopLatch();
  assert(m_latch && m_preHeader && "should have latch and pre header");

  // Clear data stuctures.
  m_deps.clear();
  m_constStrides.clear();
  m_headerPhi.clear();
  m_stridedIntermediate.clear();

  // Analyze header phi nodes for stride values.
  getHeaderPHiStride();

  // Analyze rest of the loop instructions.
  DomTreeNode* DTNode = m_DT->getNode(m_header);
  assert(DTNode && "Could not get DT node for header");
  // In release, don't try to analyze...
  if (!DTNode)
    return false;

  ScanLoop(DTNode);

  // Analysis does not change the module.
  return false;
}

void LoopWIAnalysis::getHeaderPHiStride() {
  assert(m_header && m_latch && "loop should be simple form");

  // Loop over all of the PHI nodes, looking for a canonical indvar.
  for (BasicBlock::iterator I = m_header->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    m_headerPhi.insert(PN);
    // Initiate the phi with random, if it is found to be strided it will
    // be updated in the following lines of code.
    m_deps[PN] = LoopWIAnalysis::RANDOM;

    // Currently support only scalar phi in the header block.
    if (PN->getType()->isVectorTy()) continue;

    // The latch entry is and addition.
    Value *latchVal = PN->getIncomingValueForBlock(m_latch);
    Instruction *Inc = dyn_cast<Instruction>(latchVal);
    if (!Inc) continue;
    if (Inc->getOpcode() != Instruction::Add &&
        Inc->getOpcode() != Instruction::FAdd) continue;

    //Now check that all the added value is loop invariant.
    Value *stride = NULL;
    Value *op0 = Inc->getOperand(0);
    Value *op1 = Inc->getOperand(1);
    // isLoopInvariant means that the operand is either not an instruction
    // or it is an instruction outside of the loop. Note that we assume
    // LICM ran before.
    if (op0 == PN && m_curLoop->isLoopInvariant(op1)) {
      stride = op1;
    } else if (op1 == PN && m_curLoop->isLoopInvariant(op0)) {
      stride = op0;
    }
    if (!stride) continue;

    // PN is incremented with invariant values so it is strided.
    m_deps[PN] = LoopWIAnalysis::STRIDED;
    
    // Try to update the constant stride
    Constant *constStride = dyn_cast<Constant>(stride);
    if (!constStride)
      continue;
    
    // For vector values, this works only if the stride is a splat
    ConstantDataVector* vectorStride = dyn_cast<ConstantDataVector>(constStride);

    if (vectorStride) {
      constStride = vectorStride->getSplatValue();
      if (!constStride)
        continue;
    }
    
    m_constStrides[PN] = constStride;
  }
}


void LoopWIAnalysis::calculate_dep(Instruction* I) {
  assert(I && "Bad value");
  // Default dependency is Random.
  ValDependancy res = LoopWIAnalysis::RANDOM;

  // Handling supported that can generate stride values.
  if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
    res = calculate_dep(BI);
  } else if (CastInst *CI = dyn_cast<CastInst>(I)) {
    res = calculate_dep(CI);
    updateConstStride(CI, CI->getOperand(0));
  } else if (ExtractElementInst *EEI = dyn_cast<ExtractElementInst>(I)) {
    res = calculate_dep(EEI);
  }
  m_deps[I] = res;
}

// Checks if the shuffle is broadcast (all masks are the same).
// We do not count undef masks as different.
bool LoopWIAnalysis::isBroadcast(ShuffleVectorInst *SVI) {
  assert(SVI && "null argument");

  VectorType *vTy = dyn_cast<VectorType>(SVI->getType());
  assert (vTy && "shuffle should have vector type");
  unsigned nElts = vTy->getNumElements();
  int ind = SVI->getMaskValue(0);
  for (unsigned i=1; i<nElts; ++i) {
    // For undef, getMaskValue() returns -1
    int maskI = SVI->getMaskValue(i);
    if ((maskI != ind) && (maskI != -1)) return false;
  }

  // Getting here all mask values are 0 so it is a broadcast!!
  return true;
}

// Checks is v is constant vector of the from <0, 1, 2, ...>
// LLVM 3.2 migration node - vectors of sane integer types will
// always be ConstantDataVectors, there is no need to check for
// ConstantVector
bool LoopWIAnalysis::isConsecutiveConstVector(Value *v) {
  assert(v && "bad argument");
  const ConstantDataVector *CV = dyn_cast<ConstantDataVector>(v);
  if (!CV) return false;

  assert(isa<VectorType>(CV->getType()) &&
                    "constant vector should have vector type");
  VectorType *vTy = cast<VectorType>(CV->getType());
  // Case not vector int type return false.
  if (!vTy->isIntOrIntVectorTy()) return false;
  
  for (unsigned i=0, nElts = vTy->getNumElements(); i<nElts; ++i) {
    ConstantInt *C = cast<ConstantInt>(CV->getAggregateElement(i));
    // If this is not sequnce 0,1,2,... return false
    if (!C->equalsInt(i)) return false;
  }
  // All values are sequential.
  return true;
}

// Checks if I is generetion of sequential ids.
// Only supports the pattern generated by the vectorizer:
// %a = insertelement <w x i#> undef, i# stride_val, i32 x
// %b = shufflevector <w x i#> %a, <w x i#> undef, <4 x i32> same_val
// #c = add <w x i#> %b, <i# 0, i# 1, ...>
bool LoopWIAnalysis::isSequentialVector(Instruction *I) {
  assert (I && I->getOpcode() == Instruction::Add && "expected add inst");
  VectorType *vTy= dyn_cast<VectorType>(I->getType());
  if (!vTy) return false;

  // Pattern of of creation sequential ids is addition of constant <0, 1, 2 ,..>
  // with broadcast using shuffle.
  Value *op0 = I->getOperand(0);
  Value *op1 = I->getOperand(1);
  ShuffleVectorInst *SVI = NULL;
  if (isConsecutiveConstVector(op0)) {
    SVI = dyn_cast<ShuffleVectorInst>(op1);
  } else if (isConsecutiveConstVector(op1)) {
    SVI = dyn_cast<ShuffleVectorInst>(op0);
  }
  if (!SVI || !isBroadcast(SVI)) return false;

  // We further demand that the shuffle is broadcast of strided value with
  // the same width of the vector.
  unsigned maskVal = SVI->getMaskValue(0);
  InsertElementInst *IEI = dyn_cast<InsertElementInst>(SVI->getOperand(0));
  if (!IEI || !IEI->hasOneUse()) return false;
  ConstantInt *C = dyn_cast<ConstantInt>(IEI->getOperand(2));
  if (!C || !C->equalsInt(maskVal)) return false;
  Value *TID = IEI->getOperand(1);
  DenseMap<Value*, Constant *>::iterator strideIt = m_constStrides.find(TID);
  if (strideIt == m_constStrides.end()) return false;
  ConstantInt *stride = dyn_cast<ConstantInt>(strideIt->second);
  if ((!stride) || (!stride->equalsInt(vTy->getNumElements()))) return false;

  // Filling data structres.
  m_stridedIntermediate.insert(IEI);
  m_stridedIntermediate.insert(SVI);
  m_constStrides[I] = stride;
  m_constStrides[IEI] = stride;
  m_constStrides[SVI] = stride;
  return true;
}

void LoopWIAnalysis::ScanLoop(DomTreeNode *N) {
  assert(N != 0 && "Null dominator tree node?");
  BasicBlock *BB = N->getBlock();

  // If this subregion is outside the current loop exit.
  if (!m_curLoop->contains(BB)) return;

  // We don't analyze instruction in sub-loops.
  if (!LoopUtils::inSubLoop(m_curLoop, BB)) {
    // Avoid analyzing original header phi nodes thar were already analyzed.
    BasicBlock::iterator I = BB == m_header ?
                  (BasicBlock::iterator)BB->getFirstNonPHI() : BB->begin();
    for (BasicBlock::iterator E = BB->end(); I != E; I++) calculate_dep(I);
  }

  // Go over blocks recursively according to Dominator tree.
  const std::vector<DomTreeNode*> &Children = N->getChildren();
  for (unsigned i = 0, e = Children.size(); i != e; ++i) ScanLoop(Children[i]);
}

// Same as WIAnalysis.
LoopWIAnalysis::ValDependancy LoopWIAnalysis::calculate_dep(CastInst *CI) {
  Value* op0 = CI->getOperand(0);
  ValDependancy dep0 = getDependency(op0);

  switch (CI->getOpcode())
  {
    case Instruction::SExt:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::UIToFP:
    case Instruction::SIToFP:
      return dep0;
    case Instruction::ZExt:
    case Instruction::BitCast:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
      return LoopWIAnalysis::RANDOM;
    // FIXME:: assumes that the value does not cross the +/- border - risky !!!!
    case Instruction::Trunc: {
      Type *destType = CI->getDestTy();
      if (destType->isVectorTy()) {
          destType = (cast<VectorType>(destType))->getElementType();
      }
      const IntegerType *intType = dyn_cast<IntegerType>(destType);
      if (intType && (intType->getBitWidth() >= MinIndexBitwidthToPreserve)) {
        return  dep0;
      } else {
        return  LoopWIAnalysis::RANDOM;
      }
      break;
    }
    default:
      assert(false && "no such opcode");
      // never get here
  }
  return LoopWIAnalysis::RANDOM;
}

LoopWIAnalysis::ValDependancy
LoopWIAnalysis::calculate_dep(BinaryOperator* BO) {
  // Special treatment for generation of sequential index.
  if (BO->getOpcode() == Instruction::Add && isSequentialVector(BO)) {
    return LoopWIAnalysis::STRIDED;;
  }

  // Calculate the dependency type for each of the operands
  Value *op0 = BO->getOperand(0);
  Value *op1 = BO->getOperand(1);

  ValDependancy dep0 = getDependency(op0);
  ValDependancy dep1 = getDependency(op1);

  switch (BO->getOpcode()) {
    case Instruction::Add:
    case Instruction::FAdd: {
      if (dep0 == LoopWIAnalysis::STRIDED && dep1 == LoopWIAnalysis::UNIFORM) {
        updateConstStride(BO, op0);
      } else if (dep1 == LoopWIAnalysis::STRIDED && dep0 == LoopWIAnalysis::UNIFORM) {
        updateConstStride(BO, op1);
      }
      return add_conversion[dep0][dep1];
    }
    case Instruction::Sub:
    case Instruction::FSub: {
      if (dep0 == LoopWIAnalysis::STRIDED && dep1 == LoopWIAnalysis::UNIFORM) {
        updateConstStride(BO, op0, false);
      } else if (dep1 == LoopWIAnalysis::STRIDED && dep0 == LoopWIAnalysis::UNIFORM) {
        updateConstStride(BO, op1, true);
      }
      return add_conversion[dep0][dep1];
    }
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::Shl:
      return mul_conversion[dep0][dep1];
    default:
      break;
  }
  return LoopWIAnalysis::RANDOM;
}

// On extract element the scalar value has the type of the vector value.
LoopWIAnalysis::ValDependancy
LoopWIAnalysis::calculate_dep(ExtractElementInst *EEI) {
  Value *vectorOp = EEI->getVectorOperand();
  updateConstStride(EEI, vectorOp);
  return getDependency(EEI->getVectorOperand());
}

//
void LoopWIAnalysis::updateConstStride(Value *toUpadte, Value *updateBy, bool negate) {
  DenseMap<Value*, Constant *>::iterator it = m_constStrides.find(updateBy);
  if (it != m_constStrides.end()) {
    //errs() << "\n\nupdate const on:\ntoUpdate = " << *toUpadte <<"\nupdateBy = " << *updateBy << "\n";
    if (updateBy->getType()->isIntOrIntVectorTy()) {
      //errs() << "updateBy consant is:  " << *x << "\n";
      ConstantInt *strideConst = dyn_cast<ConstantInt>(it->second);
      assert(strideConst && "strides are expected to be scalar");
      int64_t stride = strideConst->getSExtValue();
      if (negate) stride = -stride;
      Type *toUpdateTy = toUpadte->getType();
      Constant *newConstStride = NULL;
      if (toUpdateTy->isIntOrIntVectorTy()) {
        newConstStride = ConstantInt::get(toUpdateTy->getScalarType(), stride);
      } else if (toUpdateTy->isFPOrFPVectorTy()) {
        newConstStride = ConstantFP::get(toUpdateTy->getScalarType(), static_cast<double>(stride));
      }
      if (newConstStride) {
        //errs() << "toUpadte consant is:  " << *newConstStride << "\n";
        m_constStrides[toUpadte] = newConstStride;
      } else {
        //errs() << "unsupported constant\n";
      }
    }

  }
}

Constant *LoopWIAnalysis::getConstStride(Value *v) {
  DenseMap<Value*, Constant *>::iterator it = m_constStrides.find(v);
  if (it == m_constStrides.end()) return NULL;
  return it->second;
}

bool LoopWIAnalysis::isUniform(Value *v) {
  // Uniform values might not be calculated on the analysis
  // pre-processing stage, so check the dependency on the fly.
  return getDependency(v) == LoopWIAnalysis::UNIFORM;
}

bool LoopWIAnalysis::isStrided(Value *v) {
  DenseMap<Value*, ValDependancy>::iterator it = m_deps.find(v);
  if (it != m_deps.end()) return it->second == LoopWIAnalysis::STRIDED;
  return false;
}

bool LoopWIAnalysis::isRandom(Value *v) {
  DenseMap<Value*, ValDependancy>::iterator it = m_deps.find(v);
  if (it != m_deps.end())  return it->second == LoopWIAnalysis::RANDOM;
  return true;
}

bool LoopWIAnalysis::isStridedIntermediate(Value *v) {
  return m_stridedIntermediate.count(v);
}

void LoopWIAnalysis::clearValDep(Value *v) {
  m_deps.erase(v);
  m_constStrides.erase(v);
  m_stridedIntermediate.erase(v);
}

void LoopWIAnalysis::setValStrided(Value *v, Constant *constStride) {
  m_deps[v] = LoopWIAnalysis::STRIDED;
  if (constStride) m_constStrides[v] = constStride;
}

}// namespace intel


