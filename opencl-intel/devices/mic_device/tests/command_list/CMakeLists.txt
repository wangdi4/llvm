if (MIC_TESTING_MODE)
	#include helping functions of cmake.
	include (../../../../cmake_utils/CMakeFuncs.txt)

	set( TARGET command_list_test_type )

	include_directories( BEFORE
							../../ 
							../../../../backend/ocl_cpu_backend 
							../../../../backend/llvm/include
							)
					
	FIND_SOURCES()
	CALCULATE_TARGET_SOURCES()

	add_executable( ${TARGET} ${TARGET_SOURCES} )

	target_link_libraries( ${TARGET} 
								mic_device_testing_mode 
								task_executor
						 )
    install(TARGETS ${TARGET_NAME}
			RUNTIME DESTINATION bin
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION lib)


	add_dependencies( ${TARGET} mic_device_testing_mode )
	add_dependencies( ${TARGET} mic_native )	
else (MIC_TESTING_MODE)
	message( "command_list_test_type does not build (MIC_TESTING_MODE is OFF)" )
endif (MIC_TESTING_MODE)