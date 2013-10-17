if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
else()
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}" )
endif()

set( CLANG "${TOOLS_BINARY_DIR}/clang" )

option(INCLUDE_CONFORMANCE_TESTS "Include Conformance Tests Into Build"  OFF)
if (INCLUDE_CONFORMANCE_TESTS OR INCLUDE_CONFORMANCE_1_2_TESTS OR INCLUDE_CONFORMANCE_ITP_TESTS)
    
    if (WIN32)    
        set(MSVC 1)
        set(USE_ICC 1)
    endif (WIN32)
    
    if (INCLUDE_CONFORMANCE_TESTS)
        # Add conformance 1.1 sources
        add_subdirectory( conform1.1 )
    endif (INCLUDE_CONFORMANCE_TESTS)
    
    if (INCLUDE_CONFORMANCE_1_2_TESTS)
        # Add conformance 1.2 sources
        add_subdirectory( conform1.2 )
    endif (INCLUDE_CONFORMANCE_1_2_TESTS)    

    if (INCLUDE_CONFORMANCE_ITP_TESTS)
        # Add conformance ITP sources
        add_subdirectory( conform.itp )
    endif (INCLUDE_CONFORMANCE_ITP_TESTS)

endif (INCLUDE_CONFORMANCE_TESTS OR INCLUDE_CONFORMANCE_1_2_TESTS OR INCLUDE_CONFORMANCE_ITP_TESTS)

# The conformance tests must be conpiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}
# Therefore, it must be added to the include directories only after 
# the conformance subdirectory has been added.
include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )

add_subdirectory( tbb_native_test_type )
add_subdirectory( cpu_device_test_type )
add_subdirectory( framework_test_type )
add_subdirectory( debugger_test_type )
if(INCLUDE_TOOLS AND NOT ANDROID)
  add_subdirectory( tools_test_type)
  if(NOT ANDROID)
    add_subdirectory(recorder)
  endif(NOT ANDROID)
endif(INCLUDE_TOOLS AND NOT ANDROID)

add_subdirectory( task_executor_test_type )

if ( INCLUDE_MIC_DEVICE )
  add_subdirectory( mic_device_test_type )
endif( INCLUDE_MIC_DEVICE )

if( USE_VALGRIND )
    add_subdirectory( valgrind )
endif( USE_VALGRIND )

add_subdirectory(type_conversion)

if (INCLUDE_CMRT)
    message (STATUS "Adding Common runtime tests ...")
    add_subdirectory ( common_runtime_test_type )
endif(INCLUDE_CMRT)

#Samples
option(INCLUDE_SMPLS "Include Samples into Build"  OFF)
if (INCLUDE_SMPLS)
    add_subdirectory( samples )
endif(INCLUDE_SMPLS)
