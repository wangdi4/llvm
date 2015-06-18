if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)
message( STATUS "Frontend LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

add_subdirectory( clang_compiler )

