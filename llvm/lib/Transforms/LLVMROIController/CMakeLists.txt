# If we don't need RTTI or EH, there's no reason to export anything
# from the hello plugin.
#if( NOT LLVM_REQUIRES_RTTI )
#  if( NOT LLVM_REQUIRES_EH )
#    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/FPGA-Advisor.exports)
#  endif()
#endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()


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

