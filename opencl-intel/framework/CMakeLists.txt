if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

if (NOT WIN32)
  # disable warnings inherited from external componennts
  add_definitions(-Wno-missing-braces)
endif (NOT WIN32)


# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard system libarary directories
include(embed.rpath)

# this component has to use deprecated OpenCL API functions.
if (WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif (WIN32)

include_directories ( BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/export
        ${CL_API_HEADERS}
        ${LLVM_MAIN_SRC_DIR}/../external/opencl-icd-loader/loader
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/Context
        ${CMAKE_CURRENT_SOURCE_DIR}/Execution
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryAllocator
        ${CMAKE_CURRENT_SOURCE_DIR}/Tracing
        ${OCL_BINARY_DIR}
    )

# For debug only
if (USE_ITT_INTERNAL)
	add_definitions (-DUSE_ITT_INTERNAL)
endif()

# these sub-directories must be added here in order to recieve the ITT definitions
add_subdirectory( Platform )
add_subdirectory( Context )
add_subdirectory( Execution )
add_subdirectory( MemoryAllocator )
add_subdirectory( Tracing )

# set that name of the main output file as a target name
set( TARGET_NAME intelocl${BUILD_PLATFORM} )
set( TARGET_NAME_EMU ${TARGET_NAME}_${OUTPUT_EMU_SUFF} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

if (WIN32)
    source_group( "API" FILES cl_framework.cpp cl_framework.h framework_proxy.cpp framework_proxy.h)

    list( APPEND LINK_LIBS PowrProf.lib )
endif (WIN32)

if (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)
    set( IMPORT_LIB ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX} )
    set( IMPORT_LIB ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME_EMU}${CMAKE_IMPORT_LIBRARY_SUFFIX} )
endif (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)

link_directories( ${TBB_LIB_DIR} )

include(itt)

set(FRAMEWORK_LINK_LIBS
    Context
    Execution
    Platform
    MemoryAllocator
    Tracing
    cl_sys_utils
    CLElfLib
)

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} SHARED
    ${TARGET_SOURCES}

    COMPONENTS
      task_executor${BUILD_PLATFORM}
      cl_logger

    LINK_LIBS
      ${FRAMEWORK_LINK_LIBS}
      ${LINK_LIBS}

    RC_TEMPLATE DEFAULT

    INSTALL_PATH
      ${OUTPUT_ARCH_SUFF}
)

add_opencl_library (${TARGET_NAME_EMU} SHARED
    ${TARGET_SOURCES}

    COMPONENTS
      task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}
      cl_logger_${OUTPUT_EMU_SUFF}

    LINK_LIBS
      ${FRAMEWORK_LINK_LIBS}
      ${LINK_LIBS}

    RC_TEMPLATE DEFAULT

    INSTALL_PATH
      ${OUTPUT_EMU_SUFF}
)

# Generate config file for FPGA emulator and cpu
# For cpu
set(CL_CONFIG_DEVICES cpu)
configure_file(${OCL_SOURCE_DIR}/cmake/cl_config.in ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF}/cl.cfg)
# For fpga-emu
set(CL_CONFIG_DEVICES fpga-emu)
configure_file(${OCL_SOURCE_DIR}/cmake/cl_config.in ${OCL_BINARY_DIR}/lib/${OUTPUT_EMU_SUFF}/cl.cfg)

# For fpga-emu
if (NOT (WIN32 AND "${OUTPUT_ARCH_SUFF}" STREQUAL "x86"))
    install (CODE
        "
        file(COPY ${OCL_BINARY_DIR}/lib/${OUTPUT_EMU_SUFF}/cl.cfg
             DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${OUTPUT_EMU_SUFF})
        " COMPONENT ocl-config)
endif()
# For CPU
install (CODE
    "
        file(COPY ${OCL_BINARY_DIR}/lib/${OUTPUT_ARCH_SUFF}/cl.cfg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${OUTPUT_ARCH_SUFF})
    " COMPONENT ocl-config)

install (CODE
    "
    if (NOT WIN32)
    execute_process (COMMAND dos2unix ${CMAKE_INSTALL_PREFIX}/lib/${OUTPUT_ARCH_SUFF}/cl.cfg)
    execute_process (COMMAND dos2unix ${CMAKE_INSTALL_PREFIX}/lib/${OUTPUT_EMU_SUFF}/cl.cfg)
    endif(NOT WIN32)
    " COMPONENT ocl-config)

if (WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:task_executor${BUILD_PLATFORM}.dll")
    get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME} LINK_FLAGS_DEBUG)
    get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME} LINK_FLAGS_RELEASE)
    set_target_properties (${TARGET_NAME} PROPERTIES
        LINK_FLAGS_DEBUG  "${LK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
        LINK_FLAGS_RELEASE "${LK_FLAGS_RELEASE} tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll")
    set_target_properties(${TARGET_NAME_EMU} PROPERTIES LINK_FLAGS "/DELAYLOAD:task_executor${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}.dll")
    get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME_EMU} LINK_FLAGS_DEBUG)
    get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME_EMU} LINK_FLAGS_RELEASE)
    set_target_properties (${TARGET_NAME_EMU} PROPERTIES
        LINK_FLAGS_DEBUG  "${LK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
        LINK_FLAGS_RELEASE "${LK_FLAGS_RELEASE} tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll")

    list( APPEND LINK_LIBS delayimp.lib )
endif (WIN32)

SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cl_framework_linux_exports.txt )
SET_LINUX_EXPORTS_FILE( ${TARGET_NAME_EMU} cl_framework_linux_exports.txt )

set_target_properties( ${TARGET_NAME}  PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
set_target_properties( ${TARGET_NAME_EMU}  PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
# deploy icd library
get_filename_component(ICD_LOADER_OUTPUT_DIR ${OpenCL_LIBRARY} DIRECTORY)
file(GLOB ICD_LOADER
    "${ICD_LOADER_OUTPUT_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}OpenCL*")
if (INTEL_OPENCL_STANDALONE_PRODUCT)
  # For standalone OpenCL product we must deploy ICD loader ourselves.
  # FIXME: Cmake doesn't allow to install IMPORTED libraries before 3.13 version.
  # So, we install ICD loader library files as ocl-OpenCL component.
  install_to(${ICD_LOADER}
      DESTINATION lib/${OUTPUT_ARCH_SUFF}
      COMPONENT ocl-OpenCL
  )
endif()
# install to build directory just for convenience.
install_to(${ICD_LOADER}
           DESTINATION ${OCL_OUTPUT_LIBRARY_DIR}
           COMPONENT ocl-OpenCL
)
