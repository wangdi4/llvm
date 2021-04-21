add_llvm_component_library(LLVMIntel_DPCPPKernelTransforms
  AddImplicitArgs.cpp
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
  ImplicitArgsAnalysis.cpp
  LocalBufferAnalysis.cpp
  PhiCanonicalization.cpp
  RedundantPhiNodePass.cpp
  SplitBBonBarrierPass.cpp
  DPCPPKernelBarrierUtils.cpp
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

  Utils/CPUDetect.cpp
  Utils/FunctionDescriptor.cpp
  Utils/ImplicitArgsUtils.cpp
  Utils/NameMangleAPI.cpp
  Utils/ParameterType.cpp
  Utils/TypeAlignment.cpp

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
