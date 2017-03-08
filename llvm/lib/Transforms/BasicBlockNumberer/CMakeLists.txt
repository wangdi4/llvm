if(NOT LLVM_SPATIAL_ADVISOR)
  message("SPATIAL_ADVISOR not enabled - not building BasicBlockNumberer")
  return()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()


# Boost lib
#set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.59.0")

add_llvm_loadable_module( LLVMBasicBlockNumberer
  BasicBlockNumberer.cpp
  )

add_dependencies(LLVMBasicBlockNumberer intrinsics_gen)

