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

#include "llvm/Support/InstIterator.h"

#include <vector>

namespace intel {

  bool SimplifyGEP::runOnFunction(Function &F) {

    std::vector<GetElementPtrInst *> worklist;

    // Iterate over all instructions and search load/store instructions
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
        Value* pLastIndex = pGEP->getOperand(pGEP->getNumIndices());
        // Replace last index of original GEP instruction with Zero.
        pGEP->setOperand(pGEP->getNumIndices(), Constant::getNullValue(pLastIndex->getType()));
        // Create new GEP instruction with original GEP instruction as pointer
        // and with its old last index as the new GEP instruction only index.
        GetElementPtrInst *pNewGEP = GetElementPtrInst::Create(pGEP, pLastIndex, "simplifiedGEP");
        pNewGEP->insertAfter(pGEP);
        pGEP->replaceAllUsesWith(pNewGEP);
        // workaround as previous function replaced also the pointer of the new GEP instruction!
        pNewGEP->setOperand(pNewGEP->getPointerOperandIndex(), pGEP);
    }
    
    return true;
  }

  bool SimplifyGEP::SimplifiableGep(GetElementPtrInst *pGEP) {
    if(!pGEP || pGEP->getNumIndices() == 1) return false;

    // Support only GEP instructions with
    // pointer operand of type that contains no structures!
    Value* pPtr = pGEP->getPointerOperand();

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

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createSimplifyGEPPass() {
    return new intel::SimplifyGEP();
  }
}

char intel::SimplifyGEP::ID = 0;
static RegisterPass<intel::SimplifyGEP>
CLISimplifyGEP("SimplifyGEP", "SimplifyGEP simplify GEP instructions");

