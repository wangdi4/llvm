; RUN: SATest -BUILD -pass-manager-type=lto -llvm-option=-debug-pass-manager -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass=Structure -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-CG

; TODO:
;   check CoerceWin64Types pass when SATest is enabled

; CHECK:      Running pass: KernelTargetExtTypeLowerPass
; CHECK:      Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: SpecializeConstantPass

#ifndef NDEBUG
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: SYCLEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK:      Running pass: ExternalizeGlobalVariablesPass
; CHECK-NEXT: Running pass: CoerceTypesPass
; CHECK-NEXT: Running pass: SetPreferVectorWidthPass
; CHECK:      Running pass: LinearIdResolverPass
; CHECK:      Running pass: AddFunctionAttrsPass
; CHECK:      Running pass: ResolveVarTIDCallPass
; CHECK:      Running pass: SGRemapWICallPass
; CHECK:      Running pass: BuiltinCallToInstPass

; CHECK:      Running pass: DetectRecursionPass
; CHECK:      Running pass: DuplicateCalledKernels
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: SYCLKernelAnalysisPass
; CHECK:      Running pass: InstToFuncCallPass
; CHECK-NEXT: Running pass: MathFuncSelectPass
; CHECK:      Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK:      Running analysis: VFAnalysis
; CHECK:      Running analysis: WeightedInstCountAnalysis

; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: PreventDivCrashesPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis
; CHECK-NEXT: Running pass: SYCLKernelWGLoopCreatorPass
; CHECK:      Running pass: IndirectCallLowering ;INTEL
; CHECK:      Skipping pass UnifyFunctionExitNodesPass

; CHECK:      Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: BarrierInFunction
; CHECK:      Running pass: SGBuiltinPass
; CHECK-NEXT: Running analysis: SGSizeAnalysisPass
; CHECK-NEXT: Running pass: SGBarrierPropagatePass
; CHECK-NEXT: Running pass: SGBarrierSimplifyPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis
; CHECK-NEXT: Running pass: SGValueWidenPass
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Running pass: SGLoopConstructPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: SplitBBonBarrier
; CHECK-NEXT: Running pass: KernelBarrier
; CHECK:      Running analysis: DataPerValueAnalysis
; CHECK:      Running analysis: WIRelatedValueAnalysis

; CHECK:      Running pass: AddImplicitArgsPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: LocalBuffersPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: BuiltinImportPass
; CHECK:      Running pass: BuiltinCallToInstPass
; CHECK:      Running pass: AlwaysInlinerPass
; CHECK-NEXT: Running pass: PatchCallbackArgsPass
; CHECK-NEXT: Running pass: PrepareKernelArgsPass

; CHECK:      Running pass: CleanupWrappedKernelPass

; CHECK: Test program was successfully built.

; CHECK-CG-NOT: Type-Based Alias Analysis
