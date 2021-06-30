; RUN: SATest -BUILD -pass-manager-type=lto-legacy -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s

; CHECK:        FunctionPass Manager
; CHECK-NEXT:     Unify function exit nodes
; CHECK-NEXT:     Infer address spaces

; CHECK:        ModulePass Manager
; CHECK:          Translate SPIR-V builtins to OCL 2.0 builtins
; CHECK:          DPCPPEqualizerLegacy
; CHECK-NOT:      InternalizeNonKernelFuncLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LinearIdResolverLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK-NEXT:     DPCPPKernelAnalysisLegacy
; CHECK-NOT:      VecClone
; CHECK:          WGLoopCreatorLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK:              PhiCanonicalization
; CHECK-NEXT:         Intel Kernel RedundantPhiNode
; CHECK-NEXT:     Intel Kernel BarrierInFunction
; CHECK-NEXT:     Intel Kernel SplitBBonBarrier
; CHECK-NEXT:     Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT:     Intel Kernel WIRelatedValue Analysis
; CHECK-NEXT:     Intel Kernel DataPerValue Analysis
; CHECK-NEXT:     Intel Kernel Barrier
; CHECK:          ImplicitArgsAnalysisLegacy
; CHECK:          LocalBufferAnalysisLegacy
; CHECK-NEXT:     AddImplicitArgsLegacy
; CHECK-NEXT:     ResolveWICallLegacy
; CHECK-NEXT:     BuiltinImportLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK-NEXT:     PrepareKernelArgsLegacy
; CHECK-NEXT:     CleanupWrappedKernelLegacy
