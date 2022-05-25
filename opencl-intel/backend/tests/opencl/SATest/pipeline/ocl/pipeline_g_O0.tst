; RUN: SATest -BUILD -pass-manager-type=ocl -debug-passes=Structure -config=%s.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK

; CHECK: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK-NEXT: Running pass: CoerceTypesPass
; CHECK-NEXT: Running pass: SetPreferVectorWidthPass
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Running pass: FMASplitterPass
; CHECK-NEXT: Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running analysis: AssumptionAnalysis
; CHECK-NEXT: Running analysis: TargetIRAnalysis
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: DetectRecursionPass
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ImplicitArgsAnalysis, llvm::Module>
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: ResolveVarTIDCallPass
; CHECK-NEXT: Running pass: TaskSeqAsyncHandling
; CHECK-NEXT: Running pass: ResolveMatrixFillPass
; CHECK-NEXT: Running pass: ResolveMatrixLayoutPass
; CHECK-NEXT: Running pass: ResolveMatrixWISlicePass
; CHECK-NEXT: Running pass: InferArgumentAliasPass
; CHECK-NEXT: Running pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: InstToFuncCallPass
; CHECK-NEXT: Running pass: DuplicateCalledKernels
; CHECK-NEXT: Running pass: DPCPPKernelAnalysisPass
; CHECK-NEXT: Running analysis: LoopAnalysis on test
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: LoopUnrollPass
; CHECK-NEXT: Running pass: OptimizeIDivAndIRemPass
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, llvm::Function>
; CHECK-NEXT: Running pass: PreventDivCrashesPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Invalidating analysis: VFAnalysis
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: DPCPPKernelWGLoopCreatorPass
; CHECK-NEXT: Invalidating analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Running pass: IndirectCallLowering
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Running pass: RemoveRegionDirectivesPass
; CHECK-NEXT: Running pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: PhiCanonicalization
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: Intel Kernel RedundantPhiNode
; CHECK-NEXT: Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: Intel Kernel BarrierInFunction
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Running pass: SGBuiltinPass
; CHECK-NEXT: Running analysis: SGSizeAnalysisPass
; CHECK-NEXT: Running pass: SGBarrierPropagatePass
; CHECK-NEXT: Running pass: SGBarrierSimplifyPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Running pass: SGValueWidenPass
; CHECK-NEXT: Running pass: SGLoopConstructPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: Intel Kernel SplitBBonBarrier
; CHECK-NEXT: Running pass: Intel Kernel Barrier
; CHECK-NEXT: Running analysis: Intel Kernel DataPerValue Analysis
; CHECK-NEXT: Running analysis: Intel Kernel WIRelatedValue Analysis
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running pass: AddTLSGlobalsPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Invalidating analysis: SGSizeAnalysisPass
; CHECK-NEXT: Invalidating analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
; CHECK-NEXT: Invalidating analysis: Intel Kernel WIRelatedValue Analysis
; CHECK-NEXT: Invalidating analysis: Intel Kernel DataPerValue Analysis
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: ResolveWICallPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: LocalBuffersPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Invalidating analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: BuiltinImportPass
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
; CHECK-NEXT: Running pass: AlwaysInlinerPass
; CHECK-NEXT: Running analysis: ProfileSummaryAnalysis
; CHECK-NEXT: Running pass: PatchCallbackArgsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running pass: PrepareKernelArgsPass
; CHECK-NEXT: Running analysis: AssumptionAnalysis
; CHECK-NEXT: Running analysis: TargetIRAnalysis
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
; CHECK-NEXT: Invalidating analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running pass: CleanupWrappedKernelPass

; CHECK: Test program was successfully built.
