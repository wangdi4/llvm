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
# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES_TO_BUILD)
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
# Add custom Intel files below.  Files listed here will be included in
# INTEL_CUSTOMIZATION build.
  )

# INTEL_FEATURE_CSA
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_CSA
  ToolChains/Intel_CSA.cpp
  )
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION

set(LLVM_LINK_COMPONENTS
  BinaryFormat
  BitWriter
  Core
  IRReader
  MC
  Object
  Option
  ProfileData
  Support
  WindowsDriver
  )

if(WIN32)
  # MSVCToolChain.cpp uses version.dll.
  set(system_libs version)
endif()

add_clang_library(clangDriver
  Action.cpp
  Compilation.cpp
  Distro.cpp
  Driver.cpp
  DriverOptions.cpp
  Job.cpp
  Multilib.cpp
  OffloadBundler.cpp
  OptionUtils.cpp
  Phases.cpp
  SanitizerArgs.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains/Arch/AArch64.cpp
  ToolChains/Arch/ARM.cpp
  ToolChains/Arch/CSKY.cpp
  ToolChains/Arch/LoongArch.cpp
  ToolChains/Arch/M68k.cpp
  ToolChains/Arch/Mips.cpp
  ToolChains/Arch/PPC.cpp
  ToolChains/Arch/RISCV.cpp
  ToolChains/Arch/Sparc.cpp
  ToolChains/Arch/SystemZ.cpp
  ToolChains/Arch/VE.cpp
  ToolChains/Arch/X86.cpp
  ToolChains/AIX.cpp
  ToolChains/Ananas.cpp
  ToolChains/AMDGPU.cpp
  ToolChains/AMDGPUOpenMP.cpp
  ToolChains/AVR.cpp
  ToolChains/BareMetal.cpp
  ToolChains/Clang.cpp
  ToolChains/CloudABI.cpp
  ToolChains/CommonArgs.cpp
  ToolChains/Contiki.cpp
  ToolChains/CrossWindows.cpp
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION
  ToolChains/CSKYToolChain.cpp
  ToolChains/Cuda.cpp
  ToolChains/Darwin.cpp
  ToolChains/DragonFly.cpp
  ToolChains/Flang.cpp
  ToolChains/FreeBSD.cpp
  ToolChains/Fuchsia.cpp
  ToolChains/Gnu.cpp
  ToolChains/Haiku.cpp
  ToolChains/HIPUtility.cpp
  ToolChains/HIPAMD.cpp
  ToolChains/HIPSPV.cpp
  ToolChains/Hexagon.cpp
  ToolChains/HLSL.cpp
  ToolChains/Hurd.cpp
  ToolChains/Linux.cpp
  ToolChains/MipsLinux.cpp
  ToolChains/MinGW.cpp
  ToolChains/Minix.cpp
  ToolChains/MSP430.cpp
  ToolChains/MSVC.cpp
  ToolChains/Myriad.cpp
  ToolChains/NaCl.cpp
  ToolChains/NetBSD.cpp
  ToolChains/OpenBSD.cpp
  ToolChains/PS4CPU.cpp
  ToolChains/RISCVToolchain.cpp
  ToolChains/Solaris.cpp
  ToolChains/SPIRV.cpp
  ToolChains/TCE.cpp
  ToolChains/VEToolchain.cpp
  ToolChains/WebAssembly.cpp
  ToolChains/XCore.cpp
  ToolChains/SYCL.cpp
  ToolChains/PPCLinux.cpp
  ToolChains/PPCFreeBSD.cpp
  ToolChains/InterfaceStubs.cpp
  ToolChains/ZOS.cpp
  Types.cpp
  XRayArgs.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  ${system_libs}
  )
