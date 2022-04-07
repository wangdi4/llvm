; RUN: SATest -BUILD -pass-manager-type=lto-legacy -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s
; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:        ModulePass Manager
; CHECK-NEXT:     DPCPPPreprocessSPIRVFriendlyIRLegacy
; CHECK-NEXT:     Regularize LLVM for SPIR-V
; CHECK-NEXT:     Translate SPIR-V builtins to OCL 2.0 builtins
; CHECK-NEXT:     Name Anon Globals

#ifndef NDEBUG
; CHECK:        FunctionPass Manager
; CHECK-NEXT:     Module Verifier
#endif // #ifndef NDEBUG

; CHECK:        Unify function exit nodes
; CHECK-NOT:    Infer address spaces

; CHECK:        BuiltinLibInfoAnalysisLegacy

; CHECK:        ModulePass Manager
; CHECK:          DPCPPEqualizerLegacy
; CHECK-NEXT:     DuplicateCalledKernels
; CHECK-NOT:      InternalizeNonKernelFuncLegacy
; CHECK-NEXT:     AddFunctionAttrs
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LinearIdResolverLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK-NOT:      VFAnalysisLegacy
; CHECK-NOT:      SetVectorizationFactorLegacy
; CHECK-NOT:      VectorVariantLoweringLegacy
; CHECK-NOT:      CreateSimdVariantPropagationLegacy
; CHECK-NOT:      SGSizeCollectorLegacy
; CHECK-NOT:      SGSizeCollectorIndirectLegacy
; CHECK-NOT:      DPCPPKernelVecCloneLegacy
; CHECK-NOT:      VecClone
; CHECK:          DPCPPKernelAnalysisLegacy
; CHECK:          ResolveSubGroupWICallLegacy
; CHECK:          WGLoopCreatorLegacy
; CHECK:          Lowering __intel_indirect_call scalar calls
; CHECK-NOT:      ResolveSubGroupWICallLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK:              PhiCanonicalization
; CHECK-NEXT:         Intel Kernel RedundantPhiNode
; CHECK-NEXT:     GroupBuiltin
; CHECK-NEXT:     Intel Kernel BarrierInFunction
; CHECK-NEXT:     ResolveSubGroupWICallLegacy
; CHECK-NEXT:     Intel Kernel SplitBBonBarrier
; CHECK-NEXT:     Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT:     Intel Kernel WIRelatedValue Analysis
; CHECK-NEXT:     Intel Kernel DataPerValue Analysis
; CHECK-NEXT:     Intel Kernel Barrier
; CHECK:          ImplicitArgsAnalysisLegacy
; CHECK:          LocalBufferAnalysisLegacy
; CHECK-NEXT:     AddImplicitArgsLegacy
; CHECK-NEXT:     ResolveWICallLegacy
; CHECK:          CallGraph Construction
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     LocalBuffersLegacy
; CHECK-NEXT:     BuiltinImportLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK:          Inliner for always_inline functions
; CHECK:          PrepareKernelArgsLegacy
; CHECK-NEXT:     CleanupWrappedKernelLegacy
