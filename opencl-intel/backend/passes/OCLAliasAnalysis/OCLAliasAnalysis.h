/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __OCL_ALIAS_ANALYSIS_H__
#define __OCL_ALIAS_ANALYSIS_H__

#include "llvm/Analysis/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/ValueHandle.h"

using namespace llvm;

namespace intel {

  struct OCLAliasAnalysis;

  struct OCLAAResults : public AAResults {

    OCLAAResults(const TargetLibraryInfo &);
    OCLAAResults(OCLAAResults &&);

    virtual void initializePass() {
    }

    virtual AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB);

    virtual bool pointsToConstantMemory(const MemoryLocation &Loc, bool OrLocal = false);


    virtual void deleteValue (Value *V);
    virtual void copyValue (Value *From, Value *To);
    virtual void addEscapingUse (Use &U);

    private:

      // OCLAAACallbackVH - A CallbackVH to arrange for OCLAliasAnalysis to be
      // notified whenever a Value is deleted.
      class OCLAAACallbackVH : public CallbackVH {
        OCLAAResults *OCLAAR;
        void deleted() override;
        void allUsesReplacedWith(Value *New) override;
      public:
        OCLAAACallbackVH(Value *V, OCLAAResults *OCLAAR = nullptr);
      };

      // Helper class to hold the result of address space resolution
      class ResolveResult {
      public:
        ResolveResult(bool r, unsigned int ar) {
          resolved = r;
          addressSpace = ar;
        }
        ResolveResult(const ResolveResult& other) {
          resolved = other.resolved;
          addressSpace = other.addressSpace;
        }
        bool isResolved() {return resolved;}
        unsigned int getAddressSpace() {return addressSpace;}
        bool operator==(const ResolveResult& other) {
          return (resolved == other.resolved) &&
                 (addressSpace == other.addressSpace);
        }
      private:
        bool resolved;
        unsigned int addressSpace;
      };

      // ValuerMapType - The typedef for ValueMap.
      typedef DenseMap<OCLAAACallbackVH, ResolveResult, DenseMapInfo<Value *> >
         ValueMapType;

      // ValueExprMap - This is a cache of the values we have analyzed so far.
      ValueMapType ValueMap;

      // Go over used values and usages and loop for a cast to a named address
      // space. If there are no conversions from/to int and only one namespace
      // different from default (__private) is found resolve all values found
      // on the way to this address space.
      // 1st arg: pointer value to resolve
      // 2nd arg: if true force the resolving once more instead of using
      //          cached results.
      ResolveResult resolveAddressSpace(const Value* value, bool force);

      typedef SmallPtrSet<const Value*, 16> SmallValueSet;
      ResolveResult cacheResult(SmallValueSet& values, ResolveResult resolveResult);

      void rauwValue(Value* oldVal, Value* newVal);

      int m_disjointAddressSpaces;
  };

  struct OCLAliasAnalysis : public FunctionPass {
    std::unique_ptr<OCLAAResults> OCLAAR;
    static char ID;

    OCLAliasAnalysis();

    virtual void *getAdjustedAnalysisPointer(const void *ID) {
      if (ID == &OCLAliasAnalysis::ID)
        return (OCLAliasAnalysis*)this;
      return this;
    }
    OCLAAResults &getOCLAAResults() { return *OCLAAR; }
    const OCLAAResults &getOCLAAResults() const { return *OCLAAR; }

    bool runOnFunction(Function &F) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override;
  };

  FunctionPass *createOCLAliasAnalysisPass();
}
#endif
