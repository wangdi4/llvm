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

    private:

      int m_disjointAddressSpaces;

  };
}
#endif

