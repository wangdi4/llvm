include_directories(
  ${CMAKE_SOURCE_DIR}/lib/Target/Intel_CSA
  ${CMAKE_BINARY_DIR}/lib/Target/Intel_CSA
  )

set(LLVM_LINK_COMPONENTS
  CodeGen
  Core
  CSACodeGen
  CSADesc
  CSAInfo
  MC
  MIRParser
  SelectionDAG
  Support
  Target
  )

add_llvm_unittest(CSATests
  CSAOpInfo.cpp
  )
