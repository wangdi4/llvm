foreach(t ${LLVM_TARGETS_TO_BUILD})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${t})
    add_subdirectory(${t})
  endif()
endforeach()

set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
# INTEL_CUSTOMIZATION
  BinaryFormat
# end INTEL_CUSTOMIZATION
  MC
  MCDisassembler
  Object
  Support
  TargetParser
  )

add_llvm_unittest(MCTests
  Disassembler.cpp
  DwarfLineTables.cpp
  DwarfLineTableHeaders.cpp
# INTEL_CUSTOMIZATION
  Intel_MCTraceTest.cpp
# end INTEL_CUSTOMIZATION
  MCInstPrinter.cpp
  StringTableBuilderTest.cpp
  TargetRegistry.cpp
  MCDisassemblerTest.cpp
  )

