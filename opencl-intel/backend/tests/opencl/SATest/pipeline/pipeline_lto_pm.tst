; RUN: SATest -BUILD -pass-manager-type=lto -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager=quiet -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s -check-prefix=QUIET
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager=verbose -verify-each-pass" -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s -check-prefix=VERIFY-EACH

; TODO:
;   check CoerceWin64Types pass when SATest is enabled on Windows.

; CHECK:      Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVLowerConstExprPass
; CHECK-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; CHECK-NEXT: Running pass: NameAnonGlobalPass

#ifndef NDEBUG
; CHECK-NEXT: Running pass: VerifierPass
; CHECK-NEXT: Running analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: InferAddressSpacesPass
; CHECK:      Running pass: DPCPPEqualizerPass
; CHECK-NEXT: Running analysis: BuiltinLibInfoAnalysis
; CHECK:      Running pass: DuplicateCalledKernels
; CHECK-NEXT: Running pass: InternalizeNonKernelFuncPass

#ifndef NDEBUG
; CHECK-NEXT: Invalidating analysis: VerifierAnalysis
#endif // #ifndef NDEBUG

; CHECK:      Running pass: AddFunctionAttrsPass
; CHECK-NEXT: Running pass: LinearIdResolverPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass

; CHECK:      Running pass: DPCPPKernelAnalysisPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: WGLoopBoundariesPass

; CHECK:      Running pass: ReassociatePass
; CHECK-NEXT: Running pass: InferAddressSpacesPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK:      Running pass: EarlyCSEPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK:      Running pass: SinCosFoldPass
; CHECK-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; CHECK-NEXT: Running pass: UnifyFunctionExitNodesPass
; CHECK:      Running pass: DeduceMaxWGDimPass
; CHECK-NEXT: Running pass: InstToFuncCallPass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running pass: VectorVariantLowering
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation
; CHECK-NEXT: Running pass: SGSizeCollectorPass
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::VectorizationDimensionAnalysis, llvm::Module>
; CHECK-NEXT: Running analysis: VectorizationDimensionAnalysis
; CHECK-NEXT: Running pass: DPCPPKernelVecClonePass
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Invalidating analysis: VFAnalysis
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: VectorizationDimensionAnalysis
; CHECK-NEXT: Running pass: VectorVariantFillIn
; CHECK-NEXT: Running pass: UpdateCallAttrs

; CHECK:      Running pass: PromotePass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass

; CHECK-NOT:  Running pass: VecClonePass
; CHECK:      Running pass: vpo::VPlanDriverPass

; CHECK:      Running pass: DPCPPKernelPostVecPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: ADCEPass
; CHECK:      Running pass: HandleVPlanMask

; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK-NEXT: Running pass: DPCPPKernelWGLoopCreatorPass
; CHECK:      Running pass: LoopUnrollPass
; CHECK:      Running pass: IndirectCallLowering
; CHECK:      Running pass: ResolveSubGroupWICallPass

; CHECK:      Running pass: PhiCanonicalization
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK:      Running pass: Intel Kernel RedundantPhiNode
; CHECK:      Running pass: GroupBuiltinPass
; CHECK-NEXT: Running pass: Intel Kernel BarrierInFunction
; CHECK:      Running pass: ResolveSubGroupWICallPass
; CHECK:      Running pass: Intel Kernel SplitBBonBarrier
; CHECK-NEXT: Running pass: ReduceCrossBarrierValues
; CHECK-NEXT: Running analysis: Intel Kernel DataPerValue Analysis
; CHECK-NEXT: Running analysis: Intel Kernel DataPerBarrier Analysis
; CHECK-NEXT: Running analysis: Intel Kernel WIRelatedValue
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; CHECK-NEXT: Running analysis: DominanceFrontierAnalysis
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK:      Running pass: Intel Kernel Barrier

; CHECK:      Running pass: AddImplicitArgsPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: ImplicitArgsAnalysis
; CHECK:      Running pass: ResolveWICallPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: LocalBuffersPass
; CHECK-NEXT: Running analysis: LocalBufferAnalysis
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK:      Running pass: BuiltinImportPass
; CHECK:      Running analysis: InnerAnalysisManagerProxy
; CHECK-NEXT: Running pass: BuiltinCallToInstPass
; CHECK:      Running pass: InlinerPass
; CHECK:      Running pass: DeadArgumentEliminationPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: LoopSimplifyPass
; CHECK:      Running pass: LICMPass
; CHECK:      Running pass: LoopIdiomRecognizePass
; CHECK:      Running pass: LoopDeletionPass
; CHECK:      Running pass: PrepareKernelArgsPass
; CHECK:      Running pass: SimplifyCFGPass
; CHECK:      Running pass: SROAPass
; CHECK:      Running pass: InstCombinePass
; CHECK:      Running pass: GVNPass
; CHECK:      Running pass: DSEPass
; CHECK:      Running pass: ADCEPass
; CHECK:      Running pass: EarlyCSEPass
; CHECK:      Running pass: CleanupWrappedKernelPass


