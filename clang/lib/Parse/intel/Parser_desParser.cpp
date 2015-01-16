
if (getLangOpts().Intel) {
  // #pragma ivdep
  PP.RemovePragmaHandler(IvdepHandler.get());
  IvdepHandler.reset();
  // #pragma novector
  PP.RemovePragmaHandler(NoVectorHandler.get());
  NoVectorHandler.reset();
  // #pragma vector
  PP.RemovePragmaHandler(VectorHandler.get());
  VectorHandler.reset();
  // #pragma distribute_point
  PP.RemovePragmaHandler(DistributeHandler.get());
  DistributeHandler.reset();
  PP.RemovePragmaHandler(DistributeHandler1.get());
  DistributeHandler1.reset();
  // #pragma inline
  PP.RemovePragmaHandler(InlineHandler.get());
  InlineHandler.reset();
  // #pragma noinline
  PP.RemovePragmaHandler(NoInlineHandler.get());
  NoInlineHandler.reset();
  // #pragma forceinline
  PP.RemovePragmaHandler(ForceInlineHandler.get());
  ForceInlineHandler.reset();
  // #pragma loop_count
  PP.RemovePragmaHandler(LoopCountHandler.get());
  LoopCountHandler.reset();
  PP.RemovePragmaHandler(LoopCountHandler1.get());
  LoopCountHandler1.reset();
  // #pragma optimize
  PP.RemovePragmaHandler(IntelOptimizeHandler.get());
  IntelOptimizeHandler.reset();
  // #pragma optimization_level
  PP.RemovePragmaHandler((getLangOpts().PragmaOptimizationLevelIntel == 1)
                             ? OptimizationLevelHandler.get()
                             : GCCOptimizationLevelHandler.get());
  PP.RemovePragmaHandler("intel", OptimizationLevelHandler.get());
  OptimizationLevelHandler.reset();
  PP.RemovePragmaHandler("GCC", GCCOptimizationLevelHandler.get());
  GCCOptimizationLevelHandler.reset();
  // #pragma noparallel
  PP.RemovePragmaHandler(NoParallelHandler.get());
  NoParallelHandler.reset();
  // #pragma parallel
  PP.RemovePragmaHandler(ParallelHandler.get());
  ParallelHandler.reset();
  // #pragma nounroll
  PP.RemovePragmaHandler(NoUnrollHandler.get());
  NoUnrollHandler.reset();
  // #pragma unroll
  PP.RemovePragmaHandler(UnrollHandler.get());
  UnrollHandler.reset();
  // #pragma nounroll_and_jam
  PP.RemovePragmaHandler(NoUnrollAndJamHandler.get());
  NoUnrollAndJamHandler.reset();
  // #pragma unroll_and_jam
  PP.RemovePragmaHandler(UnrollAndJamHandler.get());
  UnrollAndJamHandler.reset();
  // #pragma nofusion
  PP.RemovePragmaHandler(NoFusionHandler.get());
  NoFusionHandler.reset();
  // #pragma ident
  PP.RemovePragmaHandler(IdentHandler.get());
  IdentHandler.reset();
  // #pragma optimization_parameter
  PP.RemovePragmaHandler("intel", OptimizationParameterHandler.get());
  OptimizationParameterHandler.reset();
  // #pragma alloc_section
  PP.RemovePragmaHandler(AllocSectionHandler.get());
  AllocSectionHandler.reset();
  // #pragma section
  PP.RemovePragmaHandler(SectionHandler.get());
  SectionHandler.reset();
  // #pragma alloc_text
  PP.RemovePragmaHandler(AllocTextHandler.get());
  AllocTextHandler.reset();
  // #pragma auto_inline
  PP.RemovePragmaHandler(AutoInlineHandler.get());
  AutoInlineHandler.reset();
  // #pragma bss_seg|code_seg|const_seg|data_seg
  PP.RemovePragmaHandler(BssSegHandler.get());
  BssSegHandler.reset();
  PP.RemovePragmaHandler(CodeSegHandler.get());
  CodeSegHandler.reset();
  PP.RemovePragmaHandler(ConstSegHandler.get());
  ConstSegHandler.reset();
  PP.RemovePragmaHandler(DataSegHandler.get());
  DataSegHandler.reset();
  // #pragma check_stack
  PP.RemovePragmaHandler(CheckStackHandler.get());
  CheckStackHandler.reset();
  // #pragma component
  PP.RemovePragmaHandler(ComponentHandler.get());
  ComponentHandler.reset();
  // #pragma conform
  PP.RemovePragmaHandler(ConformHandler.get());
  ConformHandler.reset();
  // #pragma deprecated
  PP.RemovePragmaHandler(DeprecatedHandler.get());
  DeprecatedHandler.reset();
  // #pragma fp_contract
  PP.RemovePragmaHandler(IntelFPContractHandler.get());
  IntelFPContractHandler.reset();
  // #pragma fenv_access
  PP.RemovePragmaHandler(IntelFenvAccessHandler.get());
  IntelFenvAccessHandler.reset();
  // #pragma init_seg
  PP.RemovePragmaHandler(InitSegHandler.get());
  InitSegHandler.reset();
  // #pragma float_control
  FCVector.clear();
  PP.RemovePragmaHandler(FloatControlHandler.get());
  FloatControlHandler.reset();
  // #pragma region
  static_cast<PragmaRegionHandler *>(RegionHandler.get())
      ->CheckOpenedRegions(PP);
  PP.RemovePragmaHandler(RegionHandler.get());
  RegionHandler.reset();
  // #pragma endregion
  PP.RemovePragmaHandler(EndRegionHandler.get());
  EndRegionHandler.reset();
  // #pragma start_map_region
  PP.RemovePragmaHandler(StartMapRegionHandler.get());
  StartMapRegionHandler.reset();
  // #pragma stop_map_region
  PP.RemovePragmaHandler(StopMapRegionHandler.get());
  StopMapRegionHandler.reset();
  // #pragma vtordisp
  PP.RemovePragmaHandler(VtorDispHandler.get());
  VtorDispHandler.reset();
}

if (getLangOpts().CilkPlus) {
  PP.RemovePragmaHandler(CilkGrainsizeHandler.get());
  CilkGrainsizeHandler.reset();
  PP.RemovePragmaHandler(SIMDHandler.get());
  SIMDHandler.reset(new PragmaSIMDHandler());
}
                    