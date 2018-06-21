set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  CodeGen
  Core
  MC
  Support
  Target
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_BINARY_DIR}/lib/Target/CSA/")

add_llvm_unittest(Intel_MIRMatcherTests
  Intel_MIRMatcherTest.cpp
  )