; QUIET:      Running pass: XmainOptLevelAnalysisInit
; QUIET-NEXT: Running pass: Annotation2MetadataPass
; QUIET-NEXT: Running pass: ForceFunctionAttrsPass
; QUIET-NEXT: Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; QUIET-NEXT: Running pass: SPIRV::SPIRVLowerConstExprPass
; QUIET-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; QUIET-NEXT: Running pass: NameAnonGlobalPass
; QUIET-NEXT: Running pass: VerifierPass
; QUIET-NEXT: Running pass: InferAddressSpacesPass
; QUIET-NEXT: Running pass: DPCPPEqualizerPass
; QUIET-NEXT: Running pass: DuplicateCalledKernels
; QUIET-NEXT: Running pass: InternalizeNonKernelFuncPass
; QUIET-NEXT: Running pass: AddFunctionAttrsPass
; QUIET-NEXT: Running pass: LinearIdResolverPass
; QUIET-NEXT: Running pass: BuiltinCallToInstPass
; QUIET-NEXT: Running pass: InferFunctionAttrsPass
; QUIET-NEXT: Running pass: InlineReportSetupPass
; QUIET-NEXT: Running pass: InlineListsPass
; QUIET-NEXT: Running pass: CoroEarlyPass
; QUIET-NEXT: Running pass: LowerSubscriptIntrinsicPass
; QUIET-NEXT: Running pass: LowerExpectIntrinsicPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: CallSiteSplittingPass
; QUIET-NEXT: Running pass: OpenMPOptPass
; QUIET-NEXT: Running pass: IPSCCPPass
; QUIET-NEXT: Running pass: CalledValuePropagationPass
; QUIET-NEXT: Running pass: GlobalOptPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: DeadArgumentEliminationPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: JumpThreadingPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: ModuleInlinerWrapperPass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::GlobalsAA, llvm::Module>
; QUIET-NEXT: Running pass: InvalidateAnalysisPass<llvm::AAManager>
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis, llvm::Module>
; QUIET-NEXT: Running pass: DevirtSCCRepeatedPass
; QUIET-NEXT: Running pass: InlinerPass
; QUIET-NEXT: Running pass: InlinerPass
; QUIET-NEXT: Running pass: ArgumentPromotionPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: PostOrderFunctionAttrsPass
; QUIET-NEXT: Running pass: OpenMPOptCGSCCPass
; QUIET-NEXT: Running pass: TbaaMDPropagationPass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::OptReportOptionsAnalysis, llvm::Function>
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: SpeculativeExecutionPass
; QUIET-NEXT: Running pass: JumpThreadingPass
; QUIET-NEXT: Running pass: CorrelatedValuePropagationPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: AggressiveInstCombinePass
; QUIET-NEXT: Running pass: LibCallsShrinkWrapPass
; QUIET-NEXT: Running pass: TailCallElimPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: ReassociatePass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: MergedLoadStoreMotionPass
; QUIET-NEXT: Running pass: GVNPass
; QUIET-NEXT: Running pass: SCCPPass
; QUIET-NEXT: Running pass: BDCEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: JumpThreadingPass
; QUIET-NEXT: Running pass: CorrelatedValuePropagationPass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: MemCpyOptPass
; QUIET-NEXT: Running pass: DSEPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: CoroElidePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: TransformSinAndCosCallsPass
; QUIET-NEXT: Running pass: CoroSplitPass
; QUIET-NEXT: Running pass: CoroCleanupPass
; QUIET-NEXT: Running pass: GlobalOptPass
; QUIET-NEXT: Running pass: GlobalDCEPass
; QUIET-NEXT: Running pass: StdContainerOptPass
; QUIET-NEXT: Running pass: CleanupFakeLoadsPass
; QUIET-NEXT: Running pass: EliminateAvailableExternallyPass
; QUIET-NEXT: Running pass: ArgNoAliasPropPass
; QUIET-NEXT: Running pass: ReversePostOrderFunctionAttrsPass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::AndersensAA, llvm::Module>
; QUIET-NEXT: Running pass: NonLTOGlobalOptPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: RecomputeGlobalsAAPass
; QUIET-NEXT: Running pass: DPCPPKernelAnalysisPass
; QUIET-NEXT: Running pass: WGLoopBoundariesPass
; QUIET-NEXT: Running pass: DCEPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: ReassociatePass
; QUIET-NEXT: Running pass: InferAddressSpacesPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SinCosFoldPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: UnifyFunctionExitNodesPass
; QUIET-NEXT: Running pass: DCEPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: ReassociatePass
; QUIET-NEXT: Running pass: InferAddressSpacesPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SinCosFoldPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: UnifyFunctionExitNodesPass
; QUIET-NEXT: Running pass: DeduceMaxWGDimPass
; QUIET-NEXT: Running pass: InstToFuncCallPass
; QUIET-NEXT: Running pass: SetVectorizationFactorPass
; QUIET-NEXT: Running pass: VectorVariantLowering
; QUIET-NEXT: Running pass: CreateSimdVariantPropagation
; QUIET-NEXT: Running pass: SGSizeCollectorPass
; QUIET-NEXT: Running pass: SGSizeCollectorIndirectPass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::VectorizationDimensionAnalysis, llvm::Module>
; QUIET-NEXT: Running pass: DPCPPKernelVecClonePass
; QUIET-NEXT: Running pass: VectorVariantFillIn
; QUIET-NEXT: Running pass: UpdateCallAttrs
; QUIET-NEXT: Running pass: Float2IntPass
; QUIET-NEXT: Running pass: LowerConstantIntrinsicsPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: VPlanPragmaOmpSimdIfPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LowerSwitchPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: VPOCFGRestructuringPass
; QUIET-NEXT: Running pass: Float2IntPass
; QUIET-NEXT: Running pass: LowerConstantIntrinsicsPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: VPlanPragmaOmpSimdIfPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LowerSwitchPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: VPOCFGRestructuringPass
; QUIET-NEXT: Running pass: Float2IntPass
; QUIET-NEXT: Running pass: LowerConstantIntrinsicsPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: LICMPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: LoopRotatePass
; QUIET-NEXT: Running pass: LoopDeletionPass
; QUIET-NEXT: Running pass: VPlanPragmaOmpSimdIfPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LowerSwitchPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: VPOCFGRestructuringPass
; QUIET-NEXT: Running pass: VPlanPragmaOmpOrderedSimdExtractPass
; QUIET-NEXT: Running pass: VPOCFGRestructuringPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: vpo::VPlanDriverPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: VPOCFGRestructuringPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: vpo::VPlanDriverPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: VPOCFGRestructuringPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: vpo::VPlanDriverPass
; QUIET-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; QUIET-NEXT: Running pass: AlwaysInlinerPass
; QUIET-NEXT: Running pass: VPODirectiveCleanupPass
; QUIET-NEXT: Running pass: LoopDistributePass
; QUIET-NEXT: Running pass: InjectTLIMappings
; QUIET-NEXT: Running pass: LoopLoadEliminationPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: VectorCombinePass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: LoopUnrollPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: WarnMissedTransformationsPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: AlignmentFromAssumptionsPass
; QUIET-NEXT: Running pass: LoopSinkPass
; QUIET-NEXT: Running pass: InstSimplifyPass
; QUIET-NEXT: Running pass: DivRemPairsPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: VPODirectiveCleanupPass
; QUIET-NEXT: Running pass: LoopDistributePass
; QUIET-NEXT: Running pass: InjectTLIMappings
; QUIET-NEXT: Running pass: LoopLoadEliminationPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: VectorCombinePass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: LoopUnrollPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: WarnMissedTransformationsPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: AlignmentFromAssumptionsPass
; QUIET-NEXT: Running pass: LoopSinkPass
; QUIET-NEXT: Running pass: InstSimplifyPass
; QUIET-NEXT: Running pass: DivRemPairsPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: VPODirectiveCleanupPass
; QUIET-NEXT: Running pass: LoopDistributePass
; QUIET-NEXT: Running pass: InjectTLIMappings
; QUIET-NEXT: Running pass: LoopLoadEliminationPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: VectorCombinePass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: LoopUnrollPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: WarnMissedTransformationsPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: AlignmentFromAssumptionsPass
; QUIET-NEXT: Running pass: LoopSinkPass
; QUIET-NEXT: Running pass: InstSimplifyPass
; QUIET-NEXT: Running pass: DivRemPairsPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: DPCPPKernelPostVecPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: PromotePass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: HandleVPlanMask
; QUIET-NEXT: Running pass: ResolveSubGroupWICallPass
; QUIET-NEXT: Running pass: DPCPPKernelWGLoopCreatorPass
; QUIET-NEXT: Running pass: LoopUnrollPass
; QUIET-NEXT: Running pass: LoopUnrollPass
; QUIET-NEXT: Running pass: IndirectCallLowering
; QUIET-NEXT: Running pass: ResolveSubGroupWICallPass
; QUIET-NEXT: Running pass: PhiCanonicalization
; QUIET-NEXT: Running pass: Intel Kernel RedundantPhiNode
; QUIET-NEXT: Running pass: PhiCanonicalization
; QUIET-NEXT: Running pass: Intel Kernel RedundantPhiNode
; QUIET-NEXT: Running pass: GroupBuiltinPass
; QUIET-NEXT: Running pass: Intel Kernel BarrierInFunction
; QUIET-NEXT: Running pass: ResolveSubGroupWICallPass
; QUIET-NEXT: Running pass: Intel Kernel SplitBBonBarrier
; QUIET-NEXT: Running pass: ReduceCrossBarrierValuesPass
; QUIET-NEXT: Running pass: Intel Kernel Barrier
; QUIET-NEXT: Running pass: AddImplicitArgsPass
; QUIET-NEXT: Running pass: ResolveWICallPass
; QUIET-NEXT: Running pass: LocalBuffersPass
; QUIET-NEXT: Running pass: BuiltinImportPass
; QUIET-NEXT: Running pass: GlobalDCEPass
; QUIET-NEXT: Running pass: BuiltinCallToInstPass
; QUIET-NEXT: Running pass: BuiltinCallToInstPass
; QUIET-NEXT: Running pass: ModuleInlinerWrapperPass
; QUIET-NEXT: Running pass: InlinerPass
; QUIET-NEXT: Running pass: InlinerPass
; QUIET-NEXT: Running pass: InlinerPass
; QUIET-NEXT: Running pass: InlinerPass
; QUIET-NEXT: Running pass: DeadArgumentEliminationPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LoopSimplifyPass
; QUIET-NEXT: Running pass: LCSSAPass
; QUIET-NEXT: Running pass: LICMPass
; QUIET-NEXT: Running pass: LoopIdiomRecognizePass
; QUIET-NEXT: Running pass: LoopDeletionPass
; QUIET-NEXT: Running pass: LoopStridedCodeMotionPass
; QUIET-NEXT: Running pass: LICMPass
; QUIET-NEXT: Running pass: LoopIdiomRecognizePass
; QUIET-NEXT: Running pass: LoopDeletionPass
; QUIET-NEXT: Running pass: LoopStridedCodeMotionPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: PrepareKernelArgsPass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: GVNPass
; QUIET-NEXT: Running pass: DSEPass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: SimplifyCFGPass
; QUIET-NEXT: Running pass: SROAPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: GVNPass
; QUIET-NEXT: Running pass: DSEPass
; QUIET-NEXT: Running pass: ADCEPass
; QUIET-NEXT: Running pass: EarlyCSEPass
; QUIET-NEXT: Running pass: InstCombinePass
; QUIET-NEXT: Running pass: CGProfilePass
; QUIET-NEXT: Running pass: GlobalDCEPass
; QUIET-NEXT: Running pass: ConstantMergePass
; QUIET-NEXT: Running pass: RelLookupTableConverterPass
; QUIET-NEXT: Running pass: InlineReportEmitterPass
; QUIET-NEXT: Running pass: AnnotationRemarksPass
; QUIET-NEXT: Running pass: AnnotationRemarksPass
; QUIET-NEXT: Running pass: CleanupWrappedKernelPass
; QUIET-NEXT: Test program was successfully built.


