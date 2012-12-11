add_subdirectory( cpu_device_test_type )
add_subdirectory( framework_test_type )
if(BUILD_QTGUI AND (NOT ${CMAKE_SYSTEM} STREQUAL "Windows-6.2") ) #QT is not working with win8
	add_subdirectory( kernel_builder_module)
endif(BUILD_QTGUI AND (NOT ${CMAKE_SYSTEM} STREQUAL "Windows-6.2") )
add_subdirectory( debugger_test_type )
add_subdirectory( tools_test_type)
add_subdirectory( task_executor_test_type )
if ( INCLUDE_MIC_DEVICE )
    add_subdirectory( mic_device_test_type )
endif( INCLUDE_MIC_DEVICE )



option(INCLUDE_CONFORMANCE_TESTS "Include Conformance Tests Into Build"  OFF)
if (INCLUDE_CONFORMANCE_TESTS OR INCLUDE_CONFORMANCE_1_2_TESTS)
    
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

endif (INCLUDE_CONFORMANCE_TESTS OR INCLUDE_CONFORMANCE_1_2_TESTS)

if (INCLUDE_CMRT)
    message ("Adding Common runtime tests ...")
    add_subdirectory ( common_runtime_test_type )
endif(INCLUDE_CMRT)

#Samples
option(INCLUDE_SMPLS "Include Samples into Build"  OFF)
if (INCLUDE_SMPLS)
    add_subdirectory( samples )
endif(INCLUDE_SMPLS)
