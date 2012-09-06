/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  SimplifyGEP.cpp

\*****************************************************************************/

#include "SimplifyGEP.h"
#include "VectorizerUtils.h"
#include "llvm/Support/InstIterator.h"

#include <vector>

namespace intel {

  bool SimplifyGEP::runOnFunction(Function &F) {
    // obtain TagetData of the module
    m_pTD = getAnalysisIfAvailable<TargetData>();

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

    return changed;
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

      V_ASSERT((cast<PointerType>(pPhiNode->getType())->getElementType()->isFloatingPointTy() ||
        cast<PointerType>(pPhiNode->getType())->getElementType()->isIntegerTy()) &&
        "PhiNode type is not primitive!");

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

      std::vector<Value*> phiNodeUsages(pPhiNode->use_begin(), pPhiNode->use_end());
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
    }
    
    return true;
  }

  bool SimplifyGEP::FixMultiIndicesGEP(Function &F) {
    std::vector<GetElementPtrInst *> worklist;

    // Iterate over all instructions and search GEP instructions
    for (inst_iterator ii = inst_begin(F), ie = inst_end(F); ii != ie; ++ii) {
      // searching only GEP instruction with more than 1 index that support simplification
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
    for (unsigned int iter=0; iter<worklist.size(); ++iter) {
      GetElementPtrInst *pGEP = worklist[iter];

      V_ASSERT(pGEP->getPointerOperandIndex() == 0 && "assume Ptr operand is first operand!");

      if (IsUniformSimplifiableGep(pGEP)) {
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
      }
      else if(IsIndexTypeSimplifiableGep(pGEP)) {
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
          if(!m_pTD) {
            // No TargetData cannot apply this SimplifyGEP approach!
            continue;
          }
          unsigned int vectorSize = m_pTD->getTypeAllocSize(baseType);
          unsigned int elementSize = m_pTD->getTypeSizeInBits(cast<VectorType>(baseType)->getElementType()) / 8;
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
      }
      else {
        // Is not simplifiable GEP instruction
      }
    }
    return true;
  }

  bool SimplifyGEP::SimplifiableGep(GetElementPtrInst *pGEP) {
    if(!pGEP || pGEP->getNumIndices() == 1) return false;

    // Support only GEP instructions with
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

  bool SimplifyGEP::IsUniformSimplifiableGep(GetElementPtrInst *pGEP) {
    // Check if all indices except the last one are uniform
    for (unsigned int i=1; i<pGEP->getNumIndices(); ++i) {
      if (WIAnalysis::UNIFORM != m_depAnalysis->whichDepend(pGEP->getOperand(i))) {
        return false;
      }
    }
    return true;
  }

  bool SimplifyGEP::IsIndexTypeSimplifiableGep(GetElementPtrInst *pGEP) {
    Type *firstIndexType = pGEP->getOperand(1)->getType();
    for (unsigned int i=1; i<pGEP->getNumOperands(); ++i) {
      if (firstIndexType != pGEP->getOperand(i)->getType()) {
        return false;
      }
    }
    return true;
  }

  int SimplifyGEP::SimplifiablePhiNode(PHINode *pPhiNode) {
    if(pPhiNode->getNumIncomingValues() != 2 || !pPhiNode->getType()->isPointerTy()) {
      // This is not a supported case for simplifying PhiNode.
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
    if (pGep2 && pGep2->getPointerOperand() == pPhiNode && pGep2->getNumIndices() == 1 && pGep1->hasOneUse()) {
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

char intel::SimplifyGEP::ID = 0;
static RegisterPass<intel::SimplifyGEP>
CLISimplifyGEP("SimplifyGEP", "SimplifyGEP simplify GEP instructions");

