#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cpu_device${BUILD_PLATFORM}${EXPERIMENTAL_POSTFIX} )

set( CMAKE_CXX_FLAGS_DEBUG "-DOCLDEVICE_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
if (NOT WIN32)
     # disable warnings inherited from external componennts
     add_definitions( "-Wno-unused-function" )
     add_definitions( "-Wno-attributes" )
endif(NOT WIN32)

if (NOT WIN32)
    set(HW_UTILS_HEADERS ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

include_directories( BEFORE ${OCL_SOURCE_DIR}/backend/ocl_cpu_backend/export
                            ${OCL_SOURCE_DIR}/clang_compiler
                            ${OCL_SOURCE_DIR}/cl_api
                            ${HW_UTILS_HEADERS} 
                            ${OCL_BINARY_DIR}
                            ${CL_API_HEADERS}
                            ../common/builtin_kernels
                    )

if(WIN32)
    include_directories( AFTER
        ${OCL_SOURCE_DIR}/externals/gpa/include
        )
endif(WIN32)

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )

if (WIN32)
    set( TBB_LIB_DIR  )
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories(   ${TBB_LIB_DIR}
                    ${OCL_SOURCE_DIR}/externals/gpa/libs/${TAL_LIB_DIR_SUFFIX}
                )

if (WIN32)
    add_definitions( -D_WINDOWS )
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
else()
    add_definitions( -D_LINUX )
endif (WIN32)

add_definitions( -D__TBB_NO_IMPLICIT_LINKAGE=1 )

#Support for built-in kernels
list (APPEND LINK_LIBS  builtin_kernels device_commands)

set(MKL_LINK_LIBS )
# Add MKL library integration
if (INCLUDE_MKL)
    list (APPEND MKL_LINK_LIBS mkl_kernels)
    include_directories( BEFORE ../common/mkl_kernels/export )
    if( WIN32 )
        link_directories( BEFORE ${MKL_DIR}/lib/${ICC_ARCH_DIR}/ )
        list(APPEND MKL_LINK_LIBS mkl_intel_ilp64 mkl_intel_thread mkl_core)
    else()
        # Next to lines uses dynamic libraries from MKL. This to be resolved against linkage with static libraries
        #link_directories( BEFORE ${MKL_DIR}/lib/${ICC_ARCH_DIR}/ )
        #list(APPEND LINK_LIBS mkl_intel_ilp64 mkl_intel_thread mkl_core)
        list(APPEND MKL_LINK_LIBS "-Wl,--start-group ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_intel_ilp64.a ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_core.a ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_intel_thread.a -Wl,--end-group")
        list(APPEND MKL_LINK_LIBS dl pthread m)
    endif(WIN32)
    
    if(USE_OMP2TBB)
        list(APPEND MKL_LINK_LIBS omp2tbb)
    else(USE_OMP2TBB)
        if ( WIN32 )
            list(APPEND MKL_LINK_LIBS ${ICC_DIR}/compiler/lib/${ICC_ARCH_DIR}/libiomp5md.lib)
        else()
            list(APPEND MKL_LINK_LIBS ${ICC_DIR}/compiler/lib/${ICC_ARCH_DIR}/libiomp5.so)
        endif(WIN32)
    endif(USE_OMP2TBB)    
endif(INCLUDE_MKL)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

#include ITT notify sources
include( ../../cmake_utils/itt.cmake )
                
#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if(WIN32)
    list(APPEND LINK_LIBS delayimp.lib)
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cpu_device_linux_exports.txt )
endif (WIN32)

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger${EXPERIMENTAL_POSTFIX}
                                      device_commands
                                      task_executor${BUILD_PLATFORM}${EXPERIMENTAL_POSTFIX}
                                      ${LINK_LIBS})
if (INCLUDE_MKL)
    target_link_libraries( ${TARGET_NAME} LINK_PRIVATE ${MKL_LINK_LIBS})
endif(INCLUDE_MKL)

install( TARGETS ${TARGET_NAME}     
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin)

#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008     
        set_target_properties( ${TARGET_NAME} PROPERTIES
            LINK_FLAGS_RELEASE    "${LINK_FLAGS_RELEASE} /DELAYLOAD:tbbmalloc.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll"
            LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /DELAYLOAD:tbbmalloc_debug.dll /NODEFAULTLIB:MSVCRT /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll"
        )
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS    "${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")
    else(${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.    
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll /PDBSTRIPPED:${PDB_NAME}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE    "${LINK_FLAGS_RELEASE} /DELAYLOAD:tbbmalloc.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
    install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
    
    if (INSTALL_PDBS)    
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
else(WIN32)
    if ( INCLUDE_MKL )
        #need explicitly set the full path to prevent failure in cpu_device_type compilation
        set_property( TARGET ${TARGET_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-z,defs" )
    endif( INCLUDE_MKL )
endif(WIN32)

