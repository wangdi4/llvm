# FIXME - We should not have to disable this warning. We need to fix the
#         violations in the source code and re-enable it.
if (UNIX)
  set_source_files_properties(iml_accuracy_interface.cpp PROPERTIES
                              COMPILE_FLAGS -Wno-error=strict-aliasing)
endif()

add_llvm_library(LLVMIntel_MapIntrinToIml
    MapIntrinToIml.cpp
    iml_accuracy_interface.cpp
    search.cpp)

add_dependencies(LLVMIntel_MapIntrinToIml intrinsics_gen)
