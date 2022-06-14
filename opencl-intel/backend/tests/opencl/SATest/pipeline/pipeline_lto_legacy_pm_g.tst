; RUN: SATest -BUILD -pass-manager-type=lto-legacy -debug-passes=Structure -config=%S/pipeline_lto_g.tst.cfg 2>&1 | FileCheck %s
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

; CHECK:        BuiltinLibInfoAnalysisLegacy

; CHECK:        ModulePass Manager
; CHECK:          DPCPPEqualizerLegacy
; CHECK:          DuplicateCalledKernels
; CHECK-NEXT:     InternalizeNonKernelFuncLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       FMASplitter
; CHECK-NEXT:     AddFunctionAttrs
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Simplify the CFG
; CHECK:            SROA
; CHECK:            Combine redundant instructions
; CHECK:            Remove redundant instructions
; CHECK:          CallGraph Construction
; CHECK-NEXT:     LinearIdResolverLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK:            Promote Memory to Register
; CHECK:            Infer address spaces
; CHECK:            BuiltinCallToInstLegacy

; CHECK:          DetectRecursionLegacy
; CHECK:          FunctionPass Manager
; CHECK-NEXT:       Reassociate expressions
; CHECK:            Infer address spaces
; CHECK:            Simplify the CFG
; CHECK:            SROA
; CHECK:            Early CSE
; CHECK:            Promote Memory to Register
; CHECK:            Combine redundant instructions
; CHECK:          ResolveVarTIDCallLegacy
; CHECK-NEXT:     InferArgumentAlias
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Unify function exit nodes
; CHECK:          InstToFuncCallLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     DPCPPKernelAnalysisLegacy
; CHECK:          FunctionPass Manager
; CHECK:            Simplify the CFG
; CHECK-NEXT:     WGLoopBoundariesLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dead Code Elimination
; CHECK-NEXT:       Simplify the CFG
; CHECK-NEXT:     DeduceMaxWGDimLegacy
; CHECK:          FunctionPass Manager
; CHECK-NEXT:       SinCosFoldLegacy
; CHECK:            Replace known math operations with optimized library functions
; CHECK:            Unify function exit nodes
; CHECK:            Simplify the CFG
; CHECK:            Combine redundant instructions
; CHECK:            Early GVN Hoisting of Expressions
; CHECK:            Dead Code Elimination
; CHECK:          Intel DPCPP Kernel ReqdSubGroupSize Pass
; CHECK:          VFAnalysisLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK:              WeightedInstCountAnalysisLegacy
; CHECK:          SetVectorizationFactorLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     VectorVariantLoweringLegacy
; CHECK-NEXT:     CreateSimdVariantPropagationLegacy
; CHECK-NEXT:     SGSizeCollectorLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     SGSizeCollectorIndirectLegacy
; CHECK-NEXT:     VectorizationDimensionAnalysisLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK:              SoaAllocaAnalysisLegacy
; CHECK:              WorkItemAnalysisLegacy
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
; CHECK-NEXT:     FunctionPass Manager
; CHECK:            Combine redundant instructions
; CHECK:            Simplify the CFG
; CHECK:            Promote Memory to Register
; CHECK:            Aggressive Dead Code Elimination
; CHECK:          VectorKernelEliminationLegacy
; CHECK:            FunctionPass Manager
; CHECK:              WeightedInstCountAnalysisLegacy
; CHECK:          HandleVPlanMask pass
; CHECK:          ResolveSubGroupWICallLegacy
; CHECK:          FunctionPass Manager
; CHECK-NEXT:       Intel DPCPP Kernel OptimizeIDivAndIRem Pass
; CHECK-NEXT:       Intel DPCPP Kernel PreventDivCrashes Pass
; CHECK:            Combine redundant instructions
; CHECK:            Global Value Numbering
; CHECK:            Optimize scalar/vector ops
; CHECK:            Jump Threading
; CHECK:          Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT:     ImplicitGIDLegacy
; CHECK-NEXT:     WGLoopCreatorLegacy
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Unify function exit nodes
; CHECK-NEXT:     FunctionPass Manager
; CHECK:            Loop Pass Manager
; CHECK:              Unroll loops
; CHECK:          Lowering __intel_indirect_call scalar calls
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dead Code Elimination
; CHECK-NEXT:       Simplify the CFG
; CHECK-NEXT:       Remove llvm.directive.region.*
; CHECK-NEXT:       Unify function exit nodes
; CHECK-NEXT:     Intel DPCPP Kernel ReplaceScalarWithMask Pass
; CHECK:          ResolveSubGroupWICallLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dead Code Elimination
; CHECK-NEXT:       Simplify the CFG
; CHECK:            Promote Memory to Register
; CHECK:            PhiCanonicalization
; CHECK-NEXT:       Intel Kernel RedundantPhiNode
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
; CHECK:          FunctionPass Manager
; CHECK:            Promote Memory to Register
; CHECK:            Canonicalize natural loops
; CHECK:            Loop Pass Manager
; CHECK:              hoist known uniform dpcpp builtins out of loops
; CHECK:            Loop Pass Manager
; CHECK-NEXT:         Loop Invariant Code Motion
; CHECK-NEXT:       Loop Pass Manager
; CHECK-NEXT:         LoopWIAnalysisLegacy
; CHECK-NEXT:         Hoist strided values out of loops
; CHECK:          CallGraph Construction
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     ImplicitArgsAnalysisLegacy
; CHECK-NEXT:     AddTLSGlobalsLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     ResolveWICallLegacy
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     LocalBufferAnalysisLegacy
; CHECK-NEXT:     LocalBuffersLegacy
; CHECK:          BuiltinImportLegacy
; CHECK-NEXT:     InternalizeGlobalVariablesLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       BuiltinCallToInstLegacy
; CHECK:          Function Integration/Inlining
; CHECK:          Dead Global Elimination
; CHECK:          Dead Argument Elimination
; CHECK-NEXT:     FunctionPass Manager
; CHECK:            SROA
; CHECK:            Canonicalize natural loops
; CHECK:            Loop Invariant Code Motion
; CHECK:            Recognize loop idioms
; CHECK:            Delete dead loops
; CHECK:            Hoist strided values out of loops
; CHECK:            Simplify the CFG
; CHECK:          ImplicitArgsAnalysisLegacy
; CHECK-NEXT:     PatchCallBackArgs
; CHECK:          PrepareKernelArgsLegacy
; CHECK-NEXT:     FunctionPass Manager
; CHECK:            Simplify the CFG
; CHECK:            SROA
; CHECK:            Combine redundant instructions
; CHECK:            Global Value Numbering
; CHECK:            Dead Store Elimination
; CHECK:            Aggressive Dead Code Elimination
; CHECK:            Early CSE
; CHECK:            Combine redundant instructions
; CHECK:          CleanupWrappedKernelLegacy
