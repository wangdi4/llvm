; RUN: SATest -BUILD -pass-manager-type=ocl -llvm-option=-debug-pass-manager -config=%s.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK

; CHECK: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: SYCLEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK-NEXT: Running pass: ExternalizeGlobalVariablesPass
; CHECK-NEXT: Running pass: CoerceTypesPass
; CHECK-NEXT: Running pass: SetPreferVectorWidthPass
; CHECK-NEXT: Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: ResolveVarTIDCallPass
; CHECK-NEXT: Running pass: SGRemapWICallPass
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: DetectRecursionPass
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ImplicitArgsAnalysis, llvm::Module
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; INTEL_CUSTOMIZATION
; CHECK-NEXT: Running pass: TaskSeqAsyncHandling
; CHECK-NEXT: Running pass: ResolveMatrixFillPass
; CHECK-NEXT: Running pass: ResolveMatrixWISlicePass
; end INTEL_CUSTOMIZATION
; CHECK-NEXT: Skipping pass UnifyFunctionExitNodesPass
; CHECK-NEXT: Skipping pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: InstToFuncCallPass
; CHECK-NEXT: Running pass: MathFuncSelectPass
; CHECK-NEXT: Running pass: DuplicateCalledKernelsPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: SYCLKernelAnalysisPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis
; CHECK-NEXT: Running analysis: TargetIRAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running analysis: AssumptionAnalysis
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: ResolveMatrixLayoutPass ; INTEL
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: PreventDivCrashesPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis
; CHECK:      Invalidating analysis: ImplicitArgsAnalysis
; CHECK:      Invalidating analysis: VFAnalysis
; CHECK:      Running pass: SYCLKernelWGLoopCreatorPass
; CHECK-NEXT: Invalidating analysis: DataPerBarrierAnalysis
; CHECK:      Running pass: IndirectCallLowering ;INTEL
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Skipping pass UnifyFunctionExitNodesPass
; CHECK-NEXT: Skipping pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: GroupBuiltinPass
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
; CHECK-NEXT: Running analysis: DataPerValueAnalysis
; CHECK-NEXT: Running analysis: WIRelatedValueAnalysis
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running pass: AddTLSGlobalsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Invalidating analysis: SGSizeAnalysisPass
; CHECK:      Invalidating analysis: DataPerBarrierAnalysis
; CHECK:      Invalidating analysis: WIRelatedValueAnalysis
; CHECK:      Invalidating analysis: DataPerValueAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK:      Running pass: LocalBuffersPass
; CHECK:      Invalidating analysis: ImplicitArgsAnalysis
; CHECK:      Invalidating analysis: LocalBufferAnalysis
; CHECK:      Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: BuiltinImportPass
; CHECK:      Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: AlwaysInlinerPass
; CHECK-NEXT: Running analysis: ProfileSummaryAnalysis
; CHECK-NEXT: Running pass: PatchCallbackArgsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running pass: PrepareKernelArgsPass
; CHECK-NEXT: Running analysis: AssumptionAnalysis
; CHECK-NEXT: Running analysis: TargetIRAnalysis
; CHECK:      Invalidating analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: CleanupWrappedKernelPass

; CHECK: Test program was successfully built.
