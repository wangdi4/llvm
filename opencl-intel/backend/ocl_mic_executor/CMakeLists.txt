#
# Create native MIC Execution engine using icc compiler
#
if (NOT INCLUDE_MIC_DEVICE)
    message (FATAL_ERROR "Entered this file without INCLUDE_MIC_DEVICE ON")
endif ()


set( TARGET_NAME OclDeviceExecutor )
set( TARGET_LIB_DIR ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR} )

set( SCRIPT_DIR ${CMAKE_CURRENT_BINARY_DIR}/scripts )
set( LIB_FILE_EXTENSION .a )
set( OBJ_FILE_EXTENSION .o )

set( DEVICE MIC )

if (WIN32)
    set( FORCE_LINUX ON )
    # Windows
    set( LIB_DIR        ${OCL_BINARY_DIR}/${IMPLIB_SUBDIR}/%CONFIG_NAME% )

    set( DEVICE_COMPILER_SCRIPT compile_win64_mic.bat )
    set( DEVICE_ARCHIVER_SCRIPT archive_win64_mic.bat )

    set( DEVICE_COMPILER ${SCRIPT_DIR}/${DEVICE_COMPILER_SCRIPT} )
    set( DEVICE_ARCHIVER ${SCRIPT_DIR}/${DEVICE_ARCHIVER_SCRIPT} )

    string( REPLACE  ";" " -I " LLVM_INCLUDE_DIR "${LLVM_INCLUDE_DIR}" )
    string( REPLACE  \${CMAKE_CFG_INTDIR} %CONFIG_NAME% LLVM_INCLUDE_DIR "${LLVM_INCLUDE_DIR}" )

else ()
    # Linux    
    set( LIB_DIR        ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR} )

    set( DEVICE_COMPILER_SCRIPT compile_linux_mic.sh )
    set( DEVICE_ARCHIVER_SCRIPT archive_linux_mic.sh )

    set( DEVICE_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${DEVICE_COMPILER_SCRIPT} )
    set( DEVICE_ARCHIVER ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${DEVICE_ARCHIVER_SCRIPT} )
endif (WIN32)

#
# external tools
#

# icc executables and env scripts

if (WIN32)
    # Windows
    set( ICC_INSTALL_DIR "C:\\Program Files (x86)\\Intel\\Composer XE" )
    if ( NOT EXISTS ${ICC_INSTALL_DIR})
        message( FATAL_ERROR "ICC for Xeon Phi(MIC) can't be found")
    endif ()

    execute_process(COMMAND ${ICC_INSTALL_DIR}\\bin\\intel64_mic\\icc -dumpversion OUTPUT_VARIABLE MIC_ICC_VERSION)
    if (  MIC_ICC_VERSION VERSION_LESS 13.0 )
        message(FATAL_ERROR "ICC ${MIC_ICC_VERSION} found, expected 13.x")
    endif()

    message(STATUS "MIC C COMPILER: ${ICC_INSTALL_DIR}\\bin\\intel64_mic\\icc, version ${MIC_ICC_VERSION}" )

    set( ICC_MIC_ENV_SCRIPT "${ICC_INSTALL_DIR}\\bin\\compilervars.bat" )

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

    set( ICC_MIC_ENV_SCRIPT "${ICC_INSTALL_DIR}/bin/compilervars.sh" )
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
                        COMMAND ${DEVICE_COMPILER} ${INSTALL_SUBDIR} ${FILE_FULL_PATH_NAME} ${OBJ_FILE}
                        MAIN_DEPENDENCY ${FILE_FULL_PATH_NAME}
                        IMPLICIT_DEPENDS CXX
                        DEPENDS ${DEVICE_COMPILER}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Compiling ${FILE_NAME} for ${DEVICE}"
                        VERBATIM
                        )

endfunction ( COMPILE_DEVICE_CXX_FILE )

#
# Usage
#     ARCHIVE_DEVICE_LIB( <full_path_to_target_exe> <full_path_to_obj>.... )
#
function ( ARCHIVE_DEVICE_LIB TARGET_LIB )
    get_filename_component( FILE_NAME ${TARGET_LIB} NAME )

    if (${ARGC} GREATER  1)
        set( OBJ_FILES ${ARGN})

        add_custom_command( OUTPUT ${TARGET_LIB}
                            COMMAND ${DEVICE_ARCHIVER} ${TARGET_LIB} ${OBJ_FILES}
                            DEPENDS  ${OBJ_FILES} ${DEVICE_ARCHIVER}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                            COMMENT "Archiving ${FILE_NAME} for ${DEVICE}"
                            VERBATIM
                            )
    endif (${ARGC} GREATER 1)

endfunction ( ARCHIVE_DEVICE_LIB )


