if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

if (NOT WIN32)
  set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
  # disable warnings inherited from external componennts
  add_definitions(-Wno-attributes)
endif (NOT WIN32)

add_subdirectory( cl_hw_utils )
add_subdirectory( cl_logger )
add_subdirectory( cl_sys_utils )
add_subdirectory( CLElfLib )
add_subdirectory( cache_binary_handler )

if(WIN32)
    add_subdirectory( dyn_cl_sys_utils )
    add_subdirectory( dyn_cl_hw_utils )
endif(WIN32)

add_subdirectory( task_executor )
