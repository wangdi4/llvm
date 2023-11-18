; RUN: SATest -BUILD -enable-expensive-mem-opts=1 -pass-manager-type=ocl -llvm-option=-debug-pass-manager -config=%s.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK

; CHECK:      Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass
; CHECK-NEXT: Running pass: SYCLEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK-NEXT: Running pass: ExternalizeGlobalVariablesPass
; CHECK-NEXT: Running pass: CoerceTypesPass
; CHECK-NEXT: Running pass: SetPreferVectorWidthPass
; CHECK-NEXT: Running pass: InternalizeNonKernelFuncPass

; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK:      Running pass: AddFunctionAttrsPass
; CHECK:      Running pass: SimplifyCFGPass

; CHECK:      Running pass: SROAPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running analysis: SYCLAliasAnalysis

; CHECK:      Running pass: InstSimplifyPass
; CHECK-NEXT: Running pass: ResolveVarTIDCallPass
; CHECK-NEXT: Running pass: SGRemapWICallPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: DeadArgumentEliminationPass
; CHECK-NEXT: Running pass: GlobalOptPass

; CHECK:      Running pass: IPSCCPPass

; CHECK:      Running pass: InstSimplifyPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: InferFunctionAttrsPass

; CHECK:      Running pass: ArgumentPromotionPass

; CHECK:      Running pass: SROAPass

; CHECK:      Running pass: EarlyCSEPass
; CHECK-NEXT: Running pass: InstSimplifyPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running analysis: SYCLAliasAnalysis

; CHECK:      Running pass: JumpThreadingPass

; CHECK:      Running pass: CorrelatedValuePropagationPass

; CHECK:      Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: TailCallElimPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: ReassociatePass
; CHECK-NEXT: Running pass: LoopSimplifyPass

; CHECK:      Running pass: LCSSAPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: InstSimplifyPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass

; CHECK:      Running pass: ModuleInlinerWrapperPass

; CHECK:      Running pass: InlinerPass
; CHECK-NEXT: Running pass: InlinerPass

; CHECK:      Running pass: LoopUnrollPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: InstSimplifyPass
; CHECK-NEXT: Running pass: GVNPass

; CHECK:      Running pass: MemCpyOptPass

; CHECK:      Running pass: SCCPPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: JumpThreadingPass

; CHECK:      Running pass: CorrelatedValuePropagationPass

; CHECK:      Running pass: DSEPass
; CHECK-NEXT: Running pass: ADCEPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: StripDeadPrototypesPass
; CHECK-NEXT: Running pass: GlobalDCEPass
; CHECK-NEXT: Running pass: ConstantMergePass
; CHECK-NEXT: Running pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: DetectRecursionPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ImplicitArgsAnalysis, llvm::Module
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running pass: EarlyCSEPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: InstCombinePass
; INTEL_CUSTOMIZATION
; CHECK-NEXT: Running pass: TaskSeqAsyncHandling
; CHECK-NEXT: Running pass: ResolveMatrixFillPass
; CHECK-NEXT: Running pass: ResolveMatrixWISlicePass
; end INTEL_CUSTOMIZATION
; CHECK-NEXT: Running pass: InferArgumentAliasPass
; CHECK-NEXT: Running pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: InstToFuncCallPass
; CHECK-NEXT: Running pass: MathFuncSelectPass
; CHECK-NEXT: Running pass: DuplicateCalledKernelsPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running pass: SYCLKernelAnalysisPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: WGLoopBoundariesPass
; CHECK:      Invalidating analysis: ImplicitArgsAnalysis
; CHECK:      Running analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Running pass: DCEPass

; CHECK:      Running pass: SimplifyCFGPass

; CHECK:      Running pass: DeduceMaxWGDimPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis

; CHECK:      Running pass: SinCosFoldPass

; CHECK:      Running pass: MathLibraryFunctionsReplacementPass

; CHECK:      Running pass: UnifyFunctionExitNodesPass
; CHECK-NEXT: Running pass: SimplifyCFGPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running pass: GVNHoistPass

; CHECK:      Running pass: DCEPass
; CHECK:      Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running pass: VectorVariantLowering
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation
; CHECK-NEXT: Running pass: SGSizeCollectorPass
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass
; CHECK-NEXT: Running pass: SYCLKernelVecClonePass
; CHECK:      Running pass: VectorVariantFillIn
; CHECK-NEXT: Running pass: UpdateCallAttrs

; CHECK:      Running pass: PromotePass

; CHECK:      Running pass: LowerSwitchPass

; CHECK:      Running pass: LoopSimplifyPass

; CHECK:      Running pass: LCSSAPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: vpo::VPlanDriverLLVMPass

; CHECK:      Running pass: PromotePass

; CHECK:      Running pass: LowerSwitchPass

; CHECK:      Running pass: LoopSimplifyPass

; CHECK:      Running pass: LCSSAPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK:      Running pass: LICMPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: vpo::VPlanDriverLLVMPass

; CHECK:      Running pass: SYCLKernelPostVecPass
; CHECK:      Running pass: VPODirectiveCleanupPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: ADCEPass
; CHECK-NEXT: Running pass: VPODirectiveCleanupPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: ADCEPass

; CHECK:      Running pass: HandleVPlanMask
; CHECK-NEXT: Running pass: VerifierPass

