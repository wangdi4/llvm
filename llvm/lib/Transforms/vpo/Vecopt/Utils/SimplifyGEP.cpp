/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "Vectorizer"

#include "SimplifyGEP.h"
#include "VectorizerUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include <vector>

namespace intel {

char SimplifyGEP::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(SimplifyGEP, "SimplifyGEP", "SimplifyGEP simplify GEP instructions", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(WIAnalysis)
OCL_INITIALIZE_PASS_END(SimplifyGEP, "SimplifyGEP", "SimplifyGEP simplify GEP instructions", false, false)

  SimplifyGEP::SimplifyGEP() : FunctionPass(ID),
    OCLSTAT_INIT(Simplified_Multi_Indices_GEPs,
                "Simplified multi indices GEP instructions",
                m_kernelStats),
    OCLSTAT_INIT(Simplified_Phi_Node_GEPs,
                "Simplified Phi GEP instructions",
                m_kernelStats)
  {
    initializeSimplifyGEPPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool SimplifyGEP::runOnFunction(Function &F) {
    // obtain TagetData of the module
    DataLayoutPass* dlp = getAnalysisIfAvailable<DataLayoutPass>();
    m_pDL = dlp ? &(dlp->getDataLayout()) : NULL;

    // Obtain WIAnalysis of the function
    m_depAnalysis = &getAnalysis<WIAnalysis>();
    V_ASSERT(m_depAnalysis && "Unable to get pass");

    bool changed = false;
    changed = FixPhiNodeGEP(F) || changed;
    // This is a work around to fix WIAnaylsis after changing the function!
    // TODO: need to fix the analysis without regenrating all of it!
    if(changed) {
      m_depAnalysis->runOnFunction(F);
    }
    changed = FixMultiIndicesGEP(F) || changed;

    intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE);
    return changed;
  }

  static bool isPhiPtrToPrimitive(const PHINode* pPhiNode) {
    V_ASSERT(pPhiNode && "Got a null argument");
    PointerType *PT = dyn_cast<PointerType>(pPhiNode->getType());
    return PT && (PT->getElementType()->isFloatingPointTy() ||
           PT->getElementType()->isIntegerTy());
  }

  bool SimplifyGEP::FixPhiNodeGEP(Function &F) {
    std::vector< std::pair<PHINode *, unsigned int> > worklist;

    // Iterate over all instructions and search PhiNode instructions
    for(Function::iterator bi = F.begin(), be = F.end(); bi != be; ++bi) {
      BasicBlock *pBB = dyn_cast<BasicBlock>(bi);
      for (BasicBlock::iterator ii = pBB->begin(), ie = pBB->getFirstNonPHI(); ii != ie; ++ii) {
        // searching only PhiNode instruction (inside loops) with GEP entries.
        PHINode *pPhiNode = dyn_cast<PHINode>(&*ii);
        V_ASSERT(pPhiNode && "Reached non PHINode, should exit the for before this happens!");
        int entry = SimplifiablePhiNode(pPhiNode);
        if ( entry < 0 ) {
          // Found non Simplifiable PhiNode instruction.
          continue;
        }
        worklist.push_back(std::pair<PHINode *, unsigned int>(pPhiNode, entry));
      }
    }

    if ( worklist.empty() ) {
      // No simplifiable GEP instruction function was not changed
      return false;
    }

    for (unsigned int iter=0; iter<worklist.size(); ++iter) {
      PHINode *pPhiNode = worklist[iter].first;
      unsigned int iterEntry = worklist[iter].second;
      unsigned int initEntry = 1-iterEntry;

      V_ASSERT(isPhiPtrToPrimitive(pPhiNode) &&
        "PhiNode type is not a pointer to a primitive!");

      // pInitialValue - the initial value of the PhiNode.
      // pIterValue - the iteration value of the PhiNode.
      GetElementPtrInst *pIterValue = cast<GetElementPtrInst>(pPhiNode->getIncomingValue(iterEntry));
      Value *pInitialValue = pPhiNode->getIncomingValue(initEntry);

      // index type is choosen to be the type of the index iteration step.
      // type of all indices involved in the index calculation should be equal to this one.
      Type *indexType = pIterValue->getOperand(1)->getType();

      // If initialValue is uniform then: initial index = 0, base = pInitialValue
      // Else initialValue is a Gep Inst: initial index = Gep:index, base = Gep:base
      Value *pNewBase = NULL;
      Value *pNewInitialValue = NULL;
      if (WIAnalysis::UNIFORM == m_depAnalysis->whichDepend(pInitialValue)) {
        pNewBase = pInitialValue;
        pNewInitialValue = Constant::getNullValue(indexType);
      }
      else {
        GetElementPtrInst *pInitialValueGep = dyn_cast<GetElementPtrInst>(pInitialValue);
        if (pInitialValueGep && pInitialValueGep->getNumIndices() == 1 &&
          WIAnalysis::UNIFORM == m_depAnalysis->whichDepend(pInitialValueGep->getPointerOperand()) &&
          pInitialValueGep->getOperand(1)->getType() == indexType) {
            pNewBase = pInitialValueGep->getPointerOperand();
            pNewInitialValue = pInitialValueGep->getOperand(1); //operand-1 is first index.
        }
        else {
          // This is not a supported case for simplifying PhiNode.
          continue;
        }
      }

      // Create new index PhiNode
      PHINode *pNewPhiNode = PHINode::Create(indexType, 2, "IndexPhiNode", pPhiNode);
      // Create new index IterValue
      Value *pNewIterValue = BinaryOperator::CreateAdd(pNewPhiNode, pIterValue->getOperand(1), "IndexIterValue", pIterValue);
      // Initialize new Index PhiNode entries
      pNewPhiNode->addIncoming(pNewInitialValue, pPhiNode->getIncomingBlock(initEntry));
      pNewPhiNode->addIncoming(pNewIterValue, pPhiNode->getIncomingBlock(iterEntry));

      // Create new Gep instruction just after the PhiNode
      GetElementPtrInst *pNewIndexGep = GetElementPtrInst::Create(pNewBase, pNewPhiNode, "IndexPhiNodeGEP", pPhiNode->getParent()->getFirstNonPHI());

      // Remove old PhiNode entries, need to do that before removing iterValue
      // But, should not renove PhiNode yet!
      while (pPhiNode->getNumIncomingValues() != 0) {
        pPhiNode->removeIncomingValue((unsigned int)0, false);
      }
      // Now remove old iterValue, as we assure that its only usage was the old PhiNode.
      pIterValue->eraseFromParent();

      std::vector<Value*> phiNodeUsages(pPhiNode->user_begin(), pPhiNode->user_end());
      for (std::vector<Value*>::iterator ui = phiNodeUsages.begin(), ue = phiNodeUsages.end(); ui != ue; ++ui) {
        GetElementPtrInst *pOldGep = dyn_cast<GetElementPtrInst>(*ui);
        if (pOldGep) {
          // If uses is a GEP instruction: oldGep = GEP(pPhiNode, index)
          // Modify it and create new GEP as follows:
          //  oldGep = GEP(pNewBase, index)
          //  newGep = GEP(oldGep, pNewPhiNode)
          pOldGep->replaceUsesOfWith(pPhiNode, pNewBase);
          GetElementPtrInst *pNewGep = GetElementPtrInst::Create(pOldGep, pNewPhiNode, "IndexNewGEP");
          pNewGep->insertAfter(pOldGep);
          pOldGep->replaceAllUsesWith(pNewGep);
          // Now need to reset the base address of the new GEP to be pOldGep
          // (This is a WA: because we replaced it with pNewGep in previous line)!
          pNewGep->setOperand(pNewGep->getPointerOperandIndex(), pOldGep);
          continue;
        }
        // Otherwise, just replace the pPhiNode with the new pIndexGep
        Instruction *pUsage = dyn_cast<Instruction>(*ui);
        V_ASSERT(pUsage && "usage of PhiNode is not an instruction!");
        pUsage->replaceUsesOfWith(pPhiNode, pNewIndexGep);
      }

      // Now remove old PhiNode, as we replaced all its uses
      pPhiNode->eraseFromParent();
      Simplified_Phi_Node_GEPs++;
    }

    return true;
  }

  bool SimplifyGEP::FixMultiIndicesGEP(Function &F) {
    std::vector<GetElementPtrInst *> worklist;

    // Iterate over all instructions and search GEP instructions
    for (inst_iterator ii = inst_begin(F), ie = inst_end(F); ii != ie; ++ii) {
      // searching only GEP instruction that support simplification
      GetElementPtrInst *pGEP = dyn_cast<GetElementPtrInst>(&*ii);
      if ( SimplifiableGep(pGEP) ) {
        // Found Simplifiable GEP instruction.
        worklist.push_back(pGEP);
      }
    }

    if ( worklist.empty() ) {
      // No simplifiable GEP instruction function was not changed
      return false;
    }

    uint32_t simplifiedGEPs = 0;
    for (unsigned int iter=0; iter<worklist.size(); ++iter) {
      GetElementPtrInst *pGEP = worklist[iter];

      V_ASSERT(pGEP->getPointerOperandIndex() == 0 && "assume Ptr operand is the first operand!");

      if(pGEP->getNumIndices() == 1) {
        if(SimplifyIndexSumGep(pGEP)) ++simplifiedGEPs;
      }
      else if(SimplifyUniformGep(pGEP) || SimplifyIndexTypeGep(pGEP)) {
        ++simplifiedGEPs;
      }
    }

    Simplified_Multi_Indices_GEPs = simplifiedGEPs;
    return simplifiedGEPs > 0;
  }


  namespace {
    // Helper function of SimplifyGEP::SimplifyIndexSumGep
    inline Value * makeIndexSum(SmallVector<Value *, 8> & pIndices, GetElementPtrInst * pGEP,
                                char const* prefix) {
      typedef SmallVector<Value *, 8>::size_type size_type;

      Value * ret = pIndices[0];
      for(size_type i = 1; i < pIndices.size(); ++i) {
        Value * pLHS = ret;
        Value * pRHS = pIndices[i];
        V_ASSERT(pLHS->getType()->getScalarType()->isIntegerTy() &&
                 pRHS->getType()->getScalarType()->isIntegerTy() && "index of non-integer type");

        // SExt one of the operands if they are of different width.
        // Note that unsigned integer overflow is defined so never ZExt the operands.
        if(pLHS->getType()->getScalarSizeInBits() < pRHS->getType()->getScalarSizeInBits()) {
          pLHS = CastInst::Create(Instruction::SExt, pLHS, pRHS->getType(), "sext." + pLHS->getName(), pGEP);
        }
        else if(pRHS->getType()->getScalarSizeInBits() < pLHS->getType()->getScalarSizeInBits()) {
          pRHS = CastInst::Create(Instruction::SExt, pRHS, pLHS->getType(), "sext." + pRHS->getName(), pGEP);
        }

        BinaryOperator * newAdd = BinaryOperator::CreateAdd(pLHS, pRHS, prefix, pGEP);
        if(Instruction * pRHSInst = dyn_cast<Instruction>(pRHS)) {
          VectorizerUtils::SetDebugLocBy(newAdd, pRHSInst);
        }
        ret = newAdd;
      }
      return ret;
    }

    inline BinaryOperator * getNextAdd(Value * pVal) {
       Instruction * pInst = dyn_cast<Instruction>(pVal);
       if(!pInst) return NULL;

       if(pInst->getOpcode() == Instruction::Add) {
         return cast<BinaryOperator>(pInst);
       }
       else if(pInst->getOpcode() == Instruction::SExt) {
         // The behaviour of signed overflow of integers that are smaller than sizeof(int)
         // is defined by C99. They are to be promoted to int and the result is to be truncated.
         // It means what it is not safe to split such sum.
         Type * pValType = pInst->getOperand(0)->getType();
         if(pValType->getScalarSizeInBits() < 32) return NULL;
         // Otherwise the signed overflow behaviour is undefined and this sum can be split further.
         BinaryOperator * pBOp = dyn_cast<BinaryOperator>(pInst->getOperand(0));
         return pBOp && pBOp->getOpcode() == Instruction::Add ? pBOp : NULL;
       }
       return NULL;
    }
  } // end of the anonymous namespace

  bool SimplifyGEP::SimplifiableGep(GetElementPtrInst *pGEP) {
    if(!pGEP) return false;

    if(pGEP->getNumIndices() == 1) {
       // Support GEP instructions with a single index which is a sum of uniform and divergent
       // values or a SExt from such sum.
       BinaryOperator * pAddInst = getNextAdd(pGEP->getOperand(1));
       if(pAddInst && WIAnalysis::UNIFORM != m_depAnalysis->whichDepend(pAddInst)) return true;
       return false;
    }
    // Support GEP instructions with
    // pointer operand of type that contains no structures!
    Value *pPtr = pGEP->getPointerOperand();

    Type *type = pPtr->getType();
    if(type->isPointerTy()) {
      type = type->getContainedType(0);
    }
    while(type->isArrayTy()) {
      type = type->getContainedType(0);
    }
    if(type->isSingleValueType() && !type->isPointerTy()) {
      return true;
    }
    return false;
  }

  bool SimplifyGEP::SimplifyIndexSumGep(GetElementPtrInst *pGEP) {
    // Check this is a GEP with a sum of indices and some of them are
    // uniform. If yes make two GEPs where one is a GEP with uniform indices and
    // another one is a GEP with divergent indices.

    // Check preconditions of this transformation
    // 1. The index is the ADD instruction
    // 2. The base pointer is uniform
    // 3. The sum of indices is divergent
    Value * pBasePtrVal = pGEP->getOperand(0);
    BinaryOperator * pAddInst = getNextAdd(pGEP->getOperand(1));
    if(pAddInst &&
       WIAnalysis::UNIFORM == m_depAnalysis->whichDepend(pBasePtrVal) &&
       WIAnalysis::UNIFORM != m_depAnalysis->whichDepend(pAddInst)) {

      // Collect uniform and divergent indices
      SmallVector<Value *, 8>        uniformVals, divergentVals;
      SmallVector<Instruction *, 16> worklist;
      worklist.push_back(pAddInst);

      while(worklist.size() != 0) {
        Value * operands[2] = { worklist.back()->getOperand(0),
                                worklist.back()->getOperand(1) };
        worklist.pop_back();

        for(unsigned i = 0; i < 2; ++i) {
          Value * pVal = operands[i];
          if(WIAnalysis::UNIFORM == m_depAnalysis->whichDepend(pVal)) {
            uniformVals.push_back(pVal);
          }
          else {
            // If an index is divergent that doesn't mean all its components are divergent
            BinaryOperator * pNextAdd = getNextAdd(pVal);
            if(pNextAdd) {
              worklist.push_back(pNextAdd);
            } else {
              divergentVals.push_back(pVal);
            }
          }
        }
      }

      // Leave this GEP as is if there are no uniform indices detected
      if(uniformVals.empty())
        return false;

      // Make uniform and divergent indices
      Value * uniformIdx   = makeIndexSum(uniformVals, pGEP, "uniformIdx");
      Value * divergentIdx = makeIndexSum(divergentVals, pGEP, "divergentIdx");

      // Create new GEP instructions. The first one with the uniform index
      // which is used as a base pointer of the second GEP with divergent index
      GetElementPtrInst * pUniformGEP   = GetElementPtrInst::Create(pGEP->getOperand(0), uniformIdx,
                                                                    "uniformGEP", pGEP);
      GetElementPtrInst * pDivergentGEP = GetElementPtrInst::Create(pUniformGEP, divergentIdx,
                                                                    "divergentGEP", pGEP);

      pUniformGEP->setIsInBounds(pGEP->isInBounds());
      pDivergentGEP->setIsInBounds(pGEP->isInBounds());
      // Update the debug information
      VectorizerUtils::SetDebugLocBy(pUniformGEP, pGEP);
      VectorizerUtils::SetDebugLocBy(pDivergentGEP, pGEP);

      pGEP->replaceAllUsesWith(pDivergentGEP);
      pGEP->eraseFromParent();
      return true;
    }
    return false;
  }

  bool SimplifyGEP::SimplifyUniformGep(GetElementPtrInst *pGEP) {

    // Check if all indices except the last one are uniform
    for (unsigned int i = 1; i < pGEP->getNumIndices(); ++i) {
      if (WIAnalysis::UNIFORM != m_depAnalysis->whichDepend(pGEP->getOperand(i))) {
        return false;
      }
    }

    Value *pLastIndex = pGEP->getOperand(pGEP->getNumIndices());
    // Replace last index of original GEP instruction with Zero.
    pGEP->setOperand(pGEP->getNumIndices(), Constant::getNullValue(pLastIndex->getType()));
    // Create new GEP instruction with original GEP instruction as pointer
    // and with its old last index as the new GEP instruction only index.
    GetElementPtrInst *pNewGEP = GetElementPtrInst::Create(pGEP, pLastIndex, "simplifiedGEP");
    VectorizerUtils::SetDebugLocBy(pNewGEP, pGEP);
    pNewGEP->insertAfter(pGEP);
    pGEP->replaceAllUsesWith(pNewGEP);
    // workaround as previous function replaced also the pointer of the new GEP instruction!
    pNewGEP->setOperand(pNewGEP->getPointerOperandIndex(), pGEP);
    return true;
  }

  bool SimplifyGEP::SimplifyIndexTypeGep(GetElementPtrInst *pGEP) {

    Type *firstIndexType = pGEP->getOperand(1)->getType();
    for (unsigned int i = 1; i < pGEP->getNumOperands(); ++i) {
      if (firstIndexType != pGEP->getOperand(i)->getType()) {
        return false;
      }
    }
    // bitcast base address value to pointer of base type
    // calculate all indices into one index by multiplying array sizes with indices
    //[A x [B X [C X type]]]* GEP base, x, a, b, c ==> ((x*A + a)*B + b)*C + c
    Type *baseType = pGEP->getPointerOperand()->getType();

    V_ASSERT(baseType->isPointerTy() && "type of base address of GEP assumed to be a pointer");
    baseType = baseType->getContainedType(0);

    std::vector<unsigned int> arraySizes;
    while (baseType->isArrayTy()) {
      arraySizes.push_back(cast<ArrayType>(baseType)->getNumElements());
      baseType = baseType->getContainedType(0);
    }
    V_ASSERT((baseType->isIntOrIntVectorTy() || baseType->isFPOrFPVectorTy()) && "assumed primitive base type!");

    if(baseType->isVectorTy()) {
      if(!m_pDL) {
        // No DataLayout cannot apply this SimplifyGEP approach!
        return false;
      }
      unsigned int vectorSize = m_pDL->getTypeAllocSize(baseType);
      unsigned int elementSize = m_pDL->getTypeAllocSize(cast<VectorType>(baseType)->getElementType());
      V_ASSERT((vectorSize/elementSize > 0) && (vectorSize % elementSize == 0) &&
        "vector size should be a multiply of element size");
      arraySizes.push_back(vectorSize/elementSize);
    }

    Value *newIndex = NULL;
    for (unsigned int i=1; i<pGEP->getNumOperands()-1; ++i) {
      Value *index = pGEP->getOperand(i);
      if (newIndex) {
        Instruction *addIndex =
          BinaryOperator::CreateNUWAdd(newIndex, index, "addIndex", pGEP);
        VectorizerUtils::SetDebugLocBy(addIndex, pGEP);
        newIndex = addIndex;
      }
      else {
        newIndex = index;
      }
      Constant *arraySize = ConstantInt::get(index->getType(), arraySizes[i-1]);
      Instruction *mulIndex =
        BinaryOperator::CreateNUWMul(newIndex, arraySize, "mulIndex", pGEP);
      VectorizerUtils::SetDebugLocBy(mulIndex, pGEP);
      newIndex = mulIndex;
    }
    // Add last Index
    Value *index = pGEP->getOperand(pGEP->getNumOperands()-1);
    if (newIndex) {
      Instruction *addIndex =
          BinaryOperator::CreateNUWAdd(newIndex, index, "addIndex", pGEP);
      VectorizerUtils::SetDebugLocBy(addIndex, pGEP);
      newIndex = addIndex;
    }
    else {
      newIndex = index;
    }
    V_ASSERT(newIndex && "new calculated index should not be NULL");
    Value* newBase = pGEP->getPointerOperand();
    newBase = new BitCastInst(newBase, pGEP->getType(), "ptrTypeCast", pGEP);
    GetElementPtrInst *pNewGEP = GetElementPtrInst::Create(newBase, newIndex, "simplifiedGEP", pGEP);
    VectorizerUtils::SetDebugLocBy(pNewGEP, pGEP);
    pGEP->replaceAllUsesWith(pNewGEP);
    pGEP->eraseFromParent();

    SimplifyIndexSumGep(pNewGEP);
    return true;
  }

  int SimplifyGEP::SimplifiablePhiNode(PHINode *pPhiNode) {
    // This is not a supported case for simplifying PhiNode.
    if (pPhiNode->getNumIncomingValues() != 2 || !isPhiPtrToPrimitive(pPhiNode)) {
      return -1;
    }
    // Now only need to check that one of the entries is a GEP with the PhiNode as its base.
    GetElementPtrInst *pGep1 = dyn_cast<GetElementPtrInst>(pPhiNode->getIncomingValue(0));
    GetElementPtrInst *pGep2 = dyn_cast<GetElementPtrInst>(pPhiNode->getIncomingValue(1));

    // Supported simplifying PhiNode should apply the following:
    //   1. Has only two entries
    //   2. Its value is a pointer type
    //   3. One of its value should be a Gep instruction with the PhiNode as its base
    //   4. The Gep instruction should have one index
    //   5. The Gep instruction should have one usage (the PhiNode itself)
    // This is not a supported case for simplifying PhiNode.
    if (pGep1 && pGep1->getPointerOperand() == pPhiNode && pGep1->getNumIndices() == 1 && pGep1->hasOneUse()) {
      return 0;
    }
    if (pGep2 && pGep2->getPointerOperand() == pPhiNode && pGep2->getNumIndices() == 1 && pGep2->hasOneUse()) {
      return 1;
    }
    // This is not a supported case for simplifying PhiNode.
    return -1;
  }

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createSimplifyGEPPass() {
    return new intel::SimplifyGEP();
  }
}