#
# Usage
#     add_device_library( <target_name> <input_cxx_files>.... )
#
function ( add_device_library TARGET_NAME )

    set( LIB_FILE ${TARGET_LIB_DIR}/lib${TARGET_NAME}${LIB_FILE_EXTENSION} )
    if (WIN32)
        file( TO_NATIVE_PATH "${LIB_FILE}" LIB_FILE)
    endif(WIN32)
    set( OBJ_LIST )

    if (${ARGC} GREATER  1)
        foreach( FILE ${ARGN} )
            set( SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} )

            get_filename_component( FILE_NAME ${FILE} NAME_WE )
            set( OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${FILE_NAME}${OBJ_FILE_EXTENSION} )

            if (WIN32)
                file( TO_NATIVE_PATH "${SRC_FILE}" SRC_FILE)
                file( TO_NATIVE_PATH "${OBJ_FILE}" OBJ_FILE)
            endif(WIN32)

            COMPILE_DEVICE_CXX_FILE( ${SRC_FILE} ${OBJ_FILE} )
            list( APPEND OBJ_LIST ${OBJ_FILE} )
        endforeach( FILE )

        ARCHIVE_DEVICE_LIB ( ${LIB_FILE} ${OBJ_LIST} )

        add_custom_target( ${TARGET_NAME} ALL DEPENDS ${LIB_FILE} )
    else ()
        message( FATAL_ERROR "No input files specified for ${TARGET_NAME}" )
    endif (${ARGC} GREATER 1)

endfunction ( add_device_library )


#################################################################################################

check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_COMPILER_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_ARCHIVER_SCRIPT})

if (NOT EXISTS ${SCRIPT_DIR})
        file(MAKE_DIRECTORY "${SCRIPT_DIR}")
endif()

# MIC compilation flags
set( MIC_FLAGS  -Werror
                -fstack-security-check
                -use-msasm
                -wd279
                -D__STDC_LIMIT_MACROS
                -D__STDC_CONSTANT_MACROS
            )

list( APPEND MIC_FLAGS -DKNC_CARD )

set( MIC_DEBUG_FLAGS    -g -O0 -D_DEBUG )
set( MIC_RELEASE_FLAGS     -O3 -DNDEBUG )

# inlcude path
list( APPEND MIC_FLAGS
                -I ${CMAKE_CURRENT_SOURCE_DIR}
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../../cl_api
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../ocl_cpu_backend
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../utils
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../arch_headers
                -I ${CMAKE_SOURCE_DIR}/backend/ocl_cpu_backend
                -I ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
                -I ${CMAKE_SOURCE_DIR}/backend/plugin_manager
                -I ${LLVM_INCLUDE_DIR}
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/itt/include
                -I ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
                )

if (NOT WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        # Debug
        list( APPEND MIC_FLAGS      ${MIC_DEBUG_FLAGS} )
    else ()
        # Release
        list( APPEND MIC_FLAGS      ${MIC_RELEASE_FLAGS} )
    endif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
endif (NOT WIN32)

# replace ; with spaces
foreach (FLAG ${MIC_FLAGS})
    set( FINAL_MIC_FLAGS "${FINAL_MIC_FLAGS} ${FLAG}")
endforeach( FLAG )

if (WIN32)
    string( REPLACE  / \\ FINAL_MIC_FLAGS "${FINAL_MIC_FLAGS}" )
endif(WIN32)

# FINAL_MIC_FLAGS and FINAL_MIC_LINK_FLAGS are used now
configure_file(${DEVICE_COMPILER_SCRIPT} ${DEVICE_COMPILER} @ONLY)
configure_file(${DEVICE_ARCHIVER_SCRIPT} ${DEVICE_ARCHIVER} @ONLY) 

set(TARGET_SOURCES
    MICDeviceServiceFactory.cpp
    DynamicLibraryLoader.cpp
    MICDetect.cpp
    ../ocl_cpu_backend/shared_builtin_functions.cpp
    ../ocl_cpu_backend/mic_builtin_functions.cpp
    ../ocl_cpu_backend/TargetDescription.cpp
    ../ocl_cpu_backend/MICDeviceBackendFactory.cpp
    ../ocl_cpu_backend/ExecutionService.cpp
    ../ocl_cpu_backend/MICExecutionService.cpp
    ../ocl_cpu_backend/MICSerializationService.cpp
    ../ocl_cpu_backend/Program.cpp
    ../ocl_cpu_backend/MICProgram.cpp
    MICDeviceBitCodeContainer.cpp
    ../ocl_cpu_backend/ModuleJITHolder.cpp
    ../ocl_cpu_backend/Kernel.cpp
    ../ocl_cpu_backend/MICKernel.cpp
    ../ocl_cpu_backend/KernelProperties.cpp
    ../ocl_cpu_backend/MICKernelProperties.cpp
    ../ocl_cpu_backend/MICJITContainer.cpp
    ../utils/TypeAlignment.cpp
    ../ocl_cpu_backend/Binary.cpp
    ../utils/ImplicitArgsUtils.cpp
    ../utils/Atomics.cpp
    ../ocl_cpu_backend/Executable.cpp
    ../ocl_cpu_backend/MICExecutable.cpp
    ../utils/FunctionArgument.cpp
    ../ocl_cpu_backend/opencl_printf_ext.cpp
    ../ocl_cpu_backend/opencl_mic_printf_ext.cpp
    main.cpp)

add_device_library(${TARGET_NAME} ${TARGET_SOURCES})