; VERIFY-EACH:      Running pass: XmainOptLevelAnalysisInit
; VERIFY-EACH-NEXT: Running analysis: XmainOptLevelAnalysis
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: Annotation2MetadataPass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: ForceFunctionAttrsPass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: DPCPPPreprocessSPIRVFriendlyIRPass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: SPIRV::SPIRVLowerConstExprPass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: SPIRV::SPIRVToOCL20Pass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: NameAnonGlobalPass
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: VerifierPass
; VERIFY-EACH-NEXT: Running analysis: VerifierAnalysis
; VERIFY-EACH-NEXT: Running pass: ModuleToFunctionPassAdaptor
; VERIFY-EACH-NEXT: Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
; VERIFY-EACH-NEXT: Running pass: InferAddressSpacesPass
; VERIFY-EACH-NEXT: Running analysis: AssumptionAnalysis
; VERIFY-EACH-NEXT: Running analysis: TargetIRAnalysis
; VERIFY-EACH-NEXT: Verifying function test
; VERIFY-EACH-NEXT: Running pass: DPCPPEqualizerPass
; VERIFY-EACH-NEXT: Running analysis: BuiltinLibInfoAnalysis
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: DuplicateCalledKernels
; VERIFY-EACH-NEXT: Verifying module main
; VERIFY-EACH-NEXT: Running pass: InternalizeNonKernelFuncPass
; VERIFY-EACH-NEXT: Verifying module main
