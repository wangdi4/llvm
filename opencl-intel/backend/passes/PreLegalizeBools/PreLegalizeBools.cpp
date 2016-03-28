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
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

namespace intel {

  char PreLegalizeBools::ID=0;

  // Register the pass
  OCL_INITIALIZE_PASS(PreLegalizeBools, "prelegbools", "mitigate boolean vectors legalization issue", false, false)

  PreLegalizeBools::PreLegalizeBools() : ModulePass(ID) {
      initializePreLegalizeBoolsPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool PreLegalizeBools::runOnModule(Module &M) {
    bool changed = false;
    for (Module::iterator funcIter = M.begin(); funcIter != M.end(); ++funcIter) {
        // Ignore declarations.
        if (funcIter->isDeclaration()) continue;
        // TODO: no point to run on non-vectorized kernels
        changed |= runOnFunction(*funcIter);
    }

    // Erase unneeded instructions
    for(std::set<llvm::Instruction *>::iterator iter = m_edgeTruncs.begin(),
                                                 end = m_edgeTruncs.end();
        iter != end; ++iter) {
        if((*iter)->getNumUses() == 0) (*iter)->eraseFromParent();
    }
    // And clear internal resources for a next run
    m_edgeTruncs.clear();
    m_sextCache.clear();
    return changed;
  }


  bool PreLegalizeBools::testSExt(Instruction * pInst) {

    SExtInst * pSExt = dyn_cast<SExtInst>(pInst);
    if(NULL != pSExt &&
       pSExt->getSrcTy()->isVectorTy() &&
       pSExt->getSrcTy()->getScalarType()->isIntegerTy(1) &&
       pSExt->getDestTy()->isVectorTy()) {

      Instruction * pSrcInst = dyn_cast<Instruction>(pSExt->getOperand(0));
      if(!pSrcInst) return false;

      // new trunc <N x Ty> to <N x i1> --> sext <N x i1> to <N x Ty>
      if(pSrcInst->getOpcode() == Instruction::Trunc &&
         m_edgeTruncs.count(pSrcInst)) {
        return pSrcInst->getOperand(0)->getType() == pSExt->getType();
      }
      else {
        // LOGOP/PHI <N x i1> --> sext <N x i1> to <N x Ty>
        // SELECT <N x i1>, <N x i1>, <N x i1>  --> sext <N x i1> to <N x Ty>
        // SHUFFLEVECTOR/INSERTELEMENT --> sext
        return pSrcInst->getOpcode() == Instruction::And ||
               pSrcInst->getOpcode() == Instruction::Or ||
               pSrcInst->getOpcode() == Instruction::Xor ||
               pSrcInst->getOpcode() == Instruction::PHI ||
               pSrcInst->getOpcode() == Instruction::Select ||
               pSrcInst->getOpcode() == Instruction::ShuffleVector ||
               pSrcInst->getOpcode() == Instruction::InsertElement;
      }
    }
    return false;
  }

  void PreLegalizeBools::replace(Instruction * pOldInst, Instruction * pNewInst,
                                 Instruction * pInsertBefore) {
    // Update the debug info
    pNewInst->setDebugLoc(pOldInst->getDebugLoc());
    // Replace all uses of the old Inst by a truncated value of the new Inst
    Instruction * pEdgeTrunc = CastInst::Create(Instruction::Trunc, pNewInst,
                                               pOldInst->getType(),
                                               pOldInst->getName() + ".trunc", pInsertBefore);
    pEdgeTrunc->setDebugLoc(pOldInst->getDebugLoc());
    pOldInst->replaceAllUsesWith(pEdgeTrunc);
    // Update the cache of extended values
    std::map<Value *, Value *> & typedSExtCache = m_sextCache[pNewInst->getType()];
    typedSExtCache[pOldInst] = NULL; // another Value might be allocated later at this address
    typedSExtCache[pEdgeTrunc] = pNewInst;
    // And erase the old instruction
    pOldInst->eraseFromParent();
    // Check later if this Trunc is not needed
    m_edgeTruncs.insert(pEdgeTrunc);
  }

  Value * PreLegalizeBools::makeSExtValue(Value * pVal, Type * pNewTy) {
    assert(pVal->getType()->getScalarType()->isIntegerTy(1) && "source type must be boolean");
    assert(pVal->getType()->getScalarSizeInBits() < pNewTy->getScalarSizeInBits() && "destination type must be wider than source");

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
    else if(isa<UndefValue>(pVal)) {
      // "sext i1 undef to i32" has only two possible values while for example "undef i32"
      // has 2**32 possible values Here pVal might be scalar boolean or vector of booleans,
      // for instance: insertelement <N x i1> undef, i1 undef, i32 0
      // There are the first undef is a vector and the second is a scalar.
      // OpenCL C doesn't support boolean vectors so only scalar undef might come from the original CL code.
      // But that means an unitialized boolean variable in the original CL code is used and
      // the behaviour is undefined.  So it is considered to be safe just to use the "i32 undef"
      // instead of "i1 undef".
      pNewVal = UndefValue::get(pNewTy);
    }
    else if(Constant * pConst = dyn_cast<Constant>(pVal)) {
      if(!pNewTy->isVectorTy()) {
        pNewVal = ConstantInt::getSigned(pNewTy, cast<ConstantInt>(pConst)->getSExtValue());
      }
      else {
        if(pConst->isZeroValue()) {
          pNewVal = ConstantVector::getSplat(pNewTy->getVectorNumElements(),
                                             ConstantInt::getNullValue(pNewTy->getScalarType()));
        }
        else if(pConst->isAllOnesValue()) {
          pNewVal = ConstantVector::getSplat(pNewTy->getVectorNumElements(),
                                             ConstantInt::getAllOnesValue(pNewTy->getScalarType()));
        }
      }
    }
    // Handle instructions
    else if(Instruction * pInst = dyn_cast<Instruction>(pVal)) {
      if(pInst->getOpcode() == Instruction::Trunc &&
         pInst->getOperand(0)->getType() == pNewTy &&
         m_edgeTruncs.count(pInst)) {
        // edge case: new trunc -> sext
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
        if(testSExt(pNewSExt))
          m_workSet.insert(pNewSExt);

        pNewSExt->setDebugLoc(pInst->getDebugLoc());
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

  bool PreLegalizeBools::replaceAllOneZeroCalls(Function &F) {
    // Collect calls
    std::list<CallInst *> callsList;
    for (inst_iterator it = inst_begin(F), e = inst_end(F); it != e; ++it) {
      if(CallInst * pCall = dyn_cast<CallInst>(&*it)) {
        Function * pFunc = pCall->getCalledFunction();
        assert(pFunc && "pFunc is nullptr!");
        // Replace calls to __ocl_allOne, __ocl_allZero functions
        // TODO: By now the Mangler utility is false positive w\ the following kind of names:
        //       "anything__ocl_allOne". Use it once it is fixed.
        if(!pFunc->getName().startswith("__ocl_allOne_v") &&
           !pFunc->getName().startswith("__ocl_allZero_v")) continue;
        // Skip calls w\ i32 vectors
        if(pFunc->getName().endswith("_i32")) continue;
        callsList.push_back(pCall);
      }
    }

    bool changed = false;
    while(!callsList.empty()) {
      CallInst * pOldCall = callsList.front();
      callsList.pop_front();

      Function * pOldFunc = pOldCall->getCalledFunction();
      VectorType * pOldTy = cast<VectorType>(pOldCall->getArgOperand(0)->getType());
      Type * pNewTy = VectorType::get(IntegerType::get(pOldTy->getContext(), 32),
                                                       pOldTy->getNumElements());;
      Value * pNewArgVal = makeSExtValue(pOldCall->getArgOperand(0), pNewTy);
      if(!pNewArgVal) continue;

      // Make a new call
      // Obtain a new function
      assert(pOldFunc && "pOldFunc is nullptr");
      std::string i32Name = Twine(pOldFunc->getName() + "_i32").str();
      Function * pNewFunc = pOldFunc->getParent()->getFunction(i32Name);
      if(!pNewFunc) {
        // Create the i32 function
        FunctionType * pI32FuncTy = FunctionType::get(pOldCall->getType(), pNewTy, false);
        pNewFunc = cast<Function>(pOldFunc->getParent()->getOrInsertFunction(i32Name, pI32FuncTy));
        pNewFunc->setAttributes(pOldFunc->getAttributes());
        pNewFunc->setLinkage(pOldFunc->getLinkage());
        pNewFunc->setCallingConv(pOldFunc->getCallingConv());
      }
      CallInst * pNewCall = CallInst::Create(pNewFunc, pNewArgVal, pOldCall->getName() + ".cast", pOldCall);
      pNewCall->setDebugLoc(pOldCall->getDebugLoc());
      // Erase the old call instruction
      pOldCall->replaceAllUsesWith(pNewCall);
      pOldCall->eraseFromParent();
      changed = true;
    }
    return changed;
  }

  bool PreLegalizeBools::propagateSExt(Function &F) {
    bool changed = false;

    // Collect the patterns to handle
    for (inst_iterator it = inst_begin(F), e = inst_end(F); it != e; ++it) {
      if(testSExt(&*it))
        m_workSet.insert(&*it);
    }
    changed |= m_workSet.size() > 0;

    while(m_workSet.size() > 0) {
      std::set<llvm::Instruction *>::iterator workIter = m_workSet.begin();
      Instruction * pOldSExt = *workIter;
      Value * pOldOp = pOldSExt->getOperand(0);
      m_workSet.erase(workIter);

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
      else if(ShuffleVectorInst * pOldShuffle = dyn_cast<ShuffleVectorInst>(pOldOp)) {
        Value * pNewVecLHS = makeSExtValue(pOldShuffle->getOperand(0), pNewTy);
        Value * pNewVecRHS = makeSExtValue(pOldShuffle->getOperand(1), pNewTy);
        if(!pNewVecLHS || !pNewVecRHS) continue; // give up here to avoid crashes in release
        ShuffleVectorInst * pNewShuffle =
          new ShuffleVectorInst(pNewVecLHS, pNewVecRHS, pOldShuffle->getMask(),
                                pOldShuffle->getName(), pOldShuffle);
        replace(pOldShuffle, pNewShuffle, &*(++BasicBlock::iterator(pNewShuffle)));
        pNewVal = pNewShuffle;
      }
      else if(InsertElementInst * pOldInsert = dyn_cast<InsertElementInst>(pOldOp)) {
        Value * pNewVec = makeSExtValue(pOldInsert->getOperand(0), pNewTy);
        Value * pNewEl = makeSExtValue(pOldInsert->getOperand(1), pNewTy->getScalarType());
        if(!pNewVec || !pNewEl) continue; // give up here to avoid crashes in release
        InsertElementInst * pNewInsert =
          InsertElementInst::Create(pNewVec, pNewEl, pOldInsert->getOperand(2),
                                    pOldInsert->getName(), pOldInsert);
        replace(pOldInsert, pNewInsert, &*(++BasicBlock::iterator(pOldInsert)));
        pNewVal = pNewInsert;
      }
      // Edge case "new trunc" -> "sext"
      else if(TruncInst * pEdgeTrunc = dyn_cast<TruncInst>(pOldOp)) {
        // This trunc was inserted at one of the previous iterations
        assert(pEdgeTrunc->getSrcTy() == pNewTy && m_edgeTruncs.count(pEdgeTrunc) && "algorithm error!");
        pNewVal = m_sextCache[pNewTy][pOldOp] = pEdgeTrunc->getOperand(0);
      }

      assert(NULL != pNewVal && "unrecognized pattern!");
      pOldSExt->replaceAllUsesWith(pNewVal);
      pOldSExt->eraseFromParent();
    }

    return changed;
  }

  bool PreLegalizeBools::runOnFunction(Function &F) {
    bool changed = replaceAllOneZeroCalls(F);
    changed |= propagateSExt(F);

    return changed;
  }
} // end of namespace intel

extern "C"{
  ModulePass* createPreLegalizeBoolsPass() {
    return new intel::PreLegalizeBools();
  }
}
