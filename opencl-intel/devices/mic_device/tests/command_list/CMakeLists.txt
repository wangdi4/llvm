if (MIC_TESTING_MODE)
	#include helping functions of cmake.
	include (../../../../cmake_utils/CMakeFuncs.txt)

	set( TARGET command_list_test_type )

	include_directories( BEFORE
							../../ 
							../../../../externals/llvm_wrap/llvm/ocl_cpu_backend 
							../../../../externals/llvm_wrap/llvm/llvm/include
							)
					
	FIND_SOURCES()
	CALCULATE_TARGET_SOURCES()

	add_executable( ${TARGET} ${TARGET_SOURCES} )

	target_link_libraries( ${TARGET} 
								mic_device_testing_mode 
								task_executor
						 )

	COPY_GENERATED_FILES_POST_BUILD( ${TARGET} ${TARGET}${CMAKE_EXECUTABLE_SUFFIX} )

	add_dependencies( ${TARGET} mic_device_testing_mode )
	add_dependencies( ${TARGET} mic_native )	
else (MIC_TESTING_MODE)
	message( "command_list_test_type does not build (MIC_TESTING_MODE is OFF)" )
endif (MIC_TESTING_MODE)