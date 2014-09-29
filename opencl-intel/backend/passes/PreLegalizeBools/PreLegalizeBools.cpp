/*****************************************************************************\

Copyright (c) Intel Corporation (2014).

  INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
  LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
  ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
  PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
  DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
  PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
  including liability for infringement of any proprietary rights, relating to
  use of the code. No license, express or implied, by estoppels or otherwise,
  to any intellectual property rights is granted herein.

File Name:  PreLegalizeBools.cpp

\*****************************************************************************/
#define DEBUG_TYPE "PreLegalizeBools"

#include "PreLegalizeBools.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

namespace intel {

  char PreLegalizeBools::ID=0;

  // Register the pass
  OCL_INITIALIZE_PASS(PreLegalizeBools, "prelegbools", "mitigate boolean vectors legalization issue", false, false)

  PreLegalizeBools::PreLegalizeBools() : FunctionPass(ID) {
      initializePreLegalizeBoolsPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool PreLegalizeBools::testSExt(Instruction * pInst) {

    SExtInst * pSExt = dyn_cast<SExtInst>(pInst);
    if(NULL != pSExt &&
       pSExt->getSrcTy()->isVectorTy() &&
       pSExt->getSrcTy()->getScalarType()->isIntegerTy(1) &&
       pSExt->getDestTy()->isVectorTy()) {

      Instruction * pSrcInst = dyn_cast<Instruction>(pSExt->getOperand(0));
      if(!pSrcInst) return false;

      // trunc <N x Ty> to <N x i1> --> sext <N x i1> to <N x Ty>
      if(pSrcInst->getOpcode() == Instruction::Trunc) {
        return isa<Instruction>(pSrcInst->getOperand(0)) &&
               pSrcInst->getOperand(0)->getType() == pSExt->getType();
      }
      else {
        // LOGOP/PHI <N x i1> --> sext <N x i1> to <N x Ty>
        // SELECT <N x i1>, <N x i1>, <N x i1>  --> sext <N x i1> to <N x Ty>
        return pSrcInst->getOpcode() == Instruction::And ||
               pSrcInst->getOpcode() == Instruction::Or ||
               pSrcInst->getOpcode() == Instruction::Xor ||
               pSrcInst->getOpcode() == Instruction::PHI ||
               pSrcInst->getOpcode() == Instruction::Select;
      }
    }
    return false;
  }

  void PreLegalizeBools::replace(Instruction * pOldInst, Instruction * pNewInst,
                                 Instruction * pInsertBefore) {
    // Replace all uses of the old Inst by a truncated value of the new Inst
    Instruction * pNewTrunc = CastInst::Create(Instruction::Trunc, pNewInst,
                                               pOldInst->getType(),
                                               pOldInst->getName() + ".trunc", pInsertBefore);
    pOldInst->replaceAllUsesWith(pNewTrunc);
    // Update the cache of extended values
    std::map<Value *, Value *> & typedSExtCache = m_sextCache[pNewInst->getType()];
    typedSExtCache[pOldInst] = NULL; // another Value might be allocated later at this address
    typedSExtCache[pNewTrunc] = pNewInst;
    // And erase the old instruction
    pOldInst->eraseFromParent();
    // Check later if this Trunc is not needed
    m_pendingErase.insert(pNewTrunc);
  }

  Value * PreLegalizeBools::makeSExtValue(Value * pVal, Type * pNewTy) {
    assert(isa<VectorType>(pNewTy) && "the new type must be a vector type");

    // Note the reference to pointer
    Value *& pNewVal = m_sextCache[pNewTy][pVal];
    if(NULL != pNewVal) return pNewVal;

    // Handle constants
    if(ConstantVector * pConstVec = dyn_cast<ConstantVector>(pVal)) {
      // It must be a vector of equal i1 constants
      ConstantInt * pOldConstInt = cast<ConstantInt>(pConstVec->getSplatValue());
      ConstantInt * pNewConstInt = ConstantInt::getSigned(cast<IntegerType>(pNewTy->getScalarType()),
                                                          pOldConstInt->getSExtValue());
      pNewVal = ConstantVector::getSplat(pNewTy->getVectorNumElements(), pNewConstInt);
    }
    else if(Constant * pConst = dyn_cast<Constant>(pVal)) {
      if(pConst->isZeroValue()) {
        pNewVal = ConstantVector::getSplat(pNewTy->getVectorNumElements(),
                                                ConstantInt::getNullValue(pNewTy->getScalarType()));
      }
      else if(pConst->isAllOnesValue()) {
        pNewVal = ConstantVector::getSplat(pNewTy->getVectorNumElements(),
                                                ConstantInt::getAllOnesValue(pNewTy->getScalarType()));
      }
    }
    // Handle instructions
    else if(Instruction * pInst = dyn_cast<Instruction>(pVal)) {
      if(pInst->getOpcode() == Instruction::Trunc &&
         pInst->getOperand(0)->getType() == pNewTy) {
        // Check later if this Trunc is not needed
        m_pendingErase.insert(pInst);
        pNewVal = pInst->getOperand(0);
      }
      else {
        Instruction * pNewSExt = CastInst::Create(Instruction::SExt, pInst, pNewTy,
                                                  pInst->getName() + ".sext");
        if(pInst->getOpcode() != Instruction::PHI)
          pNewSExt->insertAfter(pInst);
        else
          pNewSExt->insertBefore(pInst->getParent()->getFirstNonPHI());

        // Check if this new SExt is to be handled further
        if(testSExt(pNewSExt)) m_worklist.push_back(pNewSExt);

        pNewVal = pNewSExt;
      }
    }
    // Handle function arguments
    else if(Argument * pArg = dyn_cast<Argument>(pVal)) {
      pNewVal = CastInst::Create(Instruction::SExt, pArg, pNewTy,
                                 pArg->getName() + ".sext",
                                 pArg->getParent()->getEntryBlock().getFirstNonPHI());
    }

    assert(NULL != pNewVal && "unrecognized pattern!");
    return pNewVal;
  }

  bool PreLegalizeBools::runOnFunction(Function &F) {
    // TODO: no point to run on non-vectorized kernels

    // Collect the patterns to handle
    for (inst_iterator it = inst_begin(F), e = inst_end(F); it != e; ++it) {
      if(testSExt(&*it)) m_worklist.push_back(&*it);
    }

    bool changed = m_worklist.size() > 0;

    while(m_worklist.size() > 0) {
      Instruction * pOldSExt = m_worklist.front();
      Value * pOldOp = pOldSExt->getOperand(0);
      m_worklist.pop_front();

      Value *  pNewVal = NULL;
      Type * pNewTy = pOldSExt->getType();

      if(PHINode * pOldPHI = dyn_cast<PHINode>(pOldOp)) {
        // Create a new PHI node and insert before the old PHI
        PHINode * pNewPHI = PHINode::Create(pNewTy, pOldPHI->getNumIncomingValues(),
                                            pOldPHI->getName() + ".sext", pOldPHI);
        // By extending its incoming values
        for (unsigned int incN = 0; incN < pOldPHI->getNumIncomingValues(); ++incN) {
          Value * pNewIncVal = makeSExtValue(pOldPHI->getIncomingValue(incN), pNewTy);
          if(!pNewIncVal) {
            // give up here to avoid crashes in release
            pNewPHI->eraseFromParent();
            pNewPHI = NULL;
            break;
          }
          pNewPHI->addIncoming(pNewIncVal, pOldPHI->getIncomingBlock(incN));
        }
        if(!pNewPHI) continue;

        replace(pOldPHI, pNewPHI, pNewPHI->getParent()->getFirstNonPHI());
        pNewVal = pNewPHI;
      }
      else if(BinaryOperator * pOldBOp = dyn_cast<BinaryOperator>(pOldOp)) {
        Value * pNewOp0 = makeSExtValue(pOldBOp->getOperand(0), pNewTy);
        Value * pNewOp1 = makeSExtValue(pOldBOp->getOperand(1), pNewTy);
        if(!pNewOp0 || !pNewOp1 ) continue; // give up here to avoid crashes in release
        BinaryOperator * pNewBOp =
          BinaryOperator::Create(pOldBOp->getOpcode(),
                                 pNewOp0,
                                 pNewOp1,
                                 pOldBOp->getName(), pOldBOp);
        replace(pOldBOp, pNewBOp, &*(++BasicBlock::iterator(pNewBOp)));
        pNewVal = pNewBOp;
      }
      else if(SelectInst * pOldSelect = dyn_cast<SelectInst>(pOldOp)) {
        Value * pNewTrueVal = makeSExtValue(pOldSelect->getTrueValue(), pNewTy);
        Value * pNewFalseVal = makeSExtValue(pOldSelect->getFalseValue(), pNewTy);
        if(!pNewTrueVal || !pNewFalseVal) continue; // give up here to avoid crashes in release
        SelectInst * pNewSelect = SelectInst::Create(pOldSelect->getCondition(),
                                                     pNewTrueVal, pNewFalseVal,
                                                     pOldSelect->getName(), pOldSelect);
        replace(pOldSelect, pNewSelect, &*(++BasicBlock::iterator(pNewSelect)));
        pNewVal = pNewSelect;

      }
      else if(TruncInst * pOldTrunc = dyn_cast<TruncInst>(pOldOp)) {
        // In the most cases this trunc was inserted at one of the previous iterations
        assert(pOldTrunc->getSrcTy() == pNewTy && "algorithm error!");
        if(pOldTrunc->getSrcTy() != pNewTy) continue; // give up here to avoid crashes in release

        pNewVal = m_sextCache[pNewTy][pOldOp] = pOldTrunc->getOperand(0);
        // Check later if this Trunc is not needed
        m_pendingErase.insert(pOldTrunc);
      }

      assert(NULL != pNewVal && "unrecognized pattern!");
      pOldSExt->replaceAllUsesWith(pNewVal);
      pOldSExt->eraseFromParent();
    }

    // Erase unneeded instructions
    for(std::set<llvm::Instruction *>::iterator iter = m_pendingErase.begin(),
                                                 end = m_pendingErase.end();
        iter != end; ++iter) {
        if((*iter)->getNumUses() == 0) (*iter)->eraseFromParent();
    }
    // And clear internal resources for a next run
    m_pendingErase.clear();
    m_sextCache.clear();

    return changed;
  }
} // end of namespace intel

extern "C"{
  FunctionPass* createPreLegalizeBoolsPass() {
    return new intel::PreLegalizeBools();
  }
}
