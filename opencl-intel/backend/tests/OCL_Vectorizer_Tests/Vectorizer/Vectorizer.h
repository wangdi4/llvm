/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __VECTORIZER_H__
#define __VECTORIZER_H__

#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"

namespace llvm {
  class Function;
}

using namespace llvm;

class Vectorizer;

/// createVectorizerPass - Returns an instance of the autovectorizer pass,
/// which will be added to a PassManager and run on a Module.
extern "C" Pass *createVectorizerPass(const Module *runtimeModule, int basicVectorWidth);

/// getVectorizerFunctions - Asks the Vectorizer V to fill in a SmallVector
/// with the Function pointers which point to autovectorized versions of the
/// kernel functions, in the order the kernel functions are listed in the 
/// llvm.global.annotations structure.  If an autovectorized version of the
/// kernel is not available, this function should push_back() NULL for that
/// slot.
extern "C" int getVectorizerFunctions(Vectorizer *V, SmallVectorImpl<Function*> &Functions);

/// getVectorizerWidths - Similar to getVectorizerFunctions, but asks for the
/// width at which each kernel function was successfully autovectorized, or 0
/// if not vectorized.
extern "C" int getVectorizerWidths(Vectorizer *V, SmallVectorImpl<int> &MaxWidths);


#endif // __VECTORIZER_H__
