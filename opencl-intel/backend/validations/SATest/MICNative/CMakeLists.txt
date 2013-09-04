#
# Create native MIC server using icc compiler
#
function (SET_NATIVE_PATH ARG PATH1)
    file( TO_NATIVE_PATH "${PATH1}" MY_TEMP)
    if (WIN32)
        set( ${ARG} \"${MY_TEMP}\" PARENT_SCOPE )
    endif (WIN32)
endfunction (SET_NATIVE_PATH)

get_filename_component( TARGET_NAME ${SATEST_NATIVE_NAME} NAME_WE )

get_filename_component( EXE_NAME ${SATEST_NATIVE_NAME} NAME )

get_filename_component( EXE_FILE_EXTENSION ${SATEST_NATIVE_NAME} EXT )
set( BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/scripts )
set( OBJ_FILE_EXTENSION .o )

set( DEVICE MIC )

if (WIN32)
    set( DEVICE_COMPILER_SCRIPT compile.bat )
    set( DEVICE_LINKER_SCRIPT   link.bat )

    set( DEVICE_COMPILER  ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} )
    set( DEVICE_LINKER    ${BIN_DIR}/${DEVICE_LINKER_SCRIPT} )

    set( BACKEND_LIB_DIR  ${OCL_BINARY_DIR}/${IMPLIB_SUBDIR}/%CONFIG_NAME% )
else (WIN32)

    set( DEVICE_COMPILER_SCRIPT compile.sh )
    set( DEVICE_LINKER_SCRIPT   link.sh )

    set( DEVICE_COMPILER  ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} )
    set( DEVICE_LINKER    ${BIN_DIR}/${DEVICE_LINKER_SCRIPT} )

    set( BACKEND_LIB_DIR  ${OCL_BINARY_DIR}/lib )
endif (WIN32)

#use the same COI version on linux and windows
add_definitions ( -DCOI_LIBRARY_VERSION=2 )

# icc executables and env scripts

if (WIN32)
    # Windows
    set( ICC_INSTALL_DIR     "C:\\Program Files (x86)\\Intel\\Composer XE" )
    if ( NOT EXISTS ${ICC_INSTALL_DIR})
        message( FATAL_ERROR "ICC for Xeon Phi(MIC) can't be found")
    endif ()

    execute_process(COMMAND ${ICC_INSTALL_DIR}\\bin\\intel64_mic\\icc -dumpversion OUTPUT_VARIABLE MIC_ICC_VERSION)
    if (  MIC_ICC_VERSION VERSION_LESS 13.0 )
        message(FATAL_ERROR "ICC ${MIC_ICC_VERSION} found, expected 13.x")
    endif()

    message(STATUS "MIC C COMPILER: ${ICC_INSTALL_DIR}\\bin\\intel64_mic\\icc, version ${MIC_ICC_VERSION}" )

    set( DEVICE_INIT_ENV_SCRIPT "${ICC_INSTALL_DIR}\\bin\\compilervars.bat" )

else (WIN32)
    # linux-mic target
    set( ICC_INSTALL_DIR /opt/intel/composerxe )
    if ( NOT EXISTS ${ICC_INSTALL_DIR})
        message( FATAL_ERROR "ICC for Xeon Phi(MIC) can't be found")
    endif ()

    execute_process(COMMAND ${ICC_INSTALL_DIR}/bin/icc -dumpversion OUTPUT_VARIABLE MIC_ICC_VERSION)
    if (  MIC_ICC_VERSION VERSION_LESS 13.0 )
        message(FATAL_ERROR "ICC ${MIC_ICC_VERSION} found, expected 13.x")
    endif()

    message(STATUS "MIC C COMPILER: ${ICC_INSTALL_DIR}/bin/icc, version ${MIC_ICC_VERSION}" )

    set( DEVICE_INIT_ENV_SCRIPT "${ICC_INSTALL_DIR}/bin/compilervars.sh" )
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
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                        COMMAND ${DEVICE_COMPILER} ${INSTALL_SUBDIR} ${FILE_FULL_PATH_NAME} ${OBJ_FILE}

                        MAIN_DEPENDENCY ${FILE_FULL_PATH_NAME}
                        IMPLICIT_DEPENDS CXX ${FILE_FULL_PATH_NAME}
                        DEPENDS ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} ${HEADER_FILES}
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
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                            COMMAND ${DEVICE_LINKER} ${INSTALL_SUBDIR} ${TARGET_EXE} ${OBJ_FILES}

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

    set( EXE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}_${EXE_NAME} )
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

check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_COMPILER_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_LINKER_SCRIPT})

# substitute vars in the scripts
if (NOT EXISTS ${BIN_DIR})
        file(MAKE_DIRECTORY "${BIN_DIR}")
