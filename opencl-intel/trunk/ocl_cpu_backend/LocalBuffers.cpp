/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  LocalBuffers.cpp

\*****************************************************************************/

#include "LocalBuffers.h"
#include "CompilationUtils.h"

#include "llvm/Target/TargetData.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char LocalBuffers::ID = 0;

  ModulePass* createLocalBuffersPass() {
    return new LocalBuffers();
  }

  void getKernelInfoMap(ModulePass *pPass, std::map<const Function*, TLLVMKernelInfo>& infoMap) {
    LocalBuffers *pKU = dynamic_cast<LocalBuffers*>(pPass);

    infoMap.clear();
    if ( NULL != pKU ) {
      infoMap.insert(pKU->m_mapKernelInfo.begin(), pKU->m_mapKernelInfo.end());
    }
  }

  bool LocalBuffers::runOnModule(Module &M) {

    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_localBuffersAnalysis = &getAnalysis<LocalBuffersAnalysis>();

    m_mapKernelInfo.clear();
   

    // Run on all defined function in the module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc || pFunc->isDeclaration () ) {
        // Function is not defined inside module
        continue;
      }
      runOnFunction(pFunc);
    }
    return true;
  }

  Instruction* LocalBuffers::CreateInstrFromConstant(Constant *pCE, Value *From, Value *To,
    std::vector<Instruction*> *InstInsert) {  

    Instruction *Replacement = 0;

    if (ConstantExpr *CEx = dyn_cast<ConstantExpr>(pCE)) {
      if ( CEx->getOpcode() == Instruction::GetElementPtr ) {
        SmallVector<Value*, 8> Indices;
        Value *Pointer = pCE->getOperand(0);
        Indices.reserve(pCE->getNumOperands()-1);
        if (Pointer == From) Pointer = To;

        for ( unsigned i = 1, e = pCE->getNumOperands(); i != e; ++i ) {
          Value *Val = CEx->getOperand(i);
          if (Val == From) Val = To;
          Indices.push_back(Val);
        }
        Replacement = GetElementPtrInst::Create(Pointer, Indices.begin(), Indices.end());

      } else if ( CEx->getOpcode() == Instruction::ExtractValue ) {
        Value *Agg = CEx->getOperand(0);
        if (Agg == From) Agg = To;

        const SmallVector<unsigned, 4> &Indices = CEx->getIndices();
        Replacement = ExtractValueInst::Create(Agg,  Indices.begin(), Indices.end());

      } else if (CEx->getOpcode() == Instruction::InsertValue) {
        Value *Agg = CEx->getOperand(0);
        Value *Val = CEx->getOperand(1);
        if (Agg == From) Agg = To;
        if (Val == From) Val = To;

        const SmallVector<unsigned, 4> &Indices = CEx->getIndices();
        Replacement = InsertValueInst::Create(Agg, Val, Indices.begin(), Indices.end());

      } else if (CEx->isCast()) {
        assert(CEx->getOperand(0) == From && "Cast only has one use!");
        Replacement = CastInst::Create((Instruction::CastOps)CEx->getOpcode(), To, CEx->getType());

      } else if (CEx->getOpcode() == Instruction::Select) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        Value *C3 = CEx->getOperand(2);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        if (C3 == From) C3 = To;
        Replacement = SelectInst::Create(C1, C2, C3);

      } else if (CEx->getOpcode() == Instruction::ExtractElement) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        Replacement = ExtractElementInst::Create(C1, C2);

      } else if (CEx->getOpcode() == Instruction::InsertElement) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        Value *C3 = CEx->getOperand(1);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        if (C3 == From) C3 = To;
        Replacement = InsertElementInst::Create(C1, C2, C3);

      } else if (CEx->getOpcode() == Instruction::ShuffleVector) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        Value *C3 = CEx->getOperand(2);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        if (C3 == From) C3 = To;
        Replacement = new ShuffleVectorInst(C1, C2, C3);

      } else if (CEx->isCompare()) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        Replacement = CmpInst::Create((Instruction::OtherOps)CEx->getOpcode(), CEx->getPredicate(), C1, C2);

      } else {
        assert(0 && "Unknown ConstantExpr type!");
        return NULL;
      }

      InstInsert->push_back(Replacement);
      return Replacement;
    }

    if (ConstantVector *CV = dyn_cast<ConstantVector>(pCE)) {

      const VectorType *VT = CV->getType();
      unsigned numElem = VT->getNumElements();

      Value *UpdatedVec = UndefValue::get(CV->getType()); 
      ConstantInt *Idx = NULL;
      Instruction *IE = NULL;
      for (unsigned i = 0; i< numElem; i++) {
        Value *toInsert = NULL;
        if (CV->getOperand(i) == From) {
          toInsert = To;  
        } else {
          toInsert = CV->getOperand(i);
        }
        Idx = ConstantInt::get(IntegerType::get(CV->getContext(), 32) , i);
        IE = InsertElementInst::Create(UpdatedVec, toInsert, Idx, "Insert");
        UpdatedVec = IE;
        InstInsert->push_back(IE);
      }
      assert(IE  && "Unable to find source constant");
      return IE;
    }

    assert(0  && "Unknown constant type");
    return 0;
  }

  bool LocalBuffers::ChangeConstant(Value *pTheValue, Value *pUser, Instruction *pBC, Instruction *Where) {

    // We need substitute constant expression with real instruction
    Constant *pCE = dyn_cast<Constant>(pUser);
    if ( NULL == pCE ) {
      return false;
    }

    std::vector<Instruction*> InstInsert;
    Instruction *pInst = CreateInstrFromConstant(pCE, pTheValue, pBC, &InstInsert);
    //Klocwork add assert to prevent warnings
    assert(pInst && InstInsert.size());
        
    // Change all non-constant references recursively
    std::vector<User*> users(pCE->use_begin(), pCE->use_end());
    for ( std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it ) {
      if ( dyn_cast<Constant>(*it) ) {
        ChangeConstant(pUser, *it, pInst, Where);
      }
      // Check if user is an instruction that belongs to the same function
      else if (Instruction *Inst = dyn_cast<Instruction>(*it)) {
          if (Inst->getParent()->getParent() == Where->getParent()->getParent()) {
              Inst->replaceUsesOfWith(pUser, pInst);
          }
      } else {
        // Currently, the only possible users of llvm::ContantExpr are llvm::Constant, llvm::Instruction and llvm::Operator
        // llvm::Operator is an internal class, so we do not expect it to be a user of ConstantExp
        assert(0 && "Unknown user of constant expression");
      }
    }

    // Check if the instruction was not used
    if ( pInst->use_empty() ) {
       for (std::vector<Instruction*>::iterator it = InstInsert.begin(), e = InstInsert.end(); it != e; ++it) {
         if (!(*it)->getType()->isVoidTy())
          (*it)->replaceAllUsesWith(UndefValue::get((*it)->getType()));
        delete *it;
      }
    }
    // Add instruction to the block, only the first time and only if it has uses  
    else if ( !pInst->getParent() ) {
      for (std::vector<Instruction*>::iterator it = InstInsert.begin(), e = InstInsert.end(); it != e; ++it) {
        (*it)->insertAfter(Where);
        Where = *it;
      }
    }

    if ( !pCE->use_empty() ) {
      return false;
    }

    return true;
  }

  // Substitutes a pointer to local buffer, with argument passed within kernel parameters
  void LocalBuffers::parseLocalBuffers(Function *pFunc, Argument *pLocalMem) {

    LocalBuffersAnalysis::TUsedLocals localsSet = m_localBuffersAnalysis->getDirectLocals(pFunc);

    unsigned int currLocalOffset = 0;
    // Create code for local buffer
    Instruction *pFirstInst = dyn_cast<Instruction>(pFunc->getEntryBlock().begin());

    // Iterate through local buffers
    for ( LocalBuffersAnalysis::TUsedLocals::const_iterator gi = localsSet.begin(),
      ge = localsSet.end(); gi != ge; ++gi ) {
        GlobalValue *pLclBuff = dyn_cast<GlobalValue>(*gi);

        if(NULL == pLclBuff) {
          continue;
        }

        // Calculate required buffer size
        llvm::TargetData TD(m_pModule);
        size_t uiArraySize = TD.getTypeSizeInBits(pLclBuff->getType()->getElementType())/8;
        assert(0 != uiArraySize && "zero array size!");
        // Now retrieve to the offset of the local buffer
        GetElementPtrInst *pLocalAddr =
          GetElementPtrInst::Create(pLocalMem, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), currLocalOffset), "", pFirstInst);
        // Now add bitcast to required/original pointer type
        CastInst *pBitCast = CastInst::Create(Instruction::BitCast, pLocalAddr, pLclBuff->getType(), "", pFirstInst);
        // Advance total implicit size
        currLocalOffset += ADJUST_SIZE_TO_MAXIMUM_ALIGN(uiArraySize);

        std::vector<User*> users(pLclBuff->use_begin(), pLclBuff->use_end());
        for ( std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it ) {
          if (ConstantExpr *pCE = dyn_cast<ConstantExpr>(*it))  {
            ChangeConstant(pLclBuff, pCE, pBitCast, pBitCast);
          }
           // Check if user is an instruction that belongs to the same function
          else if ( Instruction *Inst = dyn_cast<Instruction>(*it) ) {
            if ( Inst->getParent()->getParent() == pFunc ) {
              // pBitCast was already added to a basic block during it's creation
              Inst->replaceUsesOfWith(pLclBuff, pBitCast);
            }
          } else {
            // Currently, the only possible users of llvm::ContantExpr are llvm::Constant, llvm::Instruction and llvm::Contant
            // llvm::Operator is an internal class, so we do not expect it to be a user of ConstantExp
            assert(0 && "Unknown user of constant expression");
          }
        }
    }

    assert( currLocalOffset == m_localBuffersAnalysis->getDirectLocalsSize(pFunc) &&
      "CurrLocalOffset is not equal to local buffer size!" );
  }

  
  void LocalBuffers::runOnFunction(Function *pFunc) {
    // Getting the implicit arguments
    Argument *pLocalMem = 0;
    
    CompilationUtils::getImplicitArgs(pFunc, &pLocalMem, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Apple LLVM-IR workaround
    // 1.  Pass WI information structure as the next parameter after given function parameters
    // 2.  We don't want to use TLS for local memory.
    //    Our solution to move all internal local memory blocks to be allocated
    //    by the execution engine and passed within additional parameters to the kernel,
    //    those parameters are not exposed to the user

    parseLocalBuffers(pFunc, pLocalMem);
    
    m_mapKernelInfo[pFunc].stTotalImplSize = m_localBuffersAnalysis->getLocalsSize(pFunc);
  }

  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
