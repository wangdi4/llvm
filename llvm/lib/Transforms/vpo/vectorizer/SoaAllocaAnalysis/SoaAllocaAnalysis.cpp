/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "SoaAllocaAnalysis.h"
#include "Mangler.h"

#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

namespace intel {

char SoaAllocaAnalysis::ID = 0;
OCL_INITIALIZE_PASS(SoaAllocaAnalysis, "SoaAllocaAnalysis", "SoaAllocaAnalysis provides SOA alloca info regarding", false, false)

  bool SoaAllocaAnalysis::runOnFunction(Function &F) {

    m_allocaSOA.clear();

    // Compute the  first iteration of the WI-dep according to ordering
    // intstructions this ordering is generally good (as it ususally correlates
    // well with dominance).
    for (inst_iterator ii = inst_begin(F), ie = inst_end(F); ii != ie; ++ii) {
      // TODO: can we assume Alloca exists only in first basic-block?
      AllocaInst *pAlloca = dyn_cast<AllocaInst>(&*ii);
      if (!pAlloca) {
        // searching only alloca instructions
        continue;
      }
      Type* allocaType = pAlloca->getAllocatedType();

      unsigned int arrayNestedLevel = 0;
      while (ArrayType *arrayType = dyn_cast<ArrayType>(allocaType)) {
        allocaType = arrayType->getElementType();
        arrayNestedLevel++;
      }
      // At this point allocaType is not an array.

      if (!(allocaType->isIntOrIntVectorTy() || allocaType->isFPOrFPVectorTy())) {
        // Supports only Integer/Float scalar/vector types
        V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported base type (" << pAlloca << ")\n");
        continue;
      }
      bool isVectorBasedType = allocaType->isVectorTy();
      int width = isVectorBasedType ? (dyn_cast<VectorType>(allocaType))->getNumElements() : 0;
      // At this point the alloca type is supported for SOA-alloca
      // Need to check the all the derived usages of the alloca pointer are allowed
      std::set<const Value*> visited;
      if(isSupportedAlloca(pAlloca, isVectorBasedType, arrayNestedLevel, visited)) {
        // We have a alloca instruction that can be converted to SOA-alloca.
        // Mark it and all derived usages to be SOA-alloca related.
        for (std::set<const Value*>::const_iterator vi = visited.begin(),
          ve = visited.end(); vi!=ve; ++vi) {
            V_ASSERT(0 == m_allocaSOA.count(*vi) && "vistid value is already in the alloca list!");
            m_allocaSOA[*vi] = width;
        }
      }
    }

    return false;
  }

  bool SoaAllocaAnalysis::isSoaAllocaRelated(const Value* val) {
    return (0 != m_allocaSOA.count(val));
  }

  bool SoaAllocaAnalysis::isSoaAllocaScalarRelated(const Value* val) {
    return (isSoaAllocaRelated(val) && 0 == m_allocaSOA[val]);
  }

  bool SoaAllocaAnalysis::isSoaAllocaVectorRelated(const Value* val) {
    return (isSoaAllocaRelated(val) && 0 != m_allocaSOA[val]);
  }

  bool SoaAllocaAnalysis::isSoaAllocaRelatedPointer(const Value* val) {
    return (isSoaAllocaRelated(val) && (isa<AllocaInst>(val) || isa<GetElementPtrInst>(val)));
  }

  unsigned int SoaAllocaAnalysis::getSoaAllocaVectorWidth(const Value* val) {
    V_ASSERT(isSoaAllocaVectorRelated(val) &&
      "val is not related to supported soa alloca with vector base type");

    return m_allocaSOA[val];
  }

  void SoaAllocaAnalysis::print(raw_ostream &OS, const Module *M) const {
    if ( !M ) {
      OS << "No Module!\n";
      return;
    }
    //Print Module
    OS << *M;

    OS << "SAA-Start\n";
    for ( Module::const_iterator fi = M->begin(), fe = M->end(); fi != fe; ++fi ) {
      if (fi->isDeclaration()) continue;
      OS << fi->getName().str() << "\n";
      for (const_inst_iterator ii = inst_begin(fi), ie = inst_end(fi); ii != ie; ++ii) {
        const Value* val = &*ii;
        bool  bSoaAllocaRelated = const_cast<SoaAllocaAnalysis*>(this)->isSoaAllocaRelated(val);
        bool  bSoaAllocaScalarRelated = const_cast<SoaAllocaAnalysis*>(this)->isSoaAllocaScalarRelated(val);
        bool  bSoaAllocaVectorRelated = const_cast<SoaAllocaAnalysis*>(this)->isSoaAllocaVectorRelated(val);
        bool  bSoaAllocaRelatedPointer = const_cast<SoaAllocaAnalysis*>(this)->isSoaAllocaRelatedPointer(val);
        if (bSoaAllocaRelated) {
          OS << *val << " SR:[" << bSoaAllocaScalarRelated << "] VR:[" << bSoaAllocaVectorRelated << "] PR:[" << bSoaAllocaRelatedPointer << "]\n";
        }
      }
    }
    OS << "SAA-End\n";
  }

