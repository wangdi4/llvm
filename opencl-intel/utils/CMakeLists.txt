if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

message( STATUS "Frontend LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

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

if(WIN32)
macro( CREATE_ASM_RULES ADD_TO_SOURCES_LIST_VAR )

    if (${ARGC} GREATER  1)
        foreach( FILE ${ARGN} )
            get_filename_component( FILE_NAME ${FILE} NAME_WE )
            set( BIN_DIR  ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR} )
            set( OBJ_FILE ${BIN_DIR}/${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION} )
            set( SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} )

            if (CMAKE_ASM_OUTPUT_NAME_FLAG)
                                set( OBJ_OUTPUT_NAME_FLAG ${CMAKE_ASM_OUTPUT_NAME_FLAG} ${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION} )
                        endif (CMAKE_ASM_OUTPUT_NAME_FLAG)

            add_custom_command(OUTPUT ${OBJ_FILE}
                               COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
                               COMMAND ${CMAKE_COMMAND} -E chdir ${BIN_DIR}
                                                             ${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS}
                                                             ${CMAKE_ASM_INCLUDE_DIR_FLAG} ${CMAKE_CURRENT_SOURCE_DIR}
                                                             ${OBJ_OUTPUT_NAME_FLAG}
                                                             ${SRC_FILE}
                               MAIN_DEPENDENCY ${SRC_FILE}
                               VERBATIM
                               )

            list(APPEND OBJ_FILES ${OBJ_FILE})
            list(APPEND SRC_FILES ${SRC_FILE})
        endforeach( FILE )
    endif (${ARGC} GREATER 1)

    if (DEFINED OBJ_FILES)
        set (${ADD_TO_SOURCES_LIST_VAR} ${SRC_FILES} ${OBJ_FILES})
        set_source_files_properties( ${OBJ_FILES} PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE )
    endif(DEFINED OBJ_FILES)

endmacro( CREATE_ASM_RULES )
endif(WIN32)
