; RUN: SATest -BUILD -pass-manager-type=lto-legacy -debug-passes=Structure -config=%S/pipeline_lto_g.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -debug-passes=Structure -config=%S/pipeline_lto_g.tst.cfg 2>&1 | FileCheck %s
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

; CHECK:        FunctionPass Manager
; CHECK-NEXT:     Unify function exit nodes
; CHECK-NEXT:     Infer address spaces

; CHECK:        BuiltinLibInfoAnalysisLegacy

; CHECK:        ModulePass Manager
; CHECK:          DPCPPEqualizerLegacy
; CHECK-NEXT:     DuplicateCalledKernels
; CHECK-NEXT:     InternalizeNonKernelFuncLegacy
; CHECK-NEXT:     AddFunctionAttrs
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LinearIdResolverLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy

; CHECK:          Call Graph SCC Pass Manager
; CHECK:            FunctionPass Manager
; CHECK:              Loop Pass Manager
; CHECK:                Unroll loops

; CHECK:          Reassociate expressions
; CHECK:          Infer address spaces
; CHECK:          DPCPPKernelAnalysisLegacy
; CHECK:          WGLoopBoundariesLegacy
; CHECK:          DeduceMaxWGDimLegacy
; CHECK:          InstToFuncCallLegacy
; CHECK:          FunctionPass Manager
; CHECK-NEXT:       SinCosFoldLegacy
; CHECK:          Replace known math operations with optimized library functions
; CHECK:          VFAnalysisLegacy
; CHECK:          SetVectorizationFactorLegacy
; CHECK:          VectorVariantLoweringLegacy
; CHECK:          CreateSimdVariantPropagationLegacy
; CHECK:          SGSizeCollectorLegacy
; CHECK:          SGSizeCollectorIndirectLegacy
; CHECK:          DPCPPKernelVecCloneLegacy
; CHECK-NEXT:     Fill-in addresses of vector variants
; CHECK-NEXT:     UpdateCallAttrs
; CHECK:          FunctionPass Manager
; CHECK:            Promote Memory to Register
; CHECK:            Canonicalize natural loops
; CHECK:            Loop Invariant Code Motion
; CHECK-NOT:      VecClone
; CHECK:          FunctionPass Manager
; CHECK:            VPlan Vectorizer
; CHECK:          VPlan post vectorization pass for DPCPP kernels
; CHECK:          HandleVPlanMask
; CHECK:          ResolveSubGroupWICallLegacy
; CHECK:          WGLoopCreatorLegacy
; CHECK:          Unroll loops
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
; CHECK-NEXT:     ReduceCrossBarrierValuesLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Dominance Frontier Construction
; CHECK-NEXT:     Intel Kernel DataPerValue Analysis
; CHECK-NEXT:     Intel Kernel Barrier
; CHECK:          LocalBufferAnalysisLegacy
; CHECK:          ImplicitArgsAnalysisLegacy
; CHECK-NEXT:     AddTLSGlobalsLegacy
; CHECK:          ResolveWICallLegacy
; CHECK:          CallGraph Construction
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     LocalBuffersLegacy
; CHECK-NEXT:     BuiltinImportLegacy
; CHECK:          FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK:          Function Integration/Inlining
; CHECK:          Dead Argument Elimination
; CHECK:          SROA
; CHECK:          Loop Invariant Code Motion
; CHECK:          Recognize loop idioms
; CHECK:          Delete dead loops
; CHECK:          Hoist strided values out of loops
; CHECK:          Simplify the CFG
; CHECK:          PrepareKernelArgsLegacy
; CHECK:          Simplify the CFG
; CHECK:          SROA
; CHECK:          Combine redundant instructions
; CHECK:          Global Value Numbering
; CHECK:          Dead Store Elimination
; CHECK:          Aggressive Dead Code Elimination
; CHECK:          Early CSE
; CHECK:          CleanupWrappedKernelLegacy
