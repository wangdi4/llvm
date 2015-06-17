add_subdirectory(Vecopt)
add_subdirectory(Utils)

add_llvm_library(LLVMVPODriver
  Driver/VPODriver.cpp
)
