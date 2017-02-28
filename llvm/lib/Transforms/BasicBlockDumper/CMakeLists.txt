if(NOT LLVM_SPATIAL_ADVISOR)
  return()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()


# Boost lib
set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.59.0")

find_package( Boost REQUIRED)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
endif()

add_llvm_loadable_module( LLVMBasicBlockDumper
  BasicBlockDumper.cpp
  BasicBlockCompiler.cpp
  PLUGIN_TOOL
  opt
  )

add_dependencies(LLVMBasicBlockDumper intrinsics_gen)
