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
  Support
  )

add_llvm_unittest(MCTests
  Disassembler.cpp
  DwarfLineTables.cpp
<<<<<<< HEAD
# INTEL_CUSTOMIZATION
  Intel_MCTraceTest.cpp
# end INTEL_CUSTOMIZATION
=======
  DwarfLineTableHeaders.cpp
>>>>>>> 050edef8538e82264928ddf1840e785f1544ca7f
  MCInstPrinter.cpp
  StringTableBuilderTest.cpp
  TargetRegistry.cpp
  MCDisassemblerTest.cpp
  )

