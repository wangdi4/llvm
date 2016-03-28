cmake_minimum_required(VERSION 2.8.7)
use_rtti(TRUE)

if(WIN32)
    # This change was added in order to support the 15.33 GFX driver
    # CPU runtime (version 3.0) installation with the new SDK
    # TODO - undo this when OclCpuDebugging.dll is added to the driver
    set(TARGET_NAME OclCpuDebugging3)
else(WIN32)
    set(TARGET_NAME OclCpuDebugging)
endif(WIN32)

project(${TARGET_NAME})

add_subdirectory( protobuf )

if( CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bit
    set (PLATFORM 32)
else( CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bit
    set (PLATFORM 64)
endif( CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_CPU_DEBUGGING_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(OCL_CPU_DEBUGGING_IMPORT_DIRS 
  ${BACKEND_ROOT_DIR}/../cl_api
  ${OCL_CPU_DEBUGGING_DIR}/protobuf/include
  ${BACKEND_ROOT_DIR}/../utils/cl_hw_utils/export
  ${BACKEND_ROOT_DIR}/../utils/cl_sys_utils/export
)
set(OCL_CPU_DEBUGGING_EXPORT_DIR  ${OCL_CPU_DEBUGGING_DIR}/export)

#
# Settings for Linux compilation
#
if (NOT WIN32)

    # configure immediate dependencies search path at run time for non-standard libs
    SET(CMAKE_SKIP_BUILD_RPATH                  TRUE)   # do not add pointers to the build tree
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE)   # build rpath as if already installed
    SET(CMAKE_INSTALL_RPATH                     "$ORIGIN") # the rpath to use - search through installation dir only
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath

    # Warning level
    set (WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter -Wno-unused-function")

    # Compiler switches that CANNOT be modified during makefile generation
    set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

    set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
    set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")

    set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
    set (ADD_C_FLAGS_RELEASE "-O2 -g0 ")

    # Compiler switches that CAN be modified during makefile generation and configuration-independent
    add_definitions( ${WARNING_LEVEL} )

    # Linker switches
    set (INIT_LINKER_FLAGS        "-pie -Wl,--enable-new-dtags") # --enable-new-dtags sets RUNPATH to the same value as RPATH
    set (ADD_LINKER_FLAGS_DEBUG   )
    set (ADD_LINKER_FLAGS_RELEASE )

    include(FindThreads)
    set (INIT_LINKER_FLAGS "${INIT_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -l${CMAKE_DL_LIBS} ${RT_LIB}")

    # C switches
    set (CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
    set (CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

    # C++ switches
    set (CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
    set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

    # Linker switches - EXE
    set (CMAKE_EXE_LINKER_FLAGS            ${INIT_LINKER_FLAGS})
    set (CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

    # Linker switches - Shared Lib
    set (CMAKE_SHARED_LINKER_FLAGS          ${INIT_LINKER_FLAGS})
    set (CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
    set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

    # embed RPATH and RUNPATH to the binaries that assumes that everything is installed in the same directory
    #
    # Description:
    #   RPATH is used to locate dynamically load shared libraries/objects (DLLs) for the non-standard OS
    #   locations without need of relinking DLLs during installation. The algorithm is the following:
    #
    #     1. If RPATH is present in the EXE/DLL and RUNPATH is NOT present, search through it.
    #     2. If LD_LIBRARY_PATH env variable is present, search through it
    #     3. If RUNPATH is present in the EXE/DLL, search through it
    #     4. Search through locations, configured by system admin and cached in /etc/ld.so.cache
    #     5. Search through /lib and /usr/lib
    #
    #   RUNPATH influences only the immediate dependencies, while RPATH influences the whole subtree of dependencies
    #   RPATH is concidered deprecated in favor of RUNPATH, but RUNPATH does not supported by some Linux systems.
    #   If RUNPATH is not supported, system loader may report error - remove "-Wl,--enable-new-dtags" above to
    #   disable RUNPATH generation.
    #
    #   If RPATH or RUNPATH contains string $ORIGIN it is substituted by the full path to the containing EXE/DLL.
    #   Security issue 1: if EXE/DLL is marked as set-uid or set-gid, $ORIGIN is ignored.
    #   Security issue 2: if RPATH/RUNPATH references relative subdirs, intruder may fool it by using softlinks
    #
    SET(CMAKE_SKIP_BUILD_RPATH                  TRUE)   # do not add pointers to the build tree
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE)   # build rpath as if already installed
    SET(CMAKE_INSTALL_RPATH                     "$ORIGIN") # the rpath to use - search through installation dir only
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath
endif (NOT WIN32)

#
# Settings for Windows compilation
#


# Create target project

set (OCL_CPU_DEBUGGING_INCLUDE_FILES
    export/icldebuggingservice.h
    debug_communicator.h
    debug_server.h
    debuginfo_utils.h
    debugservermessages.pb.h
    protobufpackedmessage.h
    resource.h
)

set (OCL_CPU_DEBUGGING_SOURCE_FILES
    dllmain.cpp
    debug_communicator.cpp
    debug_server.cpp
    debuginfo_utils.cpp
    debugservermessages.pb.cc
    protobufpackedmessage.cpp
)

set (OCL_CPU_DEBUGGING_RC_FILES
    OclCpuDebugging.rc
)


include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${OCL_CPU_DEBUGGING_IMPORT_DIRS}
  ${OCL_SOURCE_DIR}/externals/tbb/include
)

if (WIN32)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2008/${OCL_SYS_DEPENDENT_SUBDIR} )
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
    endif (${MSVC_VERSION} EQUAL 1500)
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

# Add additional lib direcories
link_directories (
    ${TBB_LIB_DIR}
)

# Create DLL
add_library (${TARGET_NAME} SHARED 
    ${OCL_CPU_DEBUGGING_INCLUDE_FILES}
    ${OCL_CPU_DEBUGGING_SOURCE_FILES}
    ${OCL_CPU_DEBUGGING_RC_FILES}
)

# Link with appropriate libs
if (WIN32)
    set( SOCKETLIB ws2_32 )
else (WIN32)
    set( SOCKETLIB "" )
endif (WIN32)

target_link_libraries (${TARGET_NAME}
    ${SOCKETLIB}
    cl_sys_utils
    protobuf
    ${LLVM_MODULE_LIBS}
)

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} ${TARGET_NAME}_linux_exports.txt)
endif (NOT WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

# Install the (GPL'd) GDB OCL plugin. The name and location is important, as
# GDB will auto-load it if it has the same name as a loaded shared object.
set (GDB_PLUGIN libintelocl.so-gdb.py)
install( FILES ${GDB_PLUGIN} DESTINATION bin)

# This change was added in order to support the 15.33 GFX driver
# CPU runtime (version 3.0) installation with the new SDK
# TODO - undo this when OclCpuDebugging.dll is added to the driver
if (WIN32)
    if( CMAKE_SIZEOF_VOID_P EQUAL 4)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/compatibility/1533_gfx_compatible/x86/OclCpuDebugging.dll DESTINATION bin)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/compatibility/llvm34/x86/OclCpuDebugging2.dll DESTINATION bin)
    else( CMAKE_SIZEOF_VOID_P EQUAL 4)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/compatibility/1533_gfx_compatible/x64/OclCpuDebugging.dll DESTINATION bin)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/compatibility/llvm34/x64/OclCpuDebugging2.dll DESTINATION bin)
    endif( CMAKE_SIZEOF_VOID_P EQUAL 4)
endif (WIN32)