endif()

# MIC compilation flags
set( MIC_FLAGS  -Werror -fPIC -wd279)

set( MIC_DEBUG_FLAGS    -g -O0 -D_DEBUG -DDEBUG )
set( MIC_RELEASE_FLAGS     -O3 -DNDEBUG )

#SET_NATIVE_PATH(COI_DIR ${COI_DIR})
SET_NATIVE_PATH(COI_DEVICE_LIB_PATH ${COI_DEVICE_LIB_PATH})
SET_NATIVE_PATH(COI_YOCTO_DEVICE_LIB_PATH ${COI_YOCTO_DEVICE_LIB_PATH})

# inlcude path
list( APPEND MIC_FLAGS
                -I ${CMAKE_CURRENT_SOURCE_DIR}
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../../Common
                -I ${BACKEND_ROOT_DIR}/ocl_cpu_backend
                -I ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
                -I ${BACKEND_ROOT_DIR}/../cl_api
                -I ${COI_DEVICE_INCLUDE_PATH}
                -I ${COI_YOCTO_DEVICE_INCLUDE_PATH}
                -I ${BACKEND_ROOT_DIR}/external/vtune/include
                )

# MIC linking flags
set( MIC_LINK_FLAGS      )

file( TO_NATIVE_PATH "${BACKEND_ROOT_DIR}/external/vtune/libknc" VTUNE_LIB_DIR)

# lib path
list( APPEND MIC_LINK_FLAGS
                # library search path at link
                -L ${COI_DEVICE_LIB_PATH}
                -L ${COI_YOCTO_DEVICE_LIB_PATH}
                #-L ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                -L ${BACKEND_LIB_DIR}
                -L ${VTUNE_LIB_DIR}

                # libs to link
                -lscif
                -lcoi_device
                -lOclDeviceExecutor

                -lpthread
                -ljitprofiling

                ############ Begin of statically linked libraries
                # The following line affects all libraries mentioned after it, add here
                # only libraries which must be linked statically
                -Wl,-Bstatic
                -lirc
                ############ End of statically linked libraries

                # add unused defined symbols to the dynamic symbol table - they will be used dynamically
                -rdynamic

                # perform static linkage for internally defined symbols if possible
                -Bsymbolic

                # statically link all intel-specific libs
                -static-intel

                # look for required shared libs in the same dir as exe resides
                #  \\\$ will result in passing \$ to the linker script
                -Wl,-rpath=\\\$ORIGIN -Wl,--enable-new-dtags
                )

set( MIC_DEBUG_LINK_FLAGS     )
set( MIC_RELEASE_LINK_FLAGS   )

if (NOT WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Debug

        list( APPEND MIC_FLAGS      ${MIC_DEBUG_FLAGS} )
        list( APPEND MIC_LINK_FLAGS ${MIC_DEBUG_LINK_FLAGS} )

    else (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Release

        list( APPEND MIC_FLAGS      ${MIC_RELEASE_FLAGS} )
        list( APPEND MIC_LINK_FLAGS ${MIC_RELEASE_LINK_FLAGS} )

    endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
endif (NOT WIN32)

# replace ; with spaces
foreach (FLAG ${MIC_FLAGS})
    set( FINAL_MIC_FLAGS "${FINAL_MIC_FLAGS} ${FLAG}")
endforeach( FLAG )

foreach (FLAG ${MIC_LINK_FLAGS})
    set( FINAL_MIC_LINK_FLAGS "${FINAL_MIC_LINK_FLAGS} ${FLAG}")
endforeach( FLAG )

if (WIN32)
    string( REPLACE  / \\ FINAL_MIC_FLAGS ${FINAL_MIC_FLAGS} )
endif(WIN32)

# FINAL_MIC_FLAGS and FINAL_MIC_LINK_FLAGS are used now
configure_file(${DEVICE_COMPILER_SCRIPT} ${DEVICE_COMPILER} @ONLY)
configure_file(${DEVICE_LINKER_SCRIPT} ${DEVICE_LINKER} @ONLY)

set(TARGET_SOURCES
    OpenCLMICNative.cpp
    WGContext.cpp
    ../Performance.cpp
    ../dbg_utils.cpp
)

set (HEADER_FILES
    WGContext.h
    common.h
    )

add_device_executable(${TARGET_NAME} ${TARGET_SOURCES})

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/SATest )

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Debug_${EXE_NAME} DESTINATION bin
           CONFIGURATIONS Debug RENAME ${EXE_NAME}  )
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Release_${EXE_NAME} DESTINATION bin
           CONFIGURATIONS Release RENAME ${EXE_NAME} )

add_dependencies(${TARGET_NAME} OclDeviceExecutor )

