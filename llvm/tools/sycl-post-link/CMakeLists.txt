if(INTEL_CUSTOMIZATION)
get_property(LLVMGenXIntrinsics_SOURCE_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_SOURCE_PROP)
get_property(LLVMGenXIntrinsics_BINARY_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_BINARY_PROP)

include_directories(
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include)
endif()

set(LLVM_LINK_COMPONENTS
  BitWriter
  Core
  IPO
  IRReader
  Support
# INTEL_CUSTOMIZATION
  GenXIntrinsics
  VPOTransforms
# end INTEL_CUSTOMIZATION
  TransformUtils
  SYCLLowerIR
  )

get_property(LLVMGenXIntrinsics_SOURCE_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_SOURCE_PROP)
get_property(LLVMGenXIntrinsics_BINARY_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_BINARY_PROP)

include_directories(
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include)

add_llvm_tool(sycl-post-link
  sycl-post-link.cpp
  SPIRKernelParamOptInfo.cpp
  SpecConstants.cpp

# INTEL_CUSTOMIZATION
  ADDITIONAL_HEADER_DIRS
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include

  LINK_LIBS
  LLVMVPOTransforms
  LLVMGenXIntrinsics
  LLVMVPOTransforms
# end INTEL_CUSTOMIZATION

  DEPENDS
  intrinsics_gen
  LLVMGenXIntrinsics
# INTEL_CUSTOMIZATION
  LLVMVPOTransforms
# end INTEL_CUSTOMIZATION
  CUSTOM_WIN_VER
  )
if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(sycl-post-link
    "sycl-post-link.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "SYCL post-link device code processing tool")
endif()
