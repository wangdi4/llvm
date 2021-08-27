add_llvm_component_library(LLVMIntel_DPCPPKernelTransforms
  AddFunctionAttrs.cpp
  AddImplicitArgs.cpp
  BuiltinCallToInst.cpp
  BuiltinImport.cpp
  DPCPPEqualizer.cpp
  DPCPPKernelTransforms.cpp
  DPCPPKernelVecClone.cpp
  DPCPPPrepareKernelForVecClone.cpp
  DPCPPKernelPostVec.cpp
  DPCPPKernelCompilationUtils.cpp
  DPCPPKernelLoopUtils.cpp
  DPCPPKernelWGLoopCreator.cpp
  DPCPPKernelAnalysis.cpp
  DPCPPPreprocessSPIRVFriendlyIR.cpp
  ImplicitArgsAnalysis.cpp
  InternalizeNonKernelFunc.cpp
  LocalBufferAnalysis.cpp
  LocalBuffers.cpp
  LinearIdResolver.cpp
  DuplicateCalledKernelsPass.cpp
  PhiCanonicalization.cpp
  RedundantPhiNodePass.cpp
  GroupBuiltinPass.cpp
  SplitBBonBarrierPass.cpp
  KernelBarrierUtils.cpp
  WIRelatedValuePass.cpp
  DataPerBarrierPass.cpp
  DataPerValuePass.cpp
  BarrierPass.cpp
  BarrierInFunctionPass.cpp
  ResolveWICall.cpp
  SplitBBonBarrierPass.cpp
  WIRelatedValuePass.cpp
  PrepareKernelArgs.cpp
  CleanupWrappedKernel.cpp
  SetVectorizationFactor.cpp
  VectorVariant/CreateSimdVariantPropagation.cpp
  VectorVariant/SGSizeCollector.cpp
  VectorVariant/SGSizeCollectorIndirect.cpp
  VectorVariant/UpdateCallAttrs.cpp
  VectorVariant/VectorVariantFillIn.cpp
  VectorVariant/VectorVariantLowering.cpp
  VFAnalysis.cpp

  Utils/CPUDetect.cpp
  Utils/FunctionDescriptor.cpp
  Utils/ImplicitArgsUtils.cpp
  Utils/NameMangleAPI.cpp
  Utils/ParameterType.cpp
  Utils/RuntimeService.cpp
  Utils/TypeAlignment.cpp
  Utils/VectorizerUtils.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_DPCPPKernelTransforms

  DEPENDS
  intrinsics_gen
  directives_gen

  LINK_COMPONENTS
  Analysis
  Core
  IRReader
  Linker
  Support
  TransformUtils
)

# FIXME: There is no corresponding argument in add_llvm_component_library with
# the same semantic as target_include_directories. It's not typical to include
# additional directories the way below. This require some redesign for headers
# layout. Community changes can break this.
target_include_directories(LLVMIntel_DPCPPKernelTransforms
  PRIVATE ${LLVM_MAIN_SRC_DIR}/lib/Transforms/Intel_DPCPPKernelTransforms/Utils)
