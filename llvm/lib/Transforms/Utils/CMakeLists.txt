add_llvm_library(LLVMTransformUtils
  ASanStackFrameLayout.cpp
  AddDiscriminators.cpp
  BasicBlockUtils.cpp
  BreakCriticalEdges.cpp
  BuildLibCalls.cpp
  BypassSlowDivision.cpp
  CallPromotionUtils.cpp
  CloneFunction.cpp
  CloneModule.cpp
  CodeExtractor.cpp
  CtorUtils.cpp
  DemoteRegToStack.cpp
  EntryExitInstrumenter.cpp
  EscapeEnumerator.cpp
  Evaluator.cpp
  FlattenCFG.cpp
  FunctionComparator.cpp
  FunctionImportUtils.cpp
  GlobalStatus.cpp
  InlineFunction.cpp
  ImportedFunctionsInliningStatistics.cpp
  InstructionNamer.cpp
  InstructionPrecedenceTracking.cpp
  IntegerDivision.cpp
# INTEL_COLLAB
  Intel_InferAddressSpacesUtils.cpp
  Intel_GeneralUtils.cpp
  Intel_IntrinsicUtils.cpp
  ../Intel_VPO/Utils/IntrinsicUtils.cpp # TODO: Move to lib/Transforms/Utils
# end INTEL_COLLAB
  Intel_VecClone.cpp   # INTEL_CUSTOMIZATION    TODO: VEC to COLLAB
  LCSSA.cpp
  LibCallsShrinkWrap.cpp
  Local.cpp
  LoopRotationUtils.cpp
  LoopSimplify.cpp
  LoopUnroll.cpp
  LoopUnrollAndJam.cpp
  LoopUnrollPeel.cpp
  LoopUnrollRuntime.cpp
  LoopUtils.cpp
  LoopVersioning.cpp
  LowerInvoke.cpp
  LowerMemIntrinsics.cpp
  LowerSwitch.cpp
  Mem2Reg.cpp
  MetaRenamer.cpp
  ModuleUtils.cpp
  NameAnonGlobals.cpp
  OrderedInstructions.cpp
  PredicateInfo.cpp
  PromoteMemoryToRegister.cpp
  StripGCRelocates.cpp
  SSAUpdater.cpp
  SSAUpdaterBulk.cpp
  SanitizerStats.cpp
  SimplifyCFG.cpp
  SimplifyIndVar.cpp
  SimplifyLibCalls.cpp
  SplitModule.cpp
  StripNonLineTableDebugInfo.cpp
  SymbolRewriter.cpp
  UnifyFunctionExitNodes.cpp
  Utils.cpp
  ValueMapper.cpp
  VNCoercion.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Utils

  DEPENDS
  intrinsics_gen
# INTEL_COLLAB
  intel_directives_gen
# end INTEL_COLLAB
  )
