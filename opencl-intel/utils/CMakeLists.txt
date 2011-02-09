if (NOT WIN32)	
	add_custom_target(build_builtins ALL				
				COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cl_builtin_functions/gen_linux.sh  ${CMAKE_CURRENT_SOURCE_DIR}/cl_builtin_functions/  ${INSTALL_SUBDIR}							
				COMMAND $(MAKE) -C  ${CMAKE_CURRENT_SOURCE_DIR}/cl_builtin_functions/				
				COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/cl_builtin_functions/Lin64/*.so ${OCL_INSTALL_DIR}
				COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/cl_builtin_functions/Lin64/*.rtl ${OCL_INSTALL_DIR}
				COMMENT "Building cl_builtin_functions Cfg: ${INSTALL_SUBDIR}" )
		
else()
	add_subdirectory(cl_builtin_functions)	
endif(NOT WIN32)

add_subdirectory( cl_hw_utils )
add_subdirectory( cl_logger )
add_subdirectory( cl_sys_utils )
add_subdirectory( task_executor )

