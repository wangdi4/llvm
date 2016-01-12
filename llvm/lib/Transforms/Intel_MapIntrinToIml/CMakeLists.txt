find_library(LIBIML NAMES iml_attr PATHS ${CMAKE_CURRENT_SOURCE_DIR})

add_llvm_library(LLVMIntel_MapIntrinToIml
    MapIntrinToIml.cpp
    LINK_LIBS ${LIBIML})

add_dependencies(LLVMIntel_MapIntrinToIml intrinsics_gen)
