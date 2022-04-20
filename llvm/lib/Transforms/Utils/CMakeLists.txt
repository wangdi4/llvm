# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
add_llvm_component_library(LLVMTransformUtils
  AddDiscriminators.cpp
  AMDGPUEmitPrintf.cpp
  ASanStackFrameLayout.cpp
  AssumeBundleBuilder.cpp
  BasicBlockUtils.cpp
  BreakCriticalEdges.cpp
  BuildLibCalls.cpp
  BypassSlowDivision.cpp
  CallPromotionUtils.cpp
  CallGraphUpdater.cpp
  CanonicalizeAliases.cpp
  CanonicalizeFreezeInLoops.cpp
  CloneFunction.cpp
  CloneModule.cpp
  CodeExtractor.cpp
  CodeLayout.cpp
  CodeMoverUtils.cpp
  CtorUtils.cpp
  Debugify.cpp
  DemoteRegToStack.cpp
  EntryExitInstrumenter.cpp
  EscapeEnumerator.cpp
  Evaluator.cpp
  FixIrreducible.cpp
  FlattenCFG.cpp
  FunctionComparator.cpp
  FunctionImportUtils.cpp
  GlobalStatus.cpp
  GuardUtils.cpp
  HelloWorld.cpp
  InlineFunction.cpp
  InjectTLIMappings.cpp
  InstructionNamer.cpp
  IntegerDivision.cpp
  Intel_CloneUtils.cpp # INTEL_CUSTOMIZATION
# INTEL_COLLAB
  GeneralUtils.cpp
  IntrinsicUtils.cpp
  ../VPO/Utils/IntrinsicUtils.cpp # TODO: Move to lib/Transforms/Utils
# end INTEL_COLLAB
# INTEL_CUSTOMIZATION
  Intel_IMLUtils.cpp
  Intel_InferAddressSpacesUtils.cpp
  Intel_ParseAnnotateAttributes.cpp
  Intel_RegionSplitter.cpp
  Intel_VecClone.cpp                    #TODO: VEC to COLLAB
  Intel_X86EmitMultiVersionResolver.cpp
#end INTEL_CUSTOMIZATION
  LCSSA.cpp
  LibCallsShrinkWrap.cpp
  Local.cpp
  LoopPeel.cpp
  LoopRotationUtils.cpp
  LoopSimplify.cpp
  LoopUnroll.cpp
  LoopUnrollAndJam.cpp
  LoopUnrollRuntime.cpp
  LoopUtils.cpp
  LoopVersioning.cpp
  LowerAtomic.cpp
  LowerGlobalDtors.cpp
  LowerInvoke.cpp
  LowerMemIntrinsics.cpp
  LowerSwitch.cpp
  MatrixUtils.cpp
  MemoryOpRemark.cpp
  MemoryTaggingSupport.cpp
  Mem2Reg.cpp
  MetaRenamer.cpp
  MisExpect.cpp
  ModuleUtils.cpp
  NameAnonGlobals.cpp
  PredicateInfo.cpp
  PromoteMemoryToRegister.cpp
  RelLookupTableConverter.cpp
  ScalarEvolutionExpander.cpp
  SCCPSolver.cpp
  StripGCRelocates.cpp
  SSAUpdater.cpp
  SSAUpdaterBulk.cpp
  SampleProfileInference.cpp
  SampleProfileLoaderBaseUtil.cpp
  SanitizerStats.cpp
  SimplifyCFG.cpp
  SimplifyIndVar.cpp
  SimplifyLibCalls.cpp
  SizeOpts.cpp
  SplitModule.cpp
  StripNonLineTableDebugInfo.cpp
  SymbolRewriter.cpp
  UnifyFunctionExitNodes.cpp
  UnifyLoopExits.cpp
  Utils.cpp
  ValueMapper.cpp
  VNCoercion.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Utils

  DEPENDS
  intrinsics_gen
# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB

  LINK_COMPONENTS
  Analysis
  Core
  Support
  )
