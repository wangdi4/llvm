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
option( ENABLE_MIC_TRACER OFF )
if ( ENABLE_MIC_TRACER )
    add_definitions( -DENABLE_MIC_TRACER )
endif( ENABLE_MIC_TRACER )

option( MIC_USE_OMP OFF )
if (MIC_USE_OMP)
	add_definitions( -D__MIC_DA_OMP__ )
endif (MIC_USE_OMP)

set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
set(HW_UTILS_PROJ_NAME cl_hw_utils)

add_definitions( -DKNC_CARD )

add_definitions ( -DCOI_LIBRARY_VERSION=2 )

set( MIC_BACKEND_DLL_NAME   OclCpuBackEnd)
set( MIC_NATIVE_SERVER_EXE  mic_server )
set( COI_HOST_LIB           coi_host )
set( MIC_NATIVE_SUBDIR_NAME mic_native )
if (WIN32)
    set( COI_HOST_DLL_NAME		coi_host.dll )
else (WIN32)
    set( COI_HOST_DLL_NAME		libcoi_host.so )
endif(WIN32)


add_definitions( -DMIC_NATIVE_SERVER_EXE="${MIC_NATIVE_SERVER_EXE}"
                 -DMIC_BACKEND_DLL_NAME="${IMPLIB_PREFIX}${MIC_BACKEND_DLL_NAME}${BUILD_PLATFORM}${DLL_SUFIX}"
                 -DMIC_NATIVE_SUBDIR_NAME="${MIC_NATIVE_SUBDIR_NAME}"
                 -DCOI_HOST_DLL_NAME="${COI_HOST_DLL_NAME}"
                 -DUSE_ITT
               )
               
# Internal debug only
if (USE_ITT_INTERNAL)
	add_definitions( -DUSE_ITT_INTERNAL )
endif()

include_directories( BEFORE
                            ../../fe_compilers/clang_compiler
                            ${HW_UTILS_HEADERS}
                            ${OCL_BINARY_DIR}
                            ${COI_HOST_INCLUDE_PATH}
                            ${COI_YOCTO_HOST_INCLUDE_PATH}
                            mic_native )

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )



FIND_SOURCES( ${IGNORE_SOURCES} )
CALCULATE_TARGET_SOURCES()

include( ../../cmake_utils/itt.cmake )

link_directories(   ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/llvm_wrap/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/lib/${INSTALL_SUBDIR}
                    ${COI_HOST_LIB_PATH}
                    ${COI_YOCTO_HOST_LIB_PATH}
                )

if(WIN32)
	link_directories(
                    ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM_BUILD/lib/${INSTALL_SUBDIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/ocl_cpu_backend/lib/Win32
                )
else(WIN32)
	link_directories(
                    ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM_BUILD/lib/${INSTALL_SUBDIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/ocl_cpu_backend/lib/Linux64
                )
endif(WIN32)

if (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
	link_directories( ${TBB_LIB_DIR}	)
endif (WIN32)

if (WIN32)
     add_definitions( -D_WINDOWS )
else ()
    add_definitions( -D_LINUX )
endif (WIN32)

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if (NOT WIN32)
	get_target_property( CURR_RPATH ${TARGET_NAME} INSTALL_RPATH )
	set_target_properties( ${TARGET_NAME} PROPERTIES INSTALL_RPATH "${CURR_RPATH};${COI_SDK_PATH}/host-linux-${DEBUG_RELEASE}/lib" )
endif (NOT WIN32)


add_dependencies( ${TARGET_NAME} OclCpuBackEnd cl_sys_utils)

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

if (NOT WIN32)
    if (NOT MIC_TESTING_MODE)
        SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} MicDefice_linux_exports.txt )
    endif (NOT MIC_TESTING_MODE)
endif (NOT WIN32)

add_dependencies( ${TARGET_NAME} LLVM_copy_tools )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    # Visual Studio 2010
    # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
    # The /DEBUG flag is required in order to create stripped pdbs.	
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} ${COI_HOST_LIB}.lib /DELAYLOAD:${COI_HOST_LIB}.dll /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCPRTD /NODEFAULTLIB:MSVCRTD /PDBSTRIPPED:${PDB_NAME}")
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} ${COI_HOST_LIB}.lib /DELAYLOAD:${COI_HOST_LIB}.dll /DEBUG /PDBSTRIPPED:${PDB_NAME}")
	
	install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
    
    if (INSTALL_PDBS)    
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
	
else (WIN32)

	list(APPEND LINK_LIBS ${COI_HOST_LIB} )
    
endif(WIN32)

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      ${LINK_LIBS} )

add_subdirectory( mic_native )

add_subdirectory( tests )

