// Copyright 2012-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

//===----------------------------------------------------------------------===//
//
// This file contains the declarations for the pass initialization routines
// for the OCL project.
//
//===----------------------------------------------------------------------===//

#ifndef OCL_INITIALIZEPASSES_H
#define OCL_INITIALIZEPASSES_H

namespace llvm {
    class PassRegistry;
}

namespace intel {
void initializePhiCanonPass(llvm::PassRegistry&);
void initializePredicatorPass(llvm::PassRegistry&);
void initializeOCLBranchProbabilityPass(llvm::PassRegistry&);
void initializeScalarizeFunctionPass(llvm::PassRegistry&);
void initializeSimplifyGEPPass(llvm::PassRegistry&);
void initializePacketizeFunctionPass(llvm::PassRegistry&);
void initializeX86ResolverPass(llvm::PassRegistry&);
void initializeAVX512ResolverPass(llvm::PassRegistry&);
void initializeOCLBuiltinPreVectorizationPassPass(llvm::PassRegistry&);
void initializeSpecialCaseBuiltinResolverPass(llvm::PassRegistry&);
void initializeCLStreamSamplerPass(llvm::PassRegistry &);

void initializeBuiltinLibInfoPass(llvm::PassRegistry&);
void initializePrefetchPass(llvm::PassRegistry&);
void initializeLocalBuffersPass(llvm::PassRegistry &);
void initializeLocalBuffAnalysisPass(llvm::PassRegistry &);
void initializeRemovePrefetchPass(llvm::PassRegistry&);
void initializeResolveSubGroupWICallPass(llvm::PassRegistry&);
void initializeDebugInfoPassPass(llvm::PassRegistry&);
void initializeSmartGVNPass(llvm::PassRegistry &);
void initializeStripIntelIPPass(llvm::PassRegistry &);
void initializeScalarizeFunctionPass(llvm::PassRegistry &);
void initializeChooseVectorizationDimensionPass(llvm::PassRegistry &);
void initializeCoerceWin64TypesPass(llvm::PassRegistry &);
}

#endif
