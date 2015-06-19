if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)
message( STATUS "Frontend LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

if (NOT USE_COMMON_CLANG)
    add_subdirectory( common_clang )
endif()

if (NOT WIN32)
    add_subdirectory( linux_linker )
endif()

add_subdirectory( clang_compiler )
