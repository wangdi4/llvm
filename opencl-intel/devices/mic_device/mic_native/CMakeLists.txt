#
# Create native MIC server using icc compiler
#
get_filename_component( TARGET_NAME ${MIC_NATIVE_SERVER_EXE} NAME_WE )

get_filename_component( EXE_FILE_EXTENSION ${MIC_NATIVE_SERVER_EXE} EXT )
set( BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR} )
set( OBJ_FILE_EXTENSION .o )

set( DEVICE   MIC )

if (WIN32)
    # Windows
    message( WARNING "MIC native server creation is not supported on Windows yet!")
else ()
    # Linux
#    set( TMP_ICC_INSTALL_DIR $ENV{ICC_INSTALL_DIR} )
#    if (TMP_ICC_INSTALL_DIR)
#        # if environment ICC_INSTALL_DIR is set
#        set( DEVICE_INIT_ENV_SCRIPT "${TMP_ICC_INSTALL_DIR}/bin/compilervars.sh" )
#    else ()
#        message( FATAL_ERROR "ICC is not found! Setenv ICC_INSTALL_DIR to point to the ICC with MIC support" )
#    endif (TMP_ICC_INSTALL_DIR)
#    unset( TMP_ICC_INSTALL_DIR )

    set( DEVICE_COMPILER_SCRIPT compile.sh )
    set( DEVICE_LINKER_SCRIPT   link.sh )

    set( DEVICE_COMPILER ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} )
    set( DEVICE_LINKER   ${BIN_DIR}/${DEVICE_LINKER_SCRIPT} )
endif (WIN32)

#
# Check for file/dir existence
#    check_file(FILE)
#
function (check_file FILE )
    if (NOT EXISTS ${FILE})
        message( FATAL_ERROR "Cannot find ${FILE}!")
    endif()
endfunction( check_file )

#
# Usage
#     COMPILE_DEVICE_CXX_FILE( <cxx_file_full_path_name> <obj_file_full_path_name>)
#
function ( COMPILE_DEVICE_CXX_FILE FILE_FULL_PATH_NAME OBJ_FILE )
    get_filename_component( FILE_NAME ${FILE_FULL_PATH_NAME} NAME )

    add_custom_command( OUTPUT ${OBJ_FILE}

                        COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
                        COMMAND ${DEVICE_COMPILER} ${FILE_FULL_PATH_NAME} ${OBJ_FILE}

                        MAIN_DEPENDENCY ${FILE_FULL_PATH_NAME}
                        IMPLICIT_DEPENDS CXX
                        DEPENDS ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Compiling ${FILE_NAME} for ${DEVICE}"
                        VERBATIM
                        )

endfunction ( COMPILE_DEVICE_CXX_FILE )

#
# Usage
#     LINK_DEVICE_EXE( <full_path_to_target_exe> <full_path_to_obj>.... )
#
function ( LINK_DEVICE_EXE TARGET_EXE )
    get_filename_component( FILE_NAME ${TARGET_EXE} NAME )

    if (${ARGC} GREATER  1)
        set( OBJ_FILES ${ARGN})

        add_custom_command( OUTPUT ${TARGET_EXE}

                            COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
                            COMMAND ${DEVICE_LINKER} ${TARGET_EXE} ${OBJ_FILES}

                            DEPENDS  ${OBJ_FILES} ${BIN_DIR}/${DEVICE_LINKER_SCRIPT}
                            WORKING_DIRECTORY ${BIN_DIR}
                            COMMENT "Linking ${FILE_NAME} for ${DEVICE}"
                            VERBATIM
                            )
    endif (${ARGC} GREATER 1)

endfunction ( LINK_DEVICE_EXE )


#
# Usage
#     add_device_executable( <target_name> <input_cxx_files>.... )
#
function ( add_device_executable TARGET_NAME )

    set( EXE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}${EXE_FILE_EXTENSION} )
    set( OBJ_LIST )

    if (${ARGC} GREATER  1)
        foreach( FILE ${ARGN} )
            set( SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} )

            get_filename_component( FILE_NAME ${FILE} NAME_WE )
            set( OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}${OBJ_FILE_EXTENSION} )

            COMPILE_DEVICE_CXX_FILE( ${SRC_FILE} ${OBJ_FILE} )
            list( APPEND OBJ_LIST ${OBJ_FILE} )
        endforeach( FILE )

        LINK_DEVICE_EXE( ${EXE_FILE} ${OBJ_LIST} )

        add_custom_target( ${TARGET_NAME} ALL DEPENDS ${EXE_FILE} )
    else ()
        message( FATAL_ERROR "No input files specified for ${TARGET_NAME}" )
    endif (${ARGC} GREATER 1)

