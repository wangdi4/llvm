# INTEL_CUSTOMIZATION
if (MSVC)
  set_source_files_properties(PassBuilder.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()
# END INTEL_CUSTOMIZATION
add_llvm_library(LLVMPasses
  PassBuilder.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Passes

  DEPENDS
  intrinsics_gen
  )
