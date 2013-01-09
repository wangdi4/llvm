//===----------------------------------------------------------------------===//
//
// This file contains the declarations for the pass initialization routines
// for the OCL project.
//
//===----------------------------------------------------------------------===//

#ifndef OCL_INITIALIZEPASSES_H
#define OCL_INITIALIZEPASSES_H

namespace intel {

class llvm::PassRegistry;

void initializePhiCanonPass(llvm::PassRegistry&);
void initializePredicatorPass(llvm::PassRegistry&);
void initializeWIAnalysisPass(llvm::PassRegistry&);
void initializeScalarizeFunctionPass(llvm::PassRegistry&);
void initializeSimplifyGEPPass(llvm::PassRegistry&);
void initializePacketizeFunctionPass(llvm::PassRegistry&);
void initializeX86ResolverPass(llvm::PassRegistry&);
void initializeMICResolverPass(llvm::PassRegistry&);
void initializeX86LowerPass(llvm::PassRegistry&);
void initializeOCLBuiltinPreVectorizationPassPass(llvm::PassRegistry&);
void initializeSpecialCaseBuiltinResolverPass(llvm::PassRegistry&);
void initializeAppleWIDepPrePacketizationPassPass(llvm::PassRegistry&);
void initializeCLWGLoopBoundariesPass(llvm::PassRegistry&);
void initializeCLWGLoopCreatorPass(llvm::PassRegistry&);
void initializeKernelAnalysisPass(llvm::PassRegistry&);
void initializeIRInjectModulePass(llvm::PassRegistry&);
void initializenameByInstTypePass(llvm::PassRegistry&);
void initializeRedundantPhiNodePass(llvm::PassRegistry&);
void initializeBarrierInFunctionPass(llvm::PassRegistry&);
void initializeRemoveDuplicationBarrierPass(llvm::PassRegistry&);
void initializeSplitBBonBarrierPass(llvm::PassRegistry&);
void initializeBarrierPass(llvm::PassRegistry&);
void initializeWIRelatedValuePass(llvm::PassRegistry&);
void initializeDataPerBarrierPass(llvm::PassRegistry&);
void initializeDataPerValuePass(llvm::PassRegistry&);
void initializeDataPerInternalFunctionPass(llvm::PassRegistry&);
void initializePreventDivCrashesPass(llvm::PassRegistry&);
void initializeShuffleCallToInstPass(llvm::PassRegistry&);
void initializeInstToFuncCallPass(llvm::PassRegistry&);
void initializeModuleCleanupWrapperPass(llvm::PassRegistry&);
void initializeAddImplicitArgsWrapperPass(llvm::PassRegistry&);
void initializeLocalBuffersWrapperPass(llvm::PassRegistry&);
void initializeLocalBuffersWithDebugWrapperPass(llvm::PassRegistry&);
void initializeRelaxedPassPass(llvm::PassRegistry&);
void initializeShiftZeroUpperBitsPass(llvm::PassRegistry&);
void initializePrefetchPass(llvm::PassRegistry&);

}

#endif
