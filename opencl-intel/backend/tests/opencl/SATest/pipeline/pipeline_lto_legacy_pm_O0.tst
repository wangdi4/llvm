; RUN: SATest -BUILD -pass-manager-type=lto-legacy -debug-passes=Structure -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s
; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:        ModulePass Manager
; CHECK-NEXT:     DPCPPPreprocessSPIRVFriendlyIRLegacy
; CHECK-NEXT:     Regularize LLVM for SPIR-V
; CHECK-NEXT:     Translate SPIR-V builtins to OCL 2.0 builtins

#ifndef NDEBUG
; CHECK:        FunctionPass Manager
; CHECK-NEXT:     Module Verifier
#endif // #ifndef NDEBUG

; CHECK:        FunctionPass Manager
; CHECK:          Unify function exit nodes

; CHECK:        BuiltinLibInfoAnalysisLegacy

; CHECK:        ModulePass Manager
; CHECK:          DPCPPEqualizerLegacy
; CHECK:          SetPreferVectorWidthLegacy
; CHECK:          DuplicateCalledKernels
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       FMASplitter
; CHECK-NEXT:     AddFunctionAttrs
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LinearIdResolverLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK:            BuiltinCallToInstLegacy
; CHECK-NOT:      DPCPPKernelVecCloneLegacy
; CHECK-NOT:      VecClone
; CHECK:          CallGraph Construction
; CHECK-NEXT:     DPCPPKernelAnalysisLegacy
; CHECK:          Intel DPCPP Kernel ReqdSubGroupSize Pass
; CHECK-NEXT:     VFAnalysisLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK:              WeightedInstCountAnalysisLegacy
; CHECK:          SetVectorizationFactorLegacy
; CHECK:          ResolveSubGroupWICallLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Intel DPCPP Kernel PreventDivCrashes Pass
; CHECK-NEXT:     WGLoopCreatorLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Unify function exit nodes
; CHECK-NEXT:     Lowering __intel_indirect_call scalar calls
; CHECK:          GroupBuiltin
; CHECK-NEXT:     Intel Kernel BarrierInFunction
; CHECK-NEXT:     Intel DPCPP Kernel RemoveDuplicatedBarrier Pass
; CHECK-NEXT:     SGSizeAnalysisLegacy
; CHECK-NEXT:     SGBuiltinLegacy
; CHECK-NEXT:     SGBarrierPropagateLegacy
; CHECK-NEXT:     SGBarrierSimplifyLegacy
; CHECK-NEXT:     SGValueWidenLegacy
; CHECK-NEXT:     SGSizeAnalysisLegacy
; CHECK-NEXT:     SGLoopConstructLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Module Verifier
; CHECK-NEXT:     ResolveSubGroupWICallLegacy
; CHECK-NEXT:     Intel Kernel SplitBBonBarrier
; CHECK-NEXT:     Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT:     Intel Kernel WIRelatedValue Analysis
; CHECK-NEXT:     Intel Kernel DataPerValue Analysis
; CHECK-NEXT:     Intel Kernel Barrier
; CHECK:          CallGraph Construction
; CHECK-NEXT:     ImplicitArgsAnalysisLegacy
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     AddImplicitArgsLegacy
; CHECK-NEXT:     ResolveWICallLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     LocalBuffersLegacy
; CHECK:          BuiltinImportLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK:          Call Graph SCC Pass Manager
; CHECK:            Inliner for always_inline functions
; CHECK-NEXT:     ImplicitArgsAnalysisLegacy
; CHECK-NEXT:     PatchCallBackArgs
; CHECK-NEXT:     PrepareKernelArgsLegacy
; CHECK-NEXT:     CleanupWrappedKernelLegacy
