; RUN: SATest -BUILD -pass-manager-type=lto-legacy -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s

; CHECK:        ModulePass Manager
; CHECK-NEXT:     DPCPPPreprocessSPIRVFriendlyIRLegacy
; CHECK-NEXT:     Regularize LLVM for SPIR-V
; CHECK-NEXT:     Translate SPIR-V builtins to OCL 2.0 builtins

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
; CHECK-NEXT:     DPCPPKernelAnalysisLegacy

; CHECK:          Call Graph SCC Pass Manager
; CHECK:            FunctionPass Manager
; CHECK:              Loop Pass Manager
; CHECK:                Unroll loops

; CHECK:          DPCPPKernelVecCloneLegacy
; CHECK-NOT:      VecClone
; CHECK-NEXT:     Fill-in addresses of vector variants
; CHECK-NEXT:     UpdateCallAttrs
; CHECK:          FunctionPass Manager
; CHECK:            VPlan Vectorizer
; CHECK:          VPlan post vectorization pass for DPCPP kernels
; CHECK:          WGLoopCreatorLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK:              PhiCanonicalization
; CHECK-NEXT:         Intel Kernel RedundantPhiNode
; CHECK-NEXT:     GroupBuiltin
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
; CHECK:          CallGraph Construction
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     LocalBuffersLegacy
; CHECK-NEXT:     BuiltinImportLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK:          Dead Argument Elimination
; CHECK:          PrepareKernelArgsLegacy
; CHECK:          CleanupWrappedKernelLegacy
