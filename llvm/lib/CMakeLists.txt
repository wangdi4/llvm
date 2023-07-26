# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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
# INTEL_CUSTOMIZATION

include(LLVM-Build)

# `Demangle', `Support' and `TableGen' libraries are added on the top-level
# CMakeLists.txt

add_subdirectory(IR)
add_subdirectory(FuzzMutate)
add_subdirectory(FileCheck)
add_subdirectory(InterfaceStub)
add_subdirectory(IRPrinter)
add_subdirectory(IRReader)
add_subdirectory(CodeGen)
add_subdirectory(BinaryFormat)
add_subdirectory(Bitcode)
add_subdirectory(Bitstream)
add_subdirectory(DWARFLinker)
add_subdirectory(DWARFLinkerParallel)
add_subdirectory(Extensions)
add_subdirectory(Frontend)
add_subdirectory(Transforms)
add_subdirectory(Linker)
add_subdirectory(Analysis)
add_subdirectory(LTO)
add_subdirectory(MC)
add_subdirectory(MCA)
add_subdirectory(ObjCopy)
add_subdirectory(Object)
add_subdirectory(ObjectYAML)
add_subdirectory(Option)
add_subdirectory(Remarks)
add_subdirectory(Debuginfod)
add_subdirectory(DebugInfo)
add_subdirectory(DWP)
add_subdirectory(ExecutionEngine)
add_subdirectory(Target)
add_subdirectory(AsmParser)
add_subdirectory(LineEditor)
add_subdirectory(ProfileData)
add_subdirectory(Passes)
add_subdirectory(SYCLLowerIR)
add_subdirectory(TargetParser)
add_subdirectory(TextAPI)
add_subdirectory(ToolDrivers)
add_subdirectory(XRay)
# INTEL_CUSTOMIZATION
add_subdirectory(Intel_MLPGO)
# end INTEL_CUSTOMIZATION
if (LLVM_INCLUDE_TESTS)
  add_subdirectory(Testing)
endif()
add_subdirectory(WindowsDriver)
add_subdirectory(WindowsManifest)

set(LLVMCONFIGLIBRARYDEPENDENCIESINC "${LLVM_BINARY_DIR}/tools/llvm-config/LibraryDependencies.inc")

# Special components which don't have any source attached but aggregate other
# components
add_llvm_component_group(all-targets LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD})
add_llvm_component_group(Engine)

# The native target may not be enabled when cross compiling
if(TARGET ${LLVM_NATIVE_ARCH})
  add_llvm_component_group(Native LINK_COMPONENTS ${LLVM_NATIVE_ARCH})
  add_llvm_component_group(NativeCodeGen LINK_COMPONENTS ${LLVM_NATIVE_ARCH}CodeGen)
else()
  add_llvm_component_group(Native)
  add_llvm_component_group(NativeCodeGen)
endif()

# Component post-processing
LLVMBuildResolveComponentsLink()
LLVMBuildGenerateCFragment(OUTPUT ${LLVMCONFIGLIBRARYDEPENDENCIESINC})
