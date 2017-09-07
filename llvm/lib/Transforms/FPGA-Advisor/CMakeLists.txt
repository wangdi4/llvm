if(NOT LLVM_SPATIAL_ADVISOR)
  message("-- Not building the Spatial Advisor")
  message("   Define the environment variable ENABLE_SPATIAL_ADVISOR to build.")
  return()
endif()
message("-- Building the Spatial Advisor")

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

# The advisor needs C++ error handling, which in turn requires RTTI.
# Due to CMake deficiencies, it turns out RTTI must be also enabled for the
# entire LLVM build.
set(LLVM_REQUIRES_EH 1)
set(LLVM_REQUIRES_RTTI 1)

# Boost lib
set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.59.0" "1.64.0")

find_library( TBB_LIBRARY tbb )

find_package( Boost REQUIRED)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
endif()

add_llvm_loadable_module( LLVMFPGA-Advisor
  DependenceGraph.cpp
  FPGA-Advisor-Instrument.cpp
  FPGA-Advisor-Analysis.cpp
  ModuleAreaEstimator.cpp
  ModuleScheduler.cpp

  PLUGIN_TOOL
  opt

  ADDITIONAL_HEADER_DIRS
  ${TBBROOT}/include
  )

set_target_properties( LLVMFPGA-Advisor PROPERTIES COMPILE_FLAGS "-I${TBBROOT}/include" )
# listing tbb here is not the right thing to do,
# but it seems to work.
target_link_libraries( LLVMFPGA-Advisor tbb )

add_dependencies(LLVMFPGA-Advisor intrinsics_gen)