  bool SoaAllocaAnalysis::isSupportedAlloca(const AllocaInst *pAI, bool isVectorBasedType,
      unsigned int arrayNestedLevel, std::set<const Value*> &visited) {
    std::vector<const Value*> usages(pAI->user_begin(), pAI->user_end());
    visited.clear();
    visited.insert(pAI);
    while (!usages.empty()) {
      const Value* usage = usages.back();
      usages.pop_back();
      // Check if usage already visited.
      if (visited.count(usage)) continue;
      // usage was not visited yet, add it to visited list and handle it.
      visited.insert(usage);
      if (const GetElementPtrInst *pGEP = dyn_cast<GetElementPtrInst>(usage)) {
        // one index for pointer + other indices for arrays
        if (pGEP->getNumIndices() <= arrayNestedLevel+1) {
          // These are allowed instructions that result in pointers,
          // so need to check their usages too.
          usages.insert(usages.end(), usage->user_begin(), usage->user_end());
          continue;
        }
        // Cannot support GEP with last index for vector type!
        V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported usage as Gep value (" << *usage << ")\n");
        return false;
      }
      else if (isa<LoadInst>(usage)) {
        // Load is allowed instructions that does not result in a pointer,
        // so only need to continue checking other usages.
        continue;
      }
      else if (const StoreInst *pSI = dyn_cast<StoreInst>(usage)) {
        if (pSI->getValueOperand() == pAI) {
          V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported usage as store value (" << *usage << ")\n");
          return false;
        }
        // Store is allowed instructions that returns void,
        // so only need to continue checking other usages.
        continue;
      }
      else if (const BitCastInst *BC = dyn_cast<BitCastInst>(usage)) {
        // Bitcast on alloca with vector based type is not supported.
        // Only Bitcast of alloca instruction is supported.
        if (!isVectorBasedType && BC->getOperand(0) == pAI) {
          for(Value::const_user_iterator ui = BC->user_begin(), ue = BC->user_end(); ui!= ue; ++ui) {
            // Only BitCastInst with users that are only memset are supported.
            if(!isSupportedMemset(dyn_cast<CallInst>(*ui))) {
              V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported bitcast usage (" << *usage << ")\n");
              return false;
            }
          }
          visited.insert(usage->user_begin(), usage->user_end());
          continue;
        }
        V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported bitcast usage (" << *usage << ")\n");
        return false;
      }
      else {
        if (const CallInst *pCall = dyn_cast<CallInst>(usage)) {
          if (Mangler::isMangledLoad(pCall->getCalledFunction()->getName())) {
            // Load is allowed instructions that does not result in a pointer,
            // so only need to continue checking other usages.
            continue;
          }
          else if (Mangler::isMangledStore(pCall->getCalledFunction()->getName())) {
            if (pCall->getArgOperand(2) == pAI) {
              V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported usage as store value (" << *usage << ")\n");
              return false;
            }
            // Store is allowed instructions that returns void,
            // so only need to continue checking other usages.
            continue;
          }
          else if(isSupportedMemset(pCall)) {
            //Theis is a case where memset is used on SOA-Alloca without Bitcast
            continue;
          }
        }
        // Reaching here means we have usage of unsupported instruction,
        // so no need to continue checking usages, return
        V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported usage (" << *usage << ")\n");
        return false;
      }
    }
    // Reaching here means alloca instruction is SOA supported
    V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca is supported (" << *pAI << ")\n");
    return true;
  }

  bool SoaAllocaAnalysis::isSupportedMemset(const CallInst *CI) {
    if (!CI) {
      return false;
    }
    if (!CI->getCalledFunction()->isIntrinsic() ||
        CI->getCalledFunction()->getIntrinsicID() != Intrinsic::memset) {
      V_PRINT(soa_alloca_stat, "SoaAllocaAnalysis: alloca with unsupported CallInst usage (" << CI << ")\n");
      //It is not a call to memset, so it is not supported.
      return false;
    }
    //V_ASSERT(CI->getNumArgOperands() == 5 && "llvm.memset function does not take 5 arguments!");
    V_ASSERT(CI->getType()->isVoidTy() && "llvm.memset function does not return void!");
    // Need to check that value to set is constant (cannot support non-uniform set value to SOA-Alloca)
    if(!isa<Constant>(CI->getArgOperand(1))) {
      return false;
    }
    return true;
  }
} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createSoaAllocaAnalysisPass() {
    return new intel::SoaAllocaAnalysis();
  }
}

