set(LLVM_LINK_COMPONENTS
  Support
  )
#if INTEL_CUSTOMIZATION
list(APPEND LLVM_OPTIONAL_SOURCES IntelVfsTestLib.cpp)
#endif INTEL_CUSTOMIZATION

add_clang_unittest(FrontendTests
  ASTUnitTest.cpp
  CompilerInstanceTest.cpp
  FixedPointString.cpp
  FrontendActionTest.cpp
  CodeGenActionTest.cpp
  ParsedSourceLocationTest.cpp
  PCHPreambleTest.cpp
  OutputStreamTest.cpp
  )
target_link_libraries(FrontendTests
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangLex
  clangSema
  clangCodeGen
  clangFrontendTool
  )

#if INTEL_CUSTOMIZATION
add_library(VfsTestLib SHARED IntelVfsTestLib.cpp)
target_link_libraries(VfsTestLib clangBasic LLVMSupport)
set_target_properties(VfsTestLib
  PROPERTIES PREFIX ""
  SUFFIX ${LTDL_SHLIB_EXT}
  )
set(outdir ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
set_output_directory(VfsTestLib BINARY_DIR ${outdir} LIBRARY_DIR ${outdir})
get_target_property(test_suite_folder FrontendTests FOLDER)
set_property(TARGET VfsTestLib PROPERTY FOLDER "${test_suite_folder}")
add_dependencies(FrontendTests VfsTestLib)
#endif INTEL_CUSTOMIZATION
