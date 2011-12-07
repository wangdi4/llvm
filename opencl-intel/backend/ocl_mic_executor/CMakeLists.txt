#
# Create native MIC Execution engine using icc compiler
#
set( TARGET_NAME OclDeviceExecutor )

set( LIB_FILE_EXTENSION .a )
set( BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR} )
set( OBJ_FILE_EXTENSION .o )

set( DEVICE   MIC )

if (WIN32)
    # Windows
    message( WARNING "MIC executor creation is not supported on Windows yet!")
else ()
    # Linux    
    set( DEVICE_COMPILER_SCRIPT compile_linux_mic.sh )
    set( DEVICE_ARCHIVER_SCRIPT archive_linux_mic.sh )

    set( DEVICE_COMPILER ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} )
    set( DEVICE_ARCHIVER ${BIN_DIR}/${DEVICE_ARCHIVER_SCRIPT} )
endif (WIN32)

#
# external tools
#

# icc executables and env scripts

if (WIN32)
    # Windows
    message( WARNING "MIC executor build is not supported on Windows yet!")
else (WIN32)
  # linux-mic target
  set (ICC_MIC_CL icpc)
  set (ICC_MIC_AR ar)
  # check env. variable for the icc mic compiler
  set(ICC_MIC_COMPILER $ENV{ICC_MIC_COMPILER_PATH})
  if(NOT ICC_MIC_COMPILER)
    if (EXISTS /opt/intel/composerxe_mic/bin)
      set( ICC_MIC_COMPILER /opt/intel/composerxe_mic/bin/ )
	  message ("ICC MIC Compiler: ${ICC_MIC_COMPILER}")
	  set(MIC_TARGET "ON")
    else()
      message (WARNING "ICC MIC Compiler not found, No MIC target Build. \nIf want to build MIC target set the enviroment variable ICC_MIC_COMPILER_PATH")
      set(MIC_TARGET "OFF")
	endif()
  else()
    message ("ICC MIC Compiler: ${ICC_MIC_COMPILER}")
	set(MIC_TARGET "ON")
  endif()
  
  if(MIC_TARGET)
    # search for the compilervars script
    set (ICC_MIC_ENV_SCRIPT compilervars.sh)
    
    # check if the script exists
    if (NOT EXISTS "${ICC_MIC_COMPILER}/${ICC_MIC_ENV_SCRIPT}")
	  message (FATAL_ERROR "ICC MIC Compiler Script do not exist, please set the enviroment variable ICC_MIC_COMPILER_PATH")
    else()
      set(ICC_MIC_ENV_SCRIPT ${ICC_MIC_COMPILER}/${ICC_MIC_ENV_SCRIPT})
    endif()
	message("ICC MIC Compiler Script: ${ICC_MIC_ENV_SCRIPT}")
  endif()
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
#     ARCHIVE_DEVICE_LIB( <full_path_to_target_exe> <full_path_to_obj>.... )
#
function ( ARCHIVE_DEVICE_LIB TARGET_LIB )
    get_filename_component( FILE_NAME ${TARGET_LIB} NAME )

    if (${ARGC} GREATER  1)
        set( OBJ_FILES ${ARGN})

        add_custom_command( OUTPUT ${TARGET_LIB}

                            COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
                            COMMAND ${DEVICE_ARCHIVER} ${TARGET_LIB} ${OBJ_FILES}

                            DEPENDS  ${OBJ_FILES} ${BIN_DIR}/${DEVICE_ARCHIVER_SCRIPT}
                            WORKING_DIRECTORY ${BIN_DIR}
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

    set( LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/../lib/lib${TARGET_NAME}${LIB_FILE_EXTENSION} )
    set( OBJ_LIST )

    if (${ARGC} GREATER  1)
        foreach( FILE ${ARGN} )
            set( SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} )

            get_filename_component( FILE_NAME ${FILE} NAME_WE )
            set( OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}${OBJ_FILE_EXTENSION} )

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

if (NOT EXISTS ${BIN_DIR})
        file(MAKE_DIRECTORY "${BIN_DIR}")
endif()

# MIC compilation flags
set( MIC_FLAGS  -Werror
                -fstack-security-check
                -use-msasm
                -wd279
            )
			
set( MIC_DEBUG_FLAGS    -g -O0 -D_DEBUG )
set( MIC_RELEASE_FLAGS     -O3 -DNDEBUG )

# inlcude path
list( APPEND MIC_FLAGS
                -I ${CMAKE_CURRENT_SOURCE_DIR}
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../../cl_api
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../ocl_cpu_backend
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../arch_headers
                )

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    # Debug
    list( APPEND MIC_FLAGS      ${MIC_DEBUG_FLAGS} )
else ()
    # Release
    list( APPEND MIC_FLAGS      ${MIC_RELEASE_FLAGS} )
endif (${CMAKE_BUILD_TYPE} STREQUAL Debug)

# replace ; with spaces
foreach (FLAG ${MIC_FLAGS})
    set( FINAL_MIC_FLAGS "${FINAL_MIC_FLAGS} ${FLAG}")
endforeach( FLAG )


# FINAL_MIC_FLAGS and FINAL_MIC_LINK_FLAGS are used now
if(MIC_TARGET)
configure_file(${DEVICE_COMPILER_SCRIPT} ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} @ONLY)
configure_file(${DEVICE_ARCHIVER_SCRIPT} ${BIN_DIR}/${DEVICE_ARCHIVER_SCRIPT} @ONLY) 
endif()

set(TARGET_SOURCES
    MICDeviceServiceFactory.cpp
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
    ../ocl_cpu_backend/TypeAlignment.cpp
    ../ocl_cpu_backend/Binary.cpp
    ../ocl_cpu_backend/ImplicitArgsUtils.cpp
    ../ocl_cpu_backend/Executable.cpp
    ../ocl_cpu_backend/MICExecutable.cpp
    ../ocl_cpu_backend/FunctionArgument.cpp
    main.cpp)

if(MIC_TARGET) 
add_device_library(${TARGET_NAME} ${TARGET_SOURCES})
endif()
