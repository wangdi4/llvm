if(NOT LLVM_SPATIAL_ADVISOR)
  message("SPATIAL_ADVISOR not enabled - not building BasicBlockProfiler")
  return()
else()
  message("SPATIAL_ADVISOR enabled - building BasicBlockProfiler")
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()


# The advisor needs C++ error handling, which in turn requires RTTI.
# Due to CMake deficiencies, it turns out RTTI must be also enabled for the
# entire LLVM build.
set(LLVM_REQUIRES_EH 1)
set(LLVM_REQUIRES_RTTI 1)

# Boost lib
set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.59.0")

find_package( Boost REQUIRED COMPONENTS system serialization)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
endif()

add_llvm_loadable_module( LLVMBasicBlockProfiler
  BasicBlockProfiler.cpp
  bbutils.cpp
  bbinstrument.cpp
  bbprofile.cpp
  )
add_dependencies(LLVMBasicBlockProfiler intrinsics_gen)

target_link_libraries( LLVMBasicBlockProfiler ${Boost_LIBRARIES} )
