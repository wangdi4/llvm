# INTEL_CUSTOMIZATION
# Placeholder list for INTEL_CUSTOMIZATION files.
set(INTEL_SOURCE_FILES
  )
set(INTEL_CSA_SOURCE_FILES
  ToolChains/CSA.cpp
  )

set(INTEL_SOURCE_FILES_TO_BUILD)
if (INTEL_CUSTOMIZATION)
  if(";${LLVM_INTEL_FEATURES};" MATCHES ";CSA;")
    list(APPEND INTEL_SOURCE_FILES ${INTEL_CSA_SOURCE_FILES})
  else()
    list(APPEND LLVM_OPTIONAL_SOURCES ${INTEL_CSA_SOURCE_FILES})
  endif()

  set(INTEL_SOURCE_FILES_TO_BUILD ${INTEL_SOURCE_FILES})
else()
  list(APPEND LLVM_OPTIONAL_SOURCES
    ${INTEL_SOURCE_FILES}
    ${INTEL_CSA_SOURCE_FILES}
    )
endif()
# end INTEL_CUSTOMIZATION

set(LLVM_LINK_COMPONENTS
  BinaryFormat
  Option
  Support
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
  Phases.cpp
  SanitizerArgs.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains/Arch/AArch64.cpp
  ToolChains/Arch/ARM.cpp
  ToolChains/Arch/Mips.cpp
  ToolChains/Arch/PPC.cpp
  ToolChains/Arch/RISCV.cpp
  ToolChains/Arch/Sparc.cpp
  ToolChains/Arch/SystemZ.cpp
  ToolChains/Arch/X86.cpp
  ToolChains/Ananas.cpp
  ToolChains/AMDGPU.cpp
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
  ToolChains/Cuda.cpp
  ToolChains/Darwin.cpp
  ToolChains/DragonFly.cpp
  ToolChains/FreeBSD.cpp
  ToolChains/Fuchsia.cpp
  ToolChains/Gnu.cpp
  ToolChains/Haiku.cpp
  ToolChains/HIP.cpp
  ToolChains/Hexagon.cpp
  ToolChains/Linux.cpp
  ToolChains/MipsLinux.cpp
  ToolChains/MinGW.cpp
  ToolChains/Minix.cpp
  ToolChains/MSVC.cpp
  ToolChains/Myriad.cpp
  ToolChains/NaCl.cpp
  ToolChains/NetBSD.cpp
  ToolChains/OpenBSD.cpp
  ToolChains/PS4CPU.cpp
  ToolChains/RISCV.cpp
  ToolChains/Solaris.cpp
  ToolChains/TCE.cpp
  ToolChains/WebAssembly.cpp
  ToolChains/XCore.cpp
  Types.cpp
  XRayArgs.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  ${system_libs}
  )
