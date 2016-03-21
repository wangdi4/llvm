add_llvm_library(LLVMIntel_MapIntrinToIml
    MapIntrinToIml.cpp
    iml_accuracy_interface.cpp
    search.cpp)

add_dependencies(LLVMIntel_MapIntrinToIml intrinsics_gen)
