set(TARGET_NAME OclCpuDebugging)

project(${TARGET_NAME})

cmake_minimum_required(VERSION 2.8.4)

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
set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter")

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
set( INIT_LINKER_FLAGS "${INIT_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -l${CMAKE_DL_LIBS} -lrt" ) 

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS           ${INIT_LINKER_FLAGS})
set( CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

# Linker switches - Shared Lib
set( CMAKE_SHARED_LINKER_FLAGS          ${INIT_LINKER_FLAGS})
set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

# Target directory
set(OCL_CPU_DEBUGGING_LIB_DIR ${OCL_CPU_DEBUGGING_DIR}/lib/Linux${PLATFORM})

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
if (WIN32)


# Target directory
set(OCL_CPU_DEBUGGING_LIB_DIR ${OCL_CPU_DEBUGGING_DIR}/lib/Win${PLATFORM})

# Linker flags
set(CMAKE_SHARED_LINKER_FLAGS  /NODEFAULTLIB:LIBCMTD)

endif (WIN32)


# Create target project

set (OCL_CPU_DEBUGGING_INCLUDE_FILES
    export/icldebuggingservice.h
    debug_communicator.h
    debug_server.h
    debuginfo_utils.h
    debugservermessages.pb.h
    protobufpackedmessage.h
)

set (OCL_CPU_DEBUGGING_SOURCE_FILES
    dllmain.cpp
    debug_communicator.cpp
    debug_server.cpp
    debuginfo_utils.cpp
    debugservermessages.pb.cc
    protobufpackedmessage.cpp
)


# Add additional lib direcories
link_directories (
    ${OCL_CPU_DEBUGGING_LIB_DIR}
    ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
) 

include_directories(
  ${OCL_CPU_DEBUGGING_IMPORT_DIRS}
  ${OCL_SOURCE_DIR}/externals/tbb/include
)


# Create DLL
add_library (${TARGET_NAME} SHARED 
    ${OCL_CPU_DEBUGGING_INCLUDE_FILES}
    ${OCL_CPU_DEBUGGING_SOURCE_FILES}
)

#For OCL SDK - Shared libraries should be in the same location as executables
if ((NOT WIN32) AND (DEFINED RUNTIME_OUTPUT_DIRECTORY))
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${TARGET_NAME} PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${RT_OUTPUT_DIRECTORY})
endif((NOT WIN32) AND (DEFINED RUNTIME_OUTPUT_DIRECTORY))

set (LLVM_LIBS    
    # LLVM libs
    LLVMCore
    LLVMSystem
    LLVMSupport
    LLVMAnalysis
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
    
    # LLVM LIBS
    ${LLVM_LIBS}
    ) 

if (WIN32)
    target_link_libraries( ${TARGET_NAME} protobuf )
else (WIN32)
    target_link_libraries( ${TARGET_NAME} protobuf pthread )
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} ${TARGET_NAME}_linux_exports.txt)
endif (WIN32)
set_target_properties( protobuf PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/ocl_backend )


add_dependencies(${TARGET_NAME}     
    ${LLVM_LIBS}
    ) 
    
install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
