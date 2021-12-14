; RUN: SATest -BUILD -tsize=1 -pass-manager-type=lto-legacy -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s
; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:        ModulePass Manager
; CHECK-NEXT:     DPCPPPreprocessSPIRVFriendlyIRLegacy
; CHECK-NEXT:     Regularize LLVM for SPIR-V
; CHECK-NEXT:     Translate SPIR-V builtins to OCL 2.0 builtins
; CHECK-NEXT:     Name Anon Globals

; CHECK:        FunctionPass Manager
; CHECK-NEXT:     Unify function exit nodes
; CHECK-NEXT:     Infer address spaces

; CHECK:        ModulePass Manager
; CHECK:          DPCPPEqualizerLegacy
; CHECK-NEXT:     DuplicateCalledKernels
; CHECK-NEXT:     InternalizeNonKernelFuncLegacy
; CHECK-NEXT:     AddFunctionAttrs
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LinearIdResolverLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK:          DPCPPKernelAnalysisLegacy

; CHECK:          Call Graph SCC Pass Manager
; CHECK:            FunctionPass Manager
; CHECK:              Loop Pass Manager
; CHECK:                Unroll loops

; CHECK-NOT:      VFAnalysisLegacy
; CHECK-NOT:      SetVectorizationFactorLegacy
; CHECK-NOT:      VectorVariantLoweringLegacy
; CHECK-NOT:      CreateSimdVariantPropagationLegacy
; CHECK-NOT:      SGSizeCollectorLegacy
; CHECK-NOT:      SGSizeCollectorIndirectLegacy
; CHECK-NOT:      DPCPPKernelVecClone pass
; CHECK-NOT:      VPlan Vectorization Driver
; CHECK-NOT:      VPlan post vectorization pass for DPCPP kernels
; CHECK:          ResolveSubGroupWICallLegacy
; CHECK-NEXT:     WGLoopCreatorLegacy
; CHECK:          Lowering __intel_indirect_call scalar calls
; CHECK:          ResolveSubGroupWICallLegacy
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
; CHECK:          Recognize loop idioms
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
; CHECK:          Function Integration/Inlining
; CHECK:          Dead Argument Elimination
; CHECK:          PrepareKernelArgsLegacy
; CHECK:          CleanupWrappedKernelLegacy
