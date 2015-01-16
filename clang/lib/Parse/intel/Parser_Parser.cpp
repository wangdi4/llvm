
if (getLangOpts().Intel) {
  // #pragma ivdep
  IvdepHandler.reset(new PragmaIvdepHandler());
  PP.AddPragmaHandler(IvdepHandler.get());
  // #pragma novector
  NoVectorHandler.reset(new PragmaNoVectorHandler());
  PP.AddPragmaHandler(NoVectorHandler.get());
  // #pragma vector
  VectorHandler.reset(new PragmaVectorHandler());
  PP.AddPragmaHandler(VectorHandler.get());
  // #pragma distribute_point
  DistributeHandler.reset(new PragmaDistributeHandler());
  PP.AddPragmaHandler(DistributeHandler.get());
  DistributeHandler1.reset(new PragmaDistributeHandler1());
  PP.AddPragmaHandler(DistributeHandler1.get());
  // #pragma inline
  InlineHandler.reset(new PragmaInlineHandler());
  PP.AddPragmaHandler(InlineHandler.get());
  // #pragma noinline
  NoInlineHandler.reset(new PragmaNoInlineHandler());
  PP.AddPragmaHandler(NoInlineHandler.get());
  // #pragma forceinline
  ForceInlineHandler.reset(new PragmaForceInlineHandler());
  PP.AddPragmaHandler(ForceInlineHandler.get());
  // #pragma loop_count
  LoopCountHandler.reset(new PragmaLoopCountHandler());
  PP.AddPragmaHandler(LoopCountHandler.get());
  LoopCountHandler1.reset(new PragmaLoopCountHandler1());
  PP.AddPragmaHandler(LoopCountHandler1.get());
  // #pragma optimize
  IntelOptimizeHandler.reset(new PragmaIntelOptimizeHandler());
  PP.AddPragmaHandler(IntelOptimizeHandler.get());
  // #pragma optimization_level
  OptimizationLevelHandler.reset(new PragmaOptimizationLevelHandler(true));
  PP.AddPragmaHandler("intel", OptimizationLevelHandler.get());
  GCCOptimizationLevelHandler.reset(new PragmaOptimizationLevelHandler(false));
  PP.AddPragmaHandler("GCC", GCCOptimizationLevelHandler.get());
  PP.AddPragmaHandler((getLangOpts().PragmaOptimizationLevelIntel == 1)
                          ? OptimizationLevelHandler.get()
                          : GCCOptimizationLevelHandler.get());
  // #pragma noparallel
  NoParallelHandler.reset(new PragmaNoParallelHandler());
  PP.AddPragmaHandler(NoParallelHandler.get());
  // #pragma parallel
  ParallelHandler.reset(new PragmaParallelHandler());
  PP.AddPragmaHandler(ParallelHandler.get());
  // #pragma nounroll
  NoUnrollHandler.reset(new PragmaNoUnrollHandler());
  PP.AddPragmaHandler(NoUnrollHandler.get());
  // #pragma unroll
  UnrollHandler.reset(new PragmaUnrollHandler());
  PP.AddPragmaHandler(UnrollHandler.get());
  // #pragma nounroll_and_jam
  NoUnrollAndJamHandler.reset(new PragmaNoUnrollAndJamHandler());
  PP.AddPragmaHandler(NoUnrollAndJamHandler.get());
  // #pragma unroll_and_jam
  UnrollAndJamHandler.reset(new PragmaUnrollAndJamHandler());
  PP.AddPragmaHandler(UnrollAndJamHandler.get());
  // #pragma nofusion
  NoFusionHandler.reset(new PragmaNoFusionHandler());
  PP.AddPragmaHandler(NoFusionHandler.get());
  // #pragma ident
  IdentHandler.reset(new PragmaIdentHandler());
  PP.AddPragmaHandler(IdentHandler.get());
  // #pragma optimization_parameter
  OptimizationParameterHandler.reset(new PragmaOptimizationParameterHandler());
  PP.AddPragmaHandler("intel", OptimizationParameterHandler.get());
  // #pragma alloc_section
  AllocSectionHandler.reset(new PragmaAllocSectionHandler());
  PP.AddPragmaHandler(AllocSectionHandler.get());
  // #pragma section
  SectionHandler.reset(new PragmaSectionHandler());
  PP.AddPragmaHandler(SectionHandler.get());
  // #pragma alloc_text
  AllocTextHandler.reset(new PragmaAllocTextHandler());
  PP.AddPragmaHandler(AllocTextHandler.get());
  // #pragma auto_inline
  AutoInlineHandler.reset(new PragmaAutoInlineHandler());
  PP.AddPragmaHandler(AutoInlineHandler.get());
  // #pragma bss_seg|code_seg|const_seg|data_seg
  BssSegHandler.reset(new PragmaBssSegHandler());
  PP.AddPragmaHandler(BssSegHandler.get());
  CodeSegHandler.reset(new PragmaCodeSegHandler());
  PP.AddPragmaHandler(CodeSegHandler.get());
  ConstSegHandler.reset(new PragmaConstSegHandler());
  PP.AddPragmaHandler(ConstSegHandler.get());
  DataSegHandler.reset(new PragmaDataSegHandler());
  PP.AddPragmaHandler(DataSegHandler.get());
  // #pragma check_stack
  CheckStackHandler.reset(new PragmaCheckStackHandler());
  PP.AddPragmaHandler(CheckStackHandler.get());
  // #pragma component
  ComponentHandler.reset(new PragmaComponentHandler());
  PP.AddPragmaHandler(ComponentHandler.get());
  // #pragma conform
  ConformHandler.reset(new PragmaConformHandler());
  PP.AddPragmaHandler(ConformHandler.get());
  // #pragma deprecated
  DeprecatedHandler.reset(new PragmaDeprecatedHandler());
  PP.AddPragmaHandler(DeprecatedHandler.get());
  // #pragma fp_contract
  IntelFPContractHandler.reset(new PragmaIntelFPContractHandler());
  PP.AddPragmaHandler(IntelFPContractHandler.get());
  // #pragma fenv_access
  IntelFenvAccessHandler.reset(new PragmaIntelFenvAccessHandler());
  PP.AddPragmaHandler(IntelFenvAccessHandler.get());
  // #pragma init_seg
  InitSegHandler.reset(new PragmaInitSegHandler());
  PP.AddPragmaHandler(InitSegHandler.get());
  // #pragma float_control
  FPState = getLangOpts().getFPModel();
  FCVector.clear();
  FCVector.push_back(FPState);
  FloatControlHandler.reset(new PragmaFloatControlHandler());
  PP.AddPragmaHandler(FloatControlHandler.get());
  // Apply default fp options
  if (FPState != (LangOptions::IFP_Fast | LangOptions::IFP_FP_Contract)) {
    Actions.ActOnPragmaOptionsFloatControl(SourceLocation(), FPState);
    if (FPState == (LangOptions::IFP_Precise | LangOptions::IFP_FEnv_Access |
                    LangOptions::IFP_Except | LangOptions::IFP_ValueSafety)) {
      Actions.ActOnPragmaCommonOnOff(SourceLocation(), "fp_contract",
                                     "FP_CONTRACT", Sema::IntelCommonOff,
                                     Sema::IntelPragmaFPContract, FPState);
      Actions.ActOnPragmaCommonOnOff(SourceLocation(), "fenv_access",
                                     "FENV_ACCESS", Sema::IntelCommonOn,
                                     Sema::IntelPragmaFEnvAccess, FPState);
    }
  }
  // #pragma region
  RegionHandler.reset(new PragmaRegionHandler());
  PP.AddPragmaHandler(RegionHandler.get());
  // #pragma endregion
  EndRegionHandler.reset(new PragmaEndRegionHandler());
  PP.AddPragmaHandler(EndRegionHandler.get());
  // #pragma start_map_region
  PragmaStartMapRegionHandler *hndlr = new PragmaStartMapRegionHandler();
  StartMapRegionHandler.reset(hndlr);
  PP.AddPragmaHandler(StartMapRegionHandler.get());
  // #pragma stop_map_region
  StopMapRegionHandler.reset(
      new PragmaStopMapRegionHandler(hndlr->RegionStarted));
  PP.AddPragmaHandler(StopMapRegionHandler.get());
  // #pragma vtordisp
  VtorDispHandler.reset(new PragmaVtorDispHandler());
  PP.AddPragmaHandler(VtorDispHandler.get());

  if (getLangOpts().AlignMac68k) {
    Actions.SetMac68kAlignment();
  }
}

if (getLangOpts().CilkPlus) {
  CilkGrainsizeHandler.reset(new PragmaCilkGrainsizeHandler());
  PP.AddPragmaHandler(CilkGrainsizeHandler.get());
  SIMDHandler.reset(new PragmaSIMDHandler());
  PP.AddPragmaHandler(SIMDHandler.get());
}
                    