endfunction ( add_device_executable )


#################################################################################################

#check_file(${DEVICE_INIT_ENV_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_COMPILER_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_LINKER_SCRIPT})

# substitute vars in the scripts
if (NOT EXISTS ${BIN_DIR})
        file(MAKE_DIRECTORY "${BIN_DIR}")
endif()

# MIC compilation flags
set( MIC_FLAGS  -Werror
#                -std=c++0x
                -fstack-security-check
                -use-msasm
                -mCG_lrb_rex_workaround=F
                -mCG_insert_nops_between_gathers_scatters=F
                -mCG_lrb_vgather_b2b_warns=F
                -mCG_lrb_workaround_vgather_vscatter_pairing=F
                -mCG_lrb_dp_workaround=F
                -sox
            )

set( MIC_DEBUG_FLAGS    -g -O0 -D_DEBUG )
set( MIC_RELEASE_FLAGS     -O3 -DNDEBUG )

# inlcude path
list( APPEND MIC_FLAGS
                -I ${CMAKE_CURRENT_SOURCE_DIR}
				-I ${CMAKE_CURRENT_SOURCE_DIR}/../
				-I ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
                -I ${OCL_SOURCE_DIR}/cl_api
                -I ${OCL_SOURCE_DIR}/externals/tbb/include
                -I ${COI_SDK_PATH}/include
                )

if ( MIC_TESTING_MODE )				
	list( APPEND MIC_FLAGS
					-DNDRANGE_UNIT_TEST
					)
endif ( MIC_TESTING_MODE )

# MIC linking flags
set( MIC_LINK_FLAGS      )

# lib path
list( APPEND MIC_LINK_FLAGS
                # library search path at link
                -L ${COI_SDK_PATH}/device-linux-${DEBUG_RELEASE}/lib
                -L ${BACKEND_SDK_PATH}/lib/Lin64_${INSTALL_SUBDIR}

                # libs to link
                -lcoi_device
                -lOclDeviceExecutor

                # add unused defined symbols to the dynamic symbol table - they will be used dynamically
                -rdynamic

                # perform static linkage for internally defined symbols if possible
                -Bsymbolic

                # statically link all intel-specific libs
                -static-intel

                # hide all defined symbols from the loader, except specified in the file
                -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/MicNative_linux_exports.txt

                # look for required shared libs in the same dir as exe resides
                #  \\\$ will result in passing \$ to the linker script
                -Wl,-rpath=\\\$ORIGIN -Wl,--enable-new-dtags
                )

set( MIC_DEBUG_LINK_FLAGS     )
set( MIC_RELEASE_LINK_FLAGS   )

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    # Debug

    list( APPEND MIC_FLAGS      ${MIC_DEBUG_FLAGS} )
    list( APPEND MIC_LINK_FLAGS ${MIC_DEBUG_LINK_FLAGS} )

else ()
    # Release

    list( APPEND MIC_FLAGS      ${MIC_RELEASE_FLAGS} )
    list( APPEND MIC_LINK_FLAGS ${MIC_RELEASE_LINK_FLAGS} )

endif (${CMAKE_BUILD_TYPE} STREQUAL Debug)

# replace ; with spaces
foreach (FLAG ${MIC_FLAGS})
    set( FINAL_MIC_FLAGS "${FINAL_MIC_FLAGS} ${FLAG}")
endforeach( FLAG )

foreach (FLAG ${MIC_LINK_FLAGS})
    set( FINAL_MIC_LINK_FLAGS "${FINAL_MIC_LINK_FLAGS} ${FLAG}")
endforeach( FLAG )

# FINAL_MIC_FLAGS and FINAL_MIC_LINK_FLAGS are used now
configure_file(${DEVICE_COMPILER_SCRIPT} ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} @ONLY)
configure_file(${DEVICE_LINKER_SCRIPT} ${BIN_DIR}/${DEVICE_LINKER_SCRIPT} @ONLY)

set(FILES_TO_COPY ${TARGET_NAME}${EXE_FILE_EXTENSION} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

add_device_executable(${TARGET_NAME} ${TARGET_SOURCES})
COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

