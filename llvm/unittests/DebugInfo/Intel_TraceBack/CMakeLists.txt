set(LLVM_LINK_COMPONENTS
  DebugInfoTraceBack
  )

add_llvm_unittest(DebugInfoTraceBackTests
  TraceDINodeTest.cpp
  )

target_link_libraries(DebugInfoTraceBackTests PRIVATE LLVMTestingSupport)