; CHECK:      Running pass: ResolveMatrixLayoutPass ; INTEL
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: LoopUnrollPass
; CHECK-NEXT: Running pass: OptimizeIDivAndIRemPass
; CHECK-NEXT: Running pass: PreventDivCrashesPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: GVNPass

; CHECK:      Running pass: VectorCombinePass
; CHECK-NEXT: Running pass: JumpThreadingPass
; CHECK-NEXT: Running pass: LoopUnrollPass
; CHECK:      Running pass: OptimizeIDivAndIRemPass
; CHECK-NEXT: Running pass: PreventDivCrashesPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK:      Running pass: GVNPass

; CHECK:      Running pass: VectorCombinePass
; CHECK-NEXT: Running pass: JumpThreadingPass

; CHECK:      Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis

; CHECK:      Running pass: SYCLKernelWGLoopCreatorPass

; CHECK:      Running pass: IndirectCallLowering ;INTEL

; CHECK:      Running pass: DCEPass

; CHECK:      Running pass: SimplifyCFGPass

; CHECK:      Running pass: UnifyFunctionExitNodesPass
; CHECK:      Running pass: ReplaceScalarWithMaskPass
; CHECK-NEXT: Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: DCEPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: PromotePass

; CHECK:      Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: BarrierInFunction
; CHECK:      Running pass: RemoveDuplicatedBarrierPass
; CHECK-NEXT: Running pass: SGBuiltinPass
; CHECK-NEXT: Running analysis: SGSizeAnalysisPass
; CHECK-NEXT: Running pass: SGBarrierPropagatePass
; CHECK-NEXT: Running pass: SGBarrierSimplifyPass
; CHECK-NEXT: Running pass: ImplicitGIDPass
; CHECK-NEXT: Running analysis: DataPerBarrierAnalysis
; CHECK-NEXT: Running pass: SGValueWidenPass
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{[llvm::]*}}FunctionAnalysisManager, {{[llvm::]*}}Module>
; CHECK-NEXT: Running pass: SGLoopConstructPass
; CHECK-NEXT: Running pass: VerifierPass

; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: SplitBBonBarrier
; CHECK-NEXT: Running pass: ReduceCrossBarrierValuesPass
; CHECK:      Running pass: VerifierPass
; CHECK-NEXT: Running pass: KernelBarrier
; CHECK-NEXT: Running pass: VerifierPass

; CHECK:      Running pass: PromotePass

; CHECK:      Running pass: LoopSimplifyPass

; CHECK:      Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass

; CHECK:      Running analysis: SYCLAliasAnalysis

; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: BuiltinLICMPass
; CHECK:      Running pass: LoopStridedCodeMotionPass
; CHECK-NEXT: Running analysis: LoopWIAnalysis

; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: BuiltinLICMPass
; CHECK:      Running pass: LoopStridedCodeMotionPass
; CHECK-NEXT: Running analysis: LoopWIAnalysis

; CHECK:      Running pass: AddTLSGlobalsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK:      Running pass: LocalBuffersPass
; CHECK:      Running pass: VerifierPass

; CHECK:      Running pass: BuiltinImportPass

; CHECK:      Running pass: InternalizeGlobalVariablesPass

; CHECK:      Running pass: BuiltinCallToInstPass
; CHECK-NEXT: Running pass: VerifierPass

; CHECK:      Running pass: ModuleInlinerWrapperPass
; CHECK:      Running pass: InlinerPass
; CHECK:      Running pass: InlinerPass

; CHECK:      Running pass: PatchCallbackArgsPass
; CHECK:      Running pass: DeadArgumentEliminationPass
; CHECK:      Running pass: ArgumentPromotionPass
; CHECK:      Running pass: InstCombinePass

; CHECK:      Running pass: DSEPass

; CHECK:      Running pass: ADCEPass
; CHECK-NEXT: Running pass: SimplifyCFGPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running pass: PromotePass

; CHECK:      Running pass: LoopSimplifyPass

; CHECK:      Running pass: LCSSAPass

; CHECK:      Running pass: LICMPass
; CHECK-NEXT: Running pass: LoopIdiomRecognizePass
; CHECK-NEXT: Running pass: LoopDeletionPass
; CHECK-NEXT: Running pass: LICMPass
; CHECK-NEXT: Running pass: LoopIdiomRecognizePass
; CHECK-NEXT: Running pass: LoopDeletionPass
; CHECK-NEXT: Running pass: PrepareKernelArgsPass
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Invalidating analysis: ImplicitArgsAnalysis

; CHECK:      Running pass: GlobalDCEPass
; CHECK:      Running pass: DCEPass

; CHECK:      Running pass: SimplifyCFGPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running analysis: SYCLAliasAnalysis

; CHECK:      Running pass: DSEPass

; CHECK:      Running pass: EarlyCSEPass
; CHECK-NEXT: Running pass: GVNPass

; CHECK:      Running pass: DCEPass

; CHECK:      Running pass: SimplifyCFGPass

; CHECK:      Running pass: InstCombinePass

; CHECK:      Running analysis: SYCLAliasAnalysis

; CHECK:      Running pass: DSEPass

; CHECK:      Running pass: EarlyCSEPass
; CHECK-NEXT: Running pass: GVNPass

; CHECK:      Running pass: VerifierPass

; CHECK:      Running pass: CleanupWrappedKernelPass

; CHECK:      Running pass: LoopUnrollPass

; CHECK: Test program was successfully built.
