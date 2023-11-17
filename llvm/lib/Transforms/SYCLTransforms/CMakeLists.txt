# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
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
add_llvm_component_library(LLVMSYCLTransforms
  AddFastMath.cpp
  AddFunctionAttrs.cpp
  AddImplicitArgs.cpp
  AddNTAttr.cpp
  AddTLSGlobals.cpp
  AutorunReplicator.cpp
  BarrierInFunctionPass.cpp
  BarrierPass.cpp
  BuiltinCallToInst.cpp
  BuiltinImport.cpp
  BuiltinLICM.cpp
  BuiltinLibInfoAnalysis.cpp
  ChannelPipeTransformation.cpp
  CleanupWrappedKernel.cpp
  CoerceTypes.cpp
  CoerceWin64Types.cpp
  DataPerBarrierPass.cpp
  DataPerValuePass.cpp
  DeduceMaxWGDim.cpp
  DetectRecursion.cpp
  DuplicateCalledKernelsPass.cpp
  ExternalizeGlobalVariables.cpp
  GroupBuiltinPass.cpp
  ImplicitArgsAnalysis.cpp
  ImplicitGID.cpp
  InferArgumentAlias.cpp
  InfiniteLoopCreator.cpp
  InstToFuncCall.cpp
  # INTEL_CUSTOMIZATION
  Intel_HandleVPlanMask.cpp
  Intel_ResolveMatrixFill.cpp
  Intel_ResolveMatrixLayout.cpp
  Intel_ResolveMatrixWISlice.cpp
  Intel_SYCLKernelPostVec.cpp
  Intel_SYCLKernelVecClone.cpp
  Intel_SYCLPrepareKernelForVecClone.cpp
  Intel_SinCosFold.cpp
  Intel_TaskSeqAsyncHandling.cpp
  Intel_VectorKernelElimination.cpp
  Intel_VectorVariant/CreateSimdVariantPropagation.cpp
  Intel_VectorVariant/IndirectCallLowering.cpp
  Intel_VectorVariant/SGSizeCollector.cpp
  Intel_VectorVariant/SGSizeCollectorIndirect.cpp
  Intel_VectorVariant/UpdateCallAttrs.cpp
  Intel_VectorVariant/VectorVariantFillIn.cpp
  Intel_VectorVariant/VectorVariantLowering.cpp
  Intel_VectorizationDimensionAnalysis.cpp
  # end INTEL_CUSTOMIZATION
  InternalizeGlobalVariables.cpp
  InternalizeNonKernelFunc.cpp
  KernelTargetExtTypeLower.cpp
  LinearIdResolver.cpp
  LocalBufferAnalysis.cpp
  LocalBuffers.cpp
  LoopStridedCodeMotion.cpp
  LoopWIAnalysis.cpp
  MathFuncSelect.cpp
  OptimizeIDivAndIRem.cpp
  PatchCallbackArgs.cpp
  PhiCanonicalization.cpp
  PipeIOTransformation.cpp
  PipeOrdering.cpp
  PipeSupport.cpp
  PrepareKernelArgs.cpp
  PreventDivCrashes.cpp
  ProfilingInfo.cpp
  ReduceCrossBarrierValues.cpp
  RedundantPhiNodePass.cpp
  RelaxedMath.cpp
  RemoveAtexit.cpp
  RemoveDeviceLibAssertFallback.cpp
  RemoveDuplicatedBarrier.cpp
  ReplaceScalarWithMask.cpp
  ReqdSubGroupSize.cpp
  ResolveSubGroupWICall.cpp
  ResolveVarTIDCall.cpp
  ResolveWICall.cpp
  SGRemapWICall.cpp
  SetPreferVectorWidth.cpp
  SetVectorizationFactor.cpp
  SoaAllocaAnalysis.cpp
  SpecializeConstant.cpp
  SplitBBonBarrierPass.cpp
  SplitBBonBarrierPass.cpp
  SubgroupEmulation/SGBarrierPropagate.cpp
  SubgroupEmulation/SGBarrierSimplify.cpp
  SubgroupEmulation/SGBuiltin.cpp
  SubgroupEmulation/SGFunctionWiden.cpp
  SubgroupEmulation/SGHelper.cpp
  SubgroupEmulation/SGLoopConstruct.cpp
  SubgroupEmulation/SGSizeAnalysis.cpp
  SubgroupEmulation/SGValueWiden.cpp
  SYCLAliasAnalysis.cpp
  SYCLEqualizer.cpp
  SYCLKernelAnalysis.cpp
  SYCLKernelWGLoopCreator.cpp
  SYCLPreprocessSPIRVFriendlyIR.cpp
  SYCLRewritePipes.cpp
  Utils/BarrierUtils.cpp
  Utils/CompilationUtils.cpp
  Utils/DiagnosticInfo.cpp
  Utils/FunctionDescriptor.cpp
  Utils/ImplicitArgsUtils.cpp
  Utils/LoopPeeling.cpp
  Utils/LoopUtils.cpp
  Utils/NameMangleAPI.cpp
  Utils/ParameterType.cpp
  Utils/Predicator.cpp
  Utils/RuntimeService.cpp
  Utils/TypeAlignment.cpp
  Utils/VectorizerUtils.cpp
  Utils/WGBoundDecoder.cpp
  Utils/SYCLChannelPipeUtils.cpp
  Utils/SYCLStatistic.cpp
  VFAnalysis.cpp
  WGLoopBoundaries.cpp
  WIRelatedValuePass.cpp
  WIRelatedValuePass.cpp
  WeightedInstCount.cpp
  WorkItemAnalysis.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/SYCLTransforms

  DEPENDS
  intrinsics_gen
  directives_gen # INTEL

  LINK_COMPONENTS
  Analysis
  Core
  IRReader
  Linker
  Support
  TransformUtils
)
