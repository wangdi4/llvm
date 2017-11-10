include_directories(
  ${CMAKE_SOURCE_DIR}/lib/Target/CSA
  ${CMAKE_BINARY_DIR}/lib/Target/CSA
  )

set(LLVM_LINK_COMPONENTS
  CSACodeGen
  CodeGen
  Core
  MC
  MIRParser
  SelectionDAG
  Support
  Target
  )

add_llvm_unittest(CSATests
  CSAOpInfo.cpp
  )
