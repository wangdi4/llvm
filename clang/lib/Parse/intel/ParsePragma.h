class PragmaSIMDHandler : public PragmaHandler {
  public:
    explicit PragmaSIMDHandler() : PragmaHandler("simd") {}

    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                                Token &FirstToken);
};
class PragmaCilkGrainsizeHandler : public PragmaHandler {
  public:
    PragmaCilkGrainsizeHandler() : PragmaHandler("cilk") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                              Token &FirstToken);
};

#ifdef INTEL_SPECIFIC_IL0_BACKEND
// #pragma ivdep
class PragmaIvdepHandler: public PragmaHandler {
  public:
    explicit PragmaIvdepHandler() : PragmaHandler("ivdep") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma novector
class PragmaNoVectorHandler: public PragmaHandler {
  public:
    explicit PragmaNoVectorHandler() : PragmaHandler("novector") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma distribute_point
class PragmaDistributeHandler: public PragmaHandler {
  public:
    explicit PragmaDistributeHandler() : PragmaHandler("distribute_point") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma distribute point
class PragmaDistributeHandler1: public PragmaHandler {
  public:
    explicit PragmaDistributeHandler1() : PragmaHandler("distribute") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma inline
class PragmaInlineHandler: public PragmaHandler {
  public:
    explicit PragmaInlineHandler() : PragmaHandler("inline") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma forceinline
class PragmaForceInlineHandler: public PragmaHandler {
  public:
    explicit PragmaForceInlineHandler() : PragmaHandler("forceinline") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma noinline
class PragmaNoInlineHandler: public PragmaHandler {
  public:
    explicit PragmaNoInlineHandler() : PragmaHandler("noinline") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma loop_count
class PragmaLoopCountHandler: public PragmaHandler {
  public:
    explicit PragmaLoopCountHandler() : PragmaHandler("loop_count") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
class PragmaLoopCountHandler1: public PragmaHandler {
  public:
    explicit PragmaLoopCountHandler1() : PragmaHandler("loop") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma optimize
class PragmaIntelOptimizeHandler: public PragmaHandler {
  public:
    explicit PragmaIntelOptimizeHandler() : PragmaHandler("optimize") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma optimization_level
class PragmaOptimizationLevelHandler: public PragmaHandler {
  private:
    bool IsIntelPragma;
  public:
    explicit PragmaOptimizationLevelHandler(bool IntelPragma) : PragmaHandler("optimization_level"), IsIntelPragma(IntelPragma) {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma noparallel
class PragmaNoParallelHandler: public PragmaHandler {
  public:
    explicit PragmaNoParallelHandler() : PragmaHandler("noparallel") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma parallel
class PragmaParallelHandler: public PragmaHandler {
  public:
    explicit PragmaParallelHandler() : PragmaHandler("parallel") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma unroll
class PragmaUnrollHandler: public PragmaHandler {
  public:
    explicit PragmaUnrollHandler() : PragmaHandler("unroll") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma nounroll
class PragmaNoUnrollHandler: public PragmaHandler {
  public:
    explicit PragmaNoUnrollHandler() : PragmaHandler("nounroll") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma unroll_and_jam
class PragmaUnrollAndJamHandler: public PragmaHandler {
  public:
    explicit PragmaUnrollAndJamHandler() : PragmaHandler("unroll_and_jam") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma nounroll_and_jam
class PragmaNoUnrollAndJamHandler: public PragmaHandler {
  public:
    explicit PragmaNoUnrollAndJamHandler() : PragmaHandler("nounroll_and_jam") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma nofusion
class PragmaNoFusionHandler: public PragmaHandler {
  public:
    explicit PragmaNoFusionHandler() : PragmaHandler("nofusion") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma ident
class PragmaIdentHandler: public PragmaHandler {
  public:
    explicit PragmaIdentHandler() : PragmaHandler("ident") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma vector
class PragmaVectorHandler: public PragmaHandler {
  public:
    explicit PragmaVectorHandler() : PragmaHandler("vector") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma intel optimization_parameter
class PragmaOptimizationParameterHandler: public PragmaHandler {
  public:
    explicit PragmaOptimizationParameterHandler() : PragmaHandler("optimization_parameter") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma alloc_section
class PragmaAllocSectionHandler: public PragmaHandler {
  public:
    explicit PragmaAllocSectionHandler() : PragmaHandler("alloc_section") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma section
class PragmaSectionHandler: public PragmaHandler {
  public:
    explicit PragmaSectionHandler() : PragmaHandler("section") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma alloc_text
class PragmaAllocTextHandler: public PragmaHandler {
  public:
    explicit PragmaAllocTextHandler() : PragmaHandler("alloc_text") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma auto_inline
class PragmaAutoInlineHandler: public PragmaHandler {
  public:
    explicit PragmaAutoInlineHandler() : PragmaHandler("auto_inline") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma bss_seg
class PragmaBssSegHandler: public PragmaHandler {
  public:
    explicit PragmaBssSegHandler() : PragmaHandler("bss_seg") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma data_seg
class PragmaDataSegHandler: public PragmaHandler {
  public:
    explicit PragmaDataSegHandler() : PragmaHandler("data_seg") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma const_seg
class PragmaConstSegHandler: public PragmaHandler {
  public:
    explicit PragmaConstSegHandler() : PragmaHandler("const_seg") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma code_seg
class PragmaCodeSegHandler: public PragmaHandler {
  public:
    explicit PragmaCodeSegHandler() : PragmaHandler("code_seg") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma check_stack
class PragmaCheckStackHandler: public PragmaHandler {
  public:
    explicit PragmaCheckStackHandler() : PragmaHandler("check_stack") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma component
class PragmaComponentHandler: public PragmaHandler {
  public:
    explicit PragmaComponentHandler() : PragmaHandler("component") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma conform
class PragmaConformHandler: public PragmaHandler {
  private:
    void Push_forScope(Preprocessor &PP, Token &Tok);
    void Pop_forScope(Preprocessor &PP, Token &Tok);
  public:
    explicit PragmaConformHandler() : PragmaHandler("conform") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma deprecated
class PragmaDeprecatedHandler: public PragmaHandler {
  public:
    explicit PragmaDeprecatedHandler() : PragmaHandler("deprecated") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma fp_contract
class PragmaIntelFPContractHandler: public PragmaHandler {
  public:
    explicit PragmaIntelFPContractHandler() : PragmaHandler("fp_contract") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
class PragmaIntelFenvAccessHandler: public PragmaHandler {
  public:
    explicit PragmaIntelFenvAccessHandler() : PragmaHandler("fenv_access") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma init_seg
class PragmaInitSegHandler: public PragmaHandler {
  public:
    explicit PragmaInitSegHandler() : PragmaHandler("init_seg") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma float_control
class PragmaFloatControlHandler: public PragmaHandler {
  public:
    explicit PragmaFloatControlHandler() : PragmaHandler("float_control") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma region
class PragmaRegionHandler: public PragmaHandler {
  public:
    explicit PragmaRegionHandler() : PragmaHandler("region") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
    void CheckOpenedRegions(Preprocessor &PP);
};
// #pragma endregion
class PragmaEndRegionHandler: public PragmaHandler {
  public:
    explicit PragmaEndRegionHandler() : PragmaHandler("endregion") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma start_map_region
class PragmaStartMapRegionHandler: public PragmaHandler {
  private:
    friend class clang::Parser;
    bool RegionStarted;
  public:
    explicit PragmaStartMapRegionHandler() : PragmaHandler("start_map_region"), RegionStarted(false) {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma stop_map_region
class PragmaStopMapRegionHandler: public PragmaHandler {
  private:
    friend class clang::Parser;
    bool &RegionStarted;
  public:
    explicit PragmaStopMapRegionHandler(bool &RS) : PragmaHandler("stop_map_region"), RegionStarted(RS) {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
// #pragma vtordisp
class PragmaVtorDispHandler: public PragmaHandler {
  public:
    explicit PragmaVtorDispHandler() : PragmaHandler("vtordisp") {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok);
};
#endif  // INTEL_SPECIFIC_IL0_BACKEND
