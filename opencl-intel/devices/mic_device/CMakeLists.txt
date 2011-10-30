#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
option( MIC_TESTING_MODE OFF )
if (NOT MIC_TESTING_MODE)
	set( TARGET_NAME mic_device )
else (NOT MIC_TESTING_MODE)
    set( TARGET_NAME mic_device_testing_mode )
	add_definitions( -DNDRANGE_UNIT_TEST )
endif (NOT MIC_TESTING_MODE)
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

set( COI_SDK_PATH           ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/coi )
set( BACKEND_SDK_PATH_RELATIVE BackEnd_SDK )
set( BACKEND_SDK_PATH       ${CMAKE_CURRENT_SOURCE_DIR}/${BACKEND_SDK_PATH_RELATIVE} )
set( MIC_BACKEND_DLL_NAME   OclMicBackEnd)
set( MIC_NATIVE_SERVER_EXE  mic_server )
set( COI_HOST_LIB           coi_host )

add_definitions( -DMIC_NATIVE_SERVER_EXE="${MIC_NATIVE_SERVER_EXE}" -DMIC_BACKEND_DLL_NAME="${IMPLIB_PREFIX}${MIC_BACKEND_DLL_NAME}${IMPLIB_SUFFIX}" )

include_directories( BEFORE
                            ${HW_UTILS_HEADERS}
                            ${OCL_BINARY_DIR}
                            ${COI_SDK_PATH}/include
                            mic_native )

if(WIN32)
include_directories(
                            ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/include )
endif(WIN32)

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )

# for Linux only
# COI stopped releasing their Debug version - only Release
#string(TOLOWER  ${CMAKE_BUILD_TYPE} DEBUG_RELEASE)
set( DEBUG_RELEASE release )
file( GLOB COI_LIBS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${COI_SDK_PATH}/host-linux-${DEBUG_RELEASE}/lib/*.so* )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()


link_directories(   ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/llvm_wrap/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/lib/${INSTALL_SUBDIR}
                    ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
                    ${COI_SDK_PATH}/host-linux-${DEBUG_RELEASE}/lib
                    ${BACKEND_SDK_PATH}/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}/
                )

if(WIN32)
link_directories(
                    ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM_BUILD/lib/${INSTALL_SUBDIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/ocl_cpu_backend/lib/Win32
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/libs/${TAL_LIB_DIR_SUFFIX}
                )
else(WIN32)
link_directories(
                    ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM_BUILD/lib/${INSTALL_SUBDIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/ocl_cpu_backend/lib/Linux64
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/libs/${TAL_LIB_DIR_SUFFIX}
                )

    file( GLOB MIC_BACKEND_LIBS_TO_COPY RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${BACKEND_SDK_PATH_RELATIVE}/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}/?*.* )
endif(WIN32)

if (WIN32)
     add_definitions( -D_WINDOWS -DUSE_GPA )
else ()
    add_definitions( -D_LINUX )
    message( WARNING " ERROR-in-MIC-device: Task Analizer DLLs for Linux are not ready yet" )
endif (WIN32)

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

add_dependencies( ${TARGET_NAME} clang OclCpuBackEnd)

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

install(FILES ${COI_LIBS} ${MIC_BACKEND_LIBS_TO_COPY} DESTINATION bin)

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      task_executor
                                      ${COI_HOST_LIB}
                                      ${LINK_LIBS} )

if (NOT WIN32)
    if (NOT MIC_TESTING_MODE)
        SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} MicDefice_linux_exports.txt )
	endif (NOT MIC_TESTING_MODE)
endif (NOT WIN32)

add_dependencies( ${TARGET_NAME} LLVM_copy_tools )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

add_subdirectory( mic_native )

add_subdirectory( tests )

