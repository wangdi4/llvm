set(LLVM_LINK_COMPONENTS Object Support)

if(NOT CLANG_BUILT_STANDALONE)
  set(tablegen_deps intrinsics_gen)
endif()

add_clang_tool(clang-offload-bundler
  ClangOffloadBundler.cpp
  
  DEPENDS
  ${tablegen_deps}
  )

set(CLANG_OFFLOAD_BUNDLER_LIB_DEPS
  clangBasic
  )
  
add_dependencies(clang clang-offload-bundler)

clang_target_link_libraries(clang-offload-bundler
  PRIVATE
  ${CLANG_OFFLOAD_BUNDLER_LIB_DEPS}
  )
<<<<<<< HEAD

install(TARGETS clang-offload-bundler RUNTIME # INTEL https://reviews.llvm.org/D56112
  COMPONENT clang-offload-bundler             # INTEL https://reviews.llvm.org/D56112
  DESTINATION bin                             # INTEL https://reviews.llvm.org/D56112
  )                                           # INTEL https://reviews.llvm.org/D56112
=======
>>>>>>> 5cd0db7e92ef31d4a979f07800235d23bffa1de9
