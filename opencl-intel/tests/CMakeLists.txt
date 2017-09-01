# The conformance tests must be conpiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}
# Therefore, it must be added to the include directories only after 
# the conformance subdirectory has been added.
include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )

# define INTEL_CUSTOMIZATION xmain macro
add_definitions( -DINTEL_CUSTOMIZATION )

# some tests have to use deprecated OpenCL API functions.
if (WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif (WIN32)

add_subdirectory( tbb_native_test_type )
add_subdirectory( cpu_device_test_type )
add_subdirectory( framework_test_type )
add_subdirectory( clang_compiler_test_type )

add_subdirectory( task_executor_test_type )
add_subdirectory( bi_test_type )

if ( INCLUDE_MIC_DEVICE )
  add_subdirectory( mic_device_test_type )
endif( INCLUDE_MIC_DEVICE )

if( USE_VALGRIND )
  add_subdirectory( valgrind )
endif( USE_VALGRIND )

if (INCLUDE_CMRT)
  message (STATUS "Adding Common runtime tests ...")
  if (NOT ANDROID OR NOT BUILD_X64)
    add_subdirectory ( common_runtime_test_type ) # until compilation errors are resolved
  endif (NOT ANDROID OR NOT BUILD_X64)
endif(INCLUDE_CMRT)

#Samples
option(INCLUDE_SMPLS "Include Samples into Build"  OFF)
if (INCLUDE_SMPLS)
  add_subdirectory( samples )
endif(INCLUDE_SMPLS)

