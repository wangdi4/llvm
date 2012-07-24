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

    for (std::vector<GetElementPtrInst*>::iterator gi = worklist.begin(),
      ge = worklist.end(); gi != ge; ++gi) {
        GetElementPtrInst *pGEP = *gi;

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
            arraySizes.insert(arraySizes.begin(), cast<ArrayType>(baseType)->getNumElements());
            baseType = baseType->getContainedType(0);
          }
          V_ASSERT(baseType->isSingleValueType() && !baseType->isPointerTy() && "assumed primitive base type!");

          if(baseType->isVectorTy()) {
            if(!m_pTD) {
              // No TargetData cannot apply this SimplifyGEP approach!
              continue;
            }
            unsigned int vectorSize = m_pTD->getTypeAllocSize(baseType);
            unsigned int elementSize = m_pTD->getTypeSizeInBits(cast<VectorType>(baseType)->getElementType()) / 8;
            V_ASSERT((vectorSize/elementSize > 0) && (vectorSize % elementSize == 0) &&
              "vector size should be a multiply of element size");
            arraySizes.insert(arraySizes.begin(), vectorSize/elementSize);
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
            Constant *arraySize = ConstantInt::get(index->getType(), arraySizes.back());
            arraySizes.pop_back();
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

