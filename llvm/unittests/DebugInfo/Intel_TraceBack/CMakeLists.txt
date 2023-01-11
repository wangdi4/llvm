set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  MC
  Support
  TargetParser
  DebugInfoTraceBack
  )

add_llvm_unittest(DebugInfoTraceBackTests
  TraceDINodeTest.cpp
  )

target_link_libraries(DebugInfoTraceBackTests PRIVATE LLVMTestingSupport)
