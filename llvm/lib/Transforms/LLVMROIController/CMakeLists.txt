if(NOT LLVM_SPATIAL_ADVISOR)
  return()
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
message("boost: ${Boost_INCLUDE_DIRS}")
message("boost lib Boost_LIBRARIES: ${Boost_LIBRARIES}")

add_llvm_loadable_module( LLVMROIController
  LLVMROIController.cpp
  LLVMControlManager.cpp    
  ROIinstrument.cpp
  )
target_link_libraries( LLVMROIController ${Boost_LIBRARIES} )

add_dependencies( LLVMROIController intrinsics_gen)

