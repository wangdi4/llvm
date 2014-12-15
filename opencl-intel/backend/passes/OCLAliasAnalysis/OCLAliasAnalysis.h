/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __OCL_ALIAS_ANALYSIS_H__
#define __OCL_ALIAS_ANALYSIS_H__

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace llvm;

namespace intel {
  struct OCLAliasAnalysis : public ImmutablePass, public AliasAnalysis {

    static char ID;

    OCLAliasAnalysis();

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AliasAnalysis::getAnalysisUsage(AU);
      AU.addRequired<AliasAnalysis>();
    }

    virtual void initializePass() {
      InitializeAliasAnalysis(this);
    }

    virtual AliasResult alias(const Location &LocA, const Location &LocB);

    virtual bool pointsToConstantMemory(const Location &Loc, bool OrLocal = false);

    virtual void *getAdjustedAnalysisPointer(const void *ID) {
      if (ID == &AliasAnalysis::ID)
        return (AliasAnalysis*)this;
      return this;
    }

    virtual void deleteValue (Value *V);
    virtual void copyValue (Value *From, Value *To);
    virtual void addEscapingUse (Use &U);

    private:

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
      std::map<const Value*, ResolveResult> m_cachedResults;

      int m_disjointAddressSpaces;

  };
}
#endif

