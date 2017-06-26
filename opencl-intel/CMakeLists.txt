#
# Global setup file for OpenCL CMake
#
cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

if( BUILD_EXPERIMENTAL_21 )
    set( EXPERIMENTAL_POSTFIX "_2_1" )
    add_definitions( -DBUILD_EXPERIMENTAL_21 )
    add_definitions( -DEXPERIMENTAL_POSTFIX="_2_1" )
endif( BUILD_EXPERIMENTAL_21 )

if( BUILD_FPGA_EMULATOR )
    add_definitions( -DBUILD_FPGA_EMULATOR  )
endif( BUILD_FPGA_EMULATOR )
# CMAKE variables
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/cmake/Modules/"
    "${CMAKE_SOURCE_DIR}/cmake_utils/"
)
include( cmake_utils/CMakeFuncs.txt )

if (WIN32)
    include( CMake_windows_tools )
elseif( ANDROID)
    if ( BUILD_X64)
        include( CMake_android_tools64 )
    else ()
       include( CMake_android_tools )
    endif ()
else()
  include( CMake_linux_tools )
endif ()

# Supported configurations
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
  )
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Available build configurations." FORCE)


# ITT/VTune integration
if( NOT ANDROID )
    add_definitions( -DUSE_ITT )
    include_directories( ${CMAKE_SOURCE_DIR}/externals/itt/include
       ${CMAKE_SOURCE_DIR}/externals/itt/ittnotify/ )
endif( NOT ANDROID )

if (NOT WIN32)
  # enable C++11 support in GCC 4.7 and above
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  add_custom_target(strip
    ${CMAKE_SOURCE_DIR}/cmake_utils/separate_linux_symbols.sh
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
    COMMENT "Separating debug symbols from binaries...")
endif (NOT WIN32)

if (WIN32)
    # Add definitions that make MSVC much less annoying.
    add_definitions(
        # For some reason MS wants to deprecate a bunch of standard functions...
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_WARNINGS
        -D_SCL_SECURE_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS

        -wd4141 # Suppress ''modifier' : used more than once' (because of __forceinline combined with inline)
        -wd4065 # Suppress 'switch statement contains 'default' but no 'case' labels'
        -wd4090 # Suppress 'function : different 'const' qualifiers'
        -wd4146 # Suppress 'unary minus operator applied to unsigned type, result still unsigned'
        -wd4180 # Suppress 'qualifier applied to function type has no meaning; ignored'
        -wd4190 # Suppress 'function has C linkage is specified, but returns type which is incompatible with C'
        -wd4224 # Suppress 'nonstandard extension used : formal parameter 'identifier' was previously defined as a type'
        -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2', possible loss of data'
        -wd4267 # Suppress ''var' : conversion from 'size_t' to 'type', possible loss of data'
        -wd4275 # Suppress 'An exported class was derived from a class that was not exported.'
        -wd4291 # Suppress ''declaration' : no matching operator delete found; memory will not be freed if initialization throws an exception'
        -wd4345 # Suppress 'behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized'
        -wd4351 # Suppress 'new behavior: elements of array 'array' will be default initialized'
        -wd4355 # Suppress ''this' : used in base member initializer list'
        -wd4503 # Suppress ''identifier' : decorated name length exceeded, name was truncated'
        -wd4551 # Suppress 'function call missing argument list'
        -wd4624 # Suppress ''derived class' : destructor could not be generated because a base class destructor is inaccessible'
        -wd4715 # Suppress ''function' : not all control paths return a value'
        -wd4800 # Suppress ''type' : forcing value to bool 'true' or 'false' (performance warning)'
        -wd4065 # Suppress 'switch statement contains 'default' but no 'case' labels'
        -wd4181 # Suppress 'qualifier applied to reference type; ignored'
        -w14062 # Promote "enumerator in switch of enum is not handled" to level 1 warning.
        -wd4985 # Supress 'MSVS9 Math.h "ceil": attributes not present on previous declaration'
    )

    #C flags
    set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /MTd")
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")

    # C++ flags
    set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MTd")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif (WIN32)

if(CMAKE_CROSSCOMPILING)
    set(CX_NATIVE_TG_DIR "${CMAKE_BINARY_DIR}/native")

    add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CX_NATIVE_TG_DIR}
                            COMMENT "Creating ${CX_NATIVE_TG_DIR}..."
    )

    add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}/CMakeCache.txt
                            COMMAND ${CMAKE_COMMAND} -UCMAKE_TOOLCHAIN_FILE
                            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                            # We are cross compiling when building for android, hence we are
                            # dropping off all non required build components, such tests/verification
                            -DBACKEND_BUILD_VERIFICATION_LIB=OFF
                            -DLLVM_PATH_BE=${LLVM_PATH_BE}/native
                            -DLLVM_PATH_FE=${LLVM_PATH_FE}/native
                            -DBUILD_X64=True
                            ${CMAKE_SOURCE_DIR}
                            WORKING_DIRECTORY ${CX_NATIVE_TG_DIR}
                            DEPENDS ${CX_NATIVE_TG_DIR}
                            COMMENT "Configuring native targets..."
    )

    # Build 'native' tools required to build target binaries:
    # ocloptimg - required to build OpenCL built-in library
    # CMake can build one target at a time and we need only ocloptimg, so we
    # just use it. If you need to build several targets, you should create
    # custom target with dependencies from all the target you need to build.
    # This will allow you to build them in parallel!
    # NOTE: actually we cheat here, we also need table-gen, but ocloptimg has
    # it in its depedencies, so we just check that it's built with ocloptimg.
    add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}/bin/ocloptimg ${CX_NATIVE_TG_DIR}/bin/ocl-tblgen
                            COMMAND ${CMAKE_COMMAND}
                            --build ${CX_NATIVE_TG_DIR}
                            --target ocloptimg
                            --config ${CMAKE_BUILD_TYPE} -- -j
                            WORKING_DIRECTORY ${CX_NATIVE_TG_DIR}
                            DEPENDS ${CX_NATIVE_TG_DIR}/CMakeCache.txt
                            COMMENT "Building native targets..."
    )

    add_custom_target(ConfigureNativeTargets DEPENDS ${CX_NATIVE_TG_DIR}/bin/ocloptimg ${CX_NATIVE_TG_DIR}/bin/ocl-tblgen )
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CX_NATIVE_TG_DIR})
else(CMAKE_CROSSCOMPILING)
    add_custom_target(ConfigureNativeTargets DEPENDS ${CMAKE_BINARY_DIR}/bin/ocloptimg ${CMAKE_BINARY_DIR}/bin/ocl-tblgen )
endif(CMAKE_CROSSCOMPILING)

if(INCLUDE_TOOLS)
    if(WIN32)
        if(BUILD_X64)
            set(BOOST_LIB_DIR_SUFFIX x64)
        else(BUILD_X64)
            set(BOOST_LIB_DIR_SUFFIX x86)
        endif(BUILD_X64)
    else(WIN32)
        if(ANDROID)
             if(BUILD_X64)
                 set(BOOST_LIB_DIR_SUFFIX android64)
             else(BUILD_X64)
                 set(BOOST_LIB_DIR_SUFFIX android32)
             endif(BUILD_X64)
        else(ANDROID)
            set(BOOST_LIB_DIR_SUFFIX x64)
        endif(ANDROID)
    endif(WIN32)

    set (BOOST_LIBRARYDIR ${BOOST_ROOT}/lib/${BOOST_LIB_DIR_SUFFIX})
    set (Boost_NO_SYSTEM_PATHS ON) # Don't search PATH

    # To add new components for Boost simply add to BOOST_COMPONENTS below
    set (BOOST_COMPONENTS regex system filesystem date_time signals thread)

    if(NOT ANDROID)
        set(Boost_USE_STATIC_LIBS ON )
        set(BOOST_ALL_DYN_LINK OFF)
        set(Boost_USE_STATIC_RUNTIME ON)
        FIND_PACKAGE(Boost 1.52.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
    else(NOT ANDROID)
        set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set (CMAKE_FIND_LIBRARY_PREFIXES "lib")
        set (BOOST_LIBRARYDIR ${BOOST_ROOT}/lib/${BOOST_LIB_DIR_SUFFIX})
        find_host_package(Boost 1.52.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
        set(Boost_USE_STATIC_RUNTIME ON)
        set(BOOST_ALL_DYN_LINK OFF)
    endif(NOT ANDROID)
    add_definitions(-DBOOST_ALL_NO_LIB)  # Tell the compiler to undefine this boost macro
    add_definitions( -DBOOST_SYSTEM_NO_DEPRECATED )
    set(BOOST_INSTALL_LIBS ${Boost_SYSTEM_LIBRARY}
                           ${Boost_FILESYSTEM_LIBRARY}
                           ${Boost_DATE_TIME_LIBRARY}
                           ${Boost_REGEX_LIBRARY}
                           ${Boost_SIGNALS_LIBRARY}
                           ${Boost_THREAD_LIBRARY}
                           )
    list(REMOVE_ITEM BOOST_INSTALL_LIBS debug optimized)

    if(WIN32)
        # craete boost liberaroes for dynamic compilation ( used by KernelBuilder
        foreach(boost_comp ${BOOST_COMPONENTS})
            string(TOUPPER ${boost_comp} boost_comp )
            list( GET Boost_${boost_comp}_LIBRARY 1 REL_LIB )
            string( REPLACE "libboost" "boost" REL_LIB ${REL_LIB} )
            string( REPLACE "-s-" "-" REL_LIB ${REL_LIB} )
            list( GET Boost_${boost_comp}_LIBRARY 3 DEBUG_LIB )
            string( REPLACE "libboost" "boost" DEBUG_LIB ${DEBUG_LIB} )
            string( REPLACE "-sgd" "-gd" DEBUG_LIB ${DEBUG_LIB} )
            set( dyn_Boost_${boost_comp}_LIBRARY optimized ${REL_LIB} debug ${DEBUG_LIB} )

            string( REPLACE ".lib" ".dll" REL_LIB ${REL_LIB} )
            string( REPLACE ".lib" ".dll" DEBUG_LIB ${DEBUG_LIB} )

            list( APPEND BOOST_INSTALL_LIBS_RELEASE ${REL_LIB} )
            list( APPEND BOOST_INSTALL_LIBS_DEBUG ${DEBUG_LIB} )
        endforeach(boost_comp)

        INSTALL(FILES ${BOOST_INSTALL_LIBS_RELEASE} DESTINATION bin CONFIGURATIONS Release)
        INSTALL(FILES ${BOOST_INSTALL_LIBS_DEBUG} DESTINATION bin CONFIGURATIONS Debug)

    else(WIN32)
        if(NOT ANDROID)

            string(REPLACE ".a" ".so;" BOOST_INSTALL_LIBS ${BOOST_INSTALL_LIBS} )
            SET(BOOST_VER ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
            string(REPLACE ".so" ".so.${BOOST_VER};" BOOST_INSTALL_LIBS_LINKS ${BOOST_INSTALL_LIBS} )

            INSTALL(FILES ${BOOST_INSTALL_LIBS} DESTINATION bin)
            INSTALL(FILES ${BOOST_INSTALL_LIBS_LINKS} DESTINATION bin)
        endif(NOT ANDROID)

    endif(WIN32)
endif(INCLUDE_TOOLS)

# Project-wise options
option(GIT_REVISIONS "Make package version from git revision"  ON)
option(BUILD_QTGUI "Build the QT related projects"  OFF)
option(INCLUDE_MIC_DEVICE "Include MIC device Into Build"  OFF)
option(USE_ITT_INTERNAL "Report internal tasks start/stop to VTune" OFF)
option(USE_HARD_TRAPPING "Use hard trapping for workers on CPU device"  ON)
option(USE_COMMON_CLANG "Use VPG supplied common clang dll instead of internale implementation" OFF)
option(INCLUDE_CMRT "Include Common Runtime into Build"  OFF)
option(BUILD_LLVM_FROM_SOURCE "Build the llvm package from source instead of using pre-installed binaries" OFF)

# find the used external packages

# QTGUI
if (BUILD_QTGUI AND NOT ANDROID)
    set( QT_VERSION "4.8.5" )
    if(DEFINED QT_REVISION) # QT_REVISION is a variable that can be defined in Build.py. It is not defined by default.
       set( QT_VERSION ${QT_REVISION})
    endif()

    #install the QT and boost libraries themselfs
    if(WIN32)
        # special processing for QT library on windows:
        # the problem is that QT doesn't naturally supports installing
        # both 32 and 64 bits libraries on the same machine (there is
        # only a single registry settings for QT library per machine )
        # this makes it difficult to rely on default QT Cmake support
        # to find the appropriate version.
        # The trick is to set the correct installation directory on the fly
        # by changing the registry information and removing the default paths
        remove_path("c:\\Qt\\4.8.1\\VS2010-64\\bin")
        remove_path("c:\\Qt\\4.8.1\\VS2010\\bin")
        execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions /v DefaultQtVersion /d ${QT_VERSION} /f)
        if(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2012-64 /f)
        else(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2012 /f)
        endif(BUILD_X64)
    else(WIN32)
        add_path("/usr/local/DigiaQtCommercialSDK/Desktop/${QT_VERSION}/gcc-64/bin")
    endif(WIN32)

    find_package( Qt4 REQUIRED)

    if(WIN32)
        SET( SHARED_DEBUG_LIBS
            ${QT_BINARY_DIR}/QtGuid4.dll
            ${QT_BINARY_DIR}/QtCored4.dll
            ${QT_BINARY_DIR}/Qt3Supportd4.dll
            ${QT_BINARY_DIR}/QtNetworkd4.dll
            ${QT_BINARY_DIR}/QtXmld4.dll
            ${QT_BINARY_DIR}/QtSqld4.dll
        )
        SET( SHARED_RELEASE_LIBS
            ${QT_BINARY_DIR}/QtGui4.dll
            ${QT_BINARY_DIR}/QtCore4.dll
            ${QT_BINARY_DIR}/Qt3Support4.dll
            ${QT_BINARY_DIR}/QtNetwork4.dll
            ${QT_BINARY_DIR}/QtXml4.dll
            ${QT_BINARY_DIR}/QtSql4.dll
        )
        SET( SHARED_DEBUG_LIBS ${SHARED_DEBUG_LIBS} ${QT_BINARY_DIR}/QtCommercialChartd.dll)
        SET( SHARED_RELEASE_LIBS ${SHARED_RELEASE_LIBS} ${QT_BINARY_DIR}/QtCommercialChart.dll)
        INSTALL(FILES ${SHARED_DEBUG_LIBS} DESTINATION bin CONFIGURATIONS Debug)
        INSTALL(FILES ${SHARED_RELEASE_LIBS} DESTINATION bin CONFIGURATIONS Release)
    else(WIN32)
        SET(QT_VERSION ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} )
        SET( SHARED_LIBS
            ${QT_LIBRARY_DIR}/libQtCore.so.4
            ${QT_LIBRARY_DIR}/libQtGui.so.4
            ${QT_LIBRARY_DIR}/libQtCore.so.${QT_VERSION}
            ${QT_LIBRARY_DIR}/libQtGui.so.${QT_VERSION}
        )
        SET( SQUISH_LIBS
            ${QT_LIBRARY_DIR}/libQt3Support.so.4
            ${QT_LIBRARY_DIR}/libQtNetwork.so.4
            ${QT_LIBRARY_DIR}/libQtSql.so.4
            ${QT_LIBRARY_DIR}/libQtXml.so.4
            ${QT_LIBRARY_DIR}/libQt3Support.so.${QT_VERSION}
            ${QT_LIBRARY_DIR}/libQtNetwork.so.${QT_VERSION}
            ${QT_LIBRARY_DIR}/libQtSql.so.${QT_VERSION}
            ${QT_LIBRARY_DIR}/libQtXml.so.${QT_VERSION}
        )
        INSTALL(FILES ${SHARED_LIBS} DESTINATION bin)
        INSTALL(FILES ${SQUISH_LIBS} DESTINATION bin)
    endif(WIN32)
endif ()

#
# Project defs
#
project(OCL C CXX)

FIND_PACKAGE ( Threads REQUIRED )
if(NOT CMAKE_THREAD_LIBS_INIT)
  if(PTHREAD_LIB STREQUAL "")
    set(PTHREAD_LIB "$ENV{PTHREAD_LIB}")
  endif()
  if(PTHREAD_LIB STREQUAL "")
    message(FATAL_ERROR "No pthread library found!\nRun the cmake with option -D PTHREAD_LIB=/path/to/library or set the enviroment variable PTHREAD_LIB")
  endif()
else(NOT CMAKE_THREAD_LIBS_INIT)
  set(PTHREAD_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()

message(STATUS "Pthread library: ${PTHREAD_LIB}")
message(STATUS "Pthread include: ${PTHREAD_INC}")

set( REVISION_ID $ENV{REVISION_ID} )
set( GIT_VERSION $ENV{REVISION} )
set( GIT_REVISION $ENV{RELEASE_BRANCH} )

if(NOT REVISION_ID)
  message(STATUS "Build type: Development")
  set(BUILDVER_WC_REVISION "0")
  set( GIT_VERSION "DEV" )
  set( GIT_REVISION "development" )
elseif ( REVISION_ID MATCHES "\\.[0-9]*" ) # Revision ID
  message(STATUS "Build type: Release")
  STRING(REGEX MATCH ".([0-9]*)" REV_ID ${REVISION_ID} )
  set(BUILDVER_WC_REVISION ${CMAKE_MATCH_1})
  set( GIT_VERSION $ENV{REVISION} )
  set( GIT_REVISION $ENV{RELEASE_BRANCH} )
elseif ( REVISION_ID MATCHES "-r[0-9]*" ) # Review ID
  message(STATUS "Build type: Review")
  set( BUILDVER_WC_REVISION "0" )
  set( GIT_VERSION $ENV{REVISION} )
  set( GIT_REVISION $ENV{RELEASE_BRANCH} )
endif()

# MAJOR and MINOR version aligned with 15.40 branch because currently
# we are delivering cpu bundle from master.
# As soon as we will deliver cpu buncle from release branch to vpg
# revert this change. CSSD100021694.
set( PRODUCTVER_MAJOR 7 )
set( PRODUCTVER_MINOR 5 )

math(EXPR BUILDVER_16_BIT_MAJOR "${BUILDVER_WC_REVISION} / 65536")
math(EXPR BUILDVER_16_BIT_MINOR "${BUILDVER_WC_REVISION} % 65536")

message(STATUS "Current build version is ${BUILDVER_WC_REVISION}")
message(STATUS "GIT Version: ${GIT_VERSION}")
message(STATUS "GIT Revision: ${GIT_REVISION}")
message(STATUS "PRODUCTVER_MAJOR: ${PRODUCTVER_MAJOR}")

# --------------
#  write a file with the BUILDVERSION define
file(WRITE ${OCL_BINARY_DIR}/buildversion.h.txt
"#define BUILDVERSION ${BUILDVER_WC_REVISION}\n
#ifndef\ _DEBUG\n
    #define BUILDVERSIONSTR \"(Build\ ${BUILDVER_WC_REVISION})\"\n
#else\n
    #define BUILDVERSIONSTR \"(Build[DEBUG]\ ${BUILDVER_WC_REVISION})\"\n
#endif\n
#define PRODUCTVERSION_MAJOR ${PRODUCTVER_MAJOR}\n
#define PRODUCTVERSION_MINOR ${PRODUCTVER_MINOR}\n
#define BUILDVERSION16BIT_MAJOR ${BUILDVER_16_BIT_MAJOR}\n
#define BUILDVERSION16BIT_MINOR ${BUILDVER_16_BIT_MINOR}\n
#define VERSIONSTRING \"${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n
#define IOCVERSIONSTRING \"1.6.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n
#define KBVERSIONSTRING \"1.4.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n")
# --------------

if(WIN32)
    file(WRITE ${OCL_BINARY_DIR}/version.ini "[Version]\nPackVer=${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_WC_REVISION}\n")
    install(FILES ${OCL_BINARY_DIR}/version.ini DESTINATION ./)
endif()
file(WRITE ${OCL_BINARY_DIR}/version.txt "Version=${PRODUCTVER_MAJOR}.${PRODUCTVER_MINOR}.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\nInternalVersion=${GIT_REVISION}\n")
file(WRITE ${OCL_BINARY_DIR}/revision.txt "BUILD=${GIT_VERSION}\nREVISION=${GIT_REVISION}\nLLVM_REVISION_FE=${LLVM_REVISION_FE}\nLLVM_REVISION_BE=${LLVM_REVISION_BE}\nLLVM_PATH_BE=${LLVM_PATH_BE}\nLLVM_PATH_FE=${LLVM_PATH_FE}\n")
install(FILES ${OCL_BINARY_DIR}/revision.txt DESTINATION ./)

#copy the version file to install folder
install(FILES ${OCL_BINARY_DIR}/version.txt DESTINATION ./)
# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OCL_BINARY_DIR}/buildversion.h.txt ${OCL_BINARY_DIR}/buildversion.h)

# vars

if(CONFORMANCE_LIST)
    string(REPLACE " " ";" CONFORMANCE_LIST ${CONFORMANCE_LIST})
endif()

add_definitions (-DCL_USE_DEPRECATED_OPENCL_1_1_APIS)

if (WIN32 AND ((${CMAKE_CFG_INTDIR} STREQUAL $(OutDir)) OR (${CMAKE_CFG_INTDIR} STREQUAL $(Configuration))))
    set( INSTALL_SUBDIR ${CMAKE_CFG_INTDIR} )
else ()
    set( INSTALL_SUBDIR ${CMAKE_BUILD_TYPE} )
endif ()

set( OCL_GLOBAL_INCLUDE  ${OCL_SOURCE_DIR}/cl_api )
set( OCL_CLANG_HEADERS   ${OCL_SOURCE_DIR}/backend/clang_headers )

# set OS-dependent source tree elements
if (WIN32)
    set(OS            Win)
    set(IMPLIB_SUBDIR lib)
    set(IMPLIB_PREFIX    )
    set(IMPLIB_SUFFIX .lib)
    set(IMPLIB_STATIC_SUFFIX .lib)
    set(DLL_SUFIX .dll)
    set(DYNAMIC_PREFIX dyn_ )
elseif (ANDROID)
    set(OS Android)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
    set(IMPLIB_STATIC_SUFFIX .a)
    set(DLL_SUFIX .so)
    set(DYNAMIC_PREFIX "" )
else ()
    set(OS Lin)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
    set(IMPLIB_STATIC_SUFFIX .a)
    set(DLL_SUFIX .so)
    set(DYNAMIC_PREFIX "" )
endif( )

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ADDR 32)
else ()
    set(ADDR 64)
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

# set windows binary suffix
if (WIN32)
    set (BUILD_PLATFORM ${ADDR})
else (WIN32)
    set (BUILD_PLATFORM "")
endif (WIN32)

set(OCL_SYS_DEPENDENT_SUBDIR        ${OS}${ADDR}_${INSTALL_SUBDIR})
set(OCL_SYS_DEPENDENT_IMPLIB_SUBDIR ${IMPLIB_SUBDIR}/${OCL_SYS_DEPENDENT_SUBDIR})
# END OF set OS-dependent source tree elements


# Define the single binary directory where the Backend tools will be placed
set(OCL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OCL_TOOLS_BINARY_DIR ${OCL_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OCL_TOOLS_BINARY_DIR} )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/lib )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/lib )


SET_UNICODE_OFF()

# define MIC directories
if (INCLUDE_MIC_DEVICE)
    if (WIN32)
        set ( MPSS_PATH               "C:\\PROGRA~1\\Intel\\MPSS" )
        set ( COI_SDK_PATH            "${MPSS_PATH}\\sdk" )
        set ( COI_HOST_INCLUDE_PATH   "${COI_SDK_PATH}\\include" )
        set ( COI_YOCTO_HOST_INCLUDE_PATH   "${COI_SDK_PATH}\\include\\intel-coi" )
        set ( COI_HOST_LIB_PATH       "${COI_SDK_PATH}\\lib" )
        set ( COI_YOCTO_HOST_LIB_PATH       ${COI_HOST_LIB_PATH} )
        set ( COI_DEVICE_INCLUDE_PATH "${COI_SDK_PATH}\\include" )
        set ( COI_YOCTO_DEVICE_INCLUDE_PATH "${COI_SDK_PATH}\\include\\intel-coi" )
        set ( COI_DEVICE_LIB_PATH     "${COI_SDK_PATH}\\lib" )
        set ( COI_YOCTO_DEVICE_LIB_PATH     "${MPSS_PATH}\\k1om-mpss-linux\\usr\\lib64" )
    else (WIN32)
        string(TOLOWER ${CMAKE_BUILD_TYPE} DEBUG_RELEASE)
        set ( COI_SDK_PATH            /opt/intel/mic/coi )
        set ( COI_HOST_INCLUDE_PATH   ${COI_SDK_PATH}/include )
        set ( COI_YOCTO_HOST_INCLUDE_PATH /usr/include/intel-coi )
        set ( COI_HOST_LIB_PATH       ${COI_SDK_PATH}/host-linux-${DEBUG_RELEASE}/lib )
        set ( COI_YOCTO_HOST_LIB_PATH       /usr/lib64 )
        set ( COI_DEVICE_INCLUDE_PATH ${COI_SDK_PATH}/include )
        set ( COI_YOCTO_DEVICE_INCLUDE_PATH /usr/include/intel-coi )
        set ( COI_DEVICE_LIB_PATH     ${COI_SDK_PATH}/device-linux-${DEBUG_RELEASE}/lib )
        set ( COI_YOCTO_DEVICE_LIB_PATH     /opt/mpss/3.1/sysroots/k1om-mpss-linux/usr/lib64 )
    endif (WIN32)

    message ( STATUS "COI host include directory  : ${COI_HOST_INCLUDE_PATH}")
    message ( STATUS "COI host lib directory      : ${COI_HOST_LIB_PATH}")
    message ( STATUS "COI device include directory: ${COI_DEVICE_INCLUDE_PATH}")
    message ( STATUS "COI device lib directory    : ${COI_DEVICE_LIB_PATH}")
endif (INCLUDE_MIC_DEVICE)

# paths
include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_hw_utils/export
        utils/cl_sys_utils/export
        devices/device_commands/export
        externals/tbb/include
        externals/tbb/enhancements
    )

# Framework IDE Folders Names
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set( FRAMEWORK_FOLDER_NAME      Framework )
set( CL_API_HEADERS             ${CMAKE_SOURCE_DIR}/framework/icd/inc)
set( API_FOLDER_NAME            ${FRAMEWORK_FOLDER_NAME}/API )
set( DEVICES_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Devices )
set( RUNTIME_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Runtime )
set( UTILS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/utils )
set( TOOLS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/Tools )
set( FRONTEND_FOLDER_NAME       Frontend )
set( EXTERNAL_FOLDER_NAME       External )



if( (NOT WIN32) AND USE_HARD_TRAPPING )
    add_definitions(-D__HARD_TRAPPING__)
endif((NOT WIN32) AND USE_HARD_TRAPPING)

if(BUILD_LLVM_FROM_SOURCE)
    add_subdirectory(${LLVM_SRC_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/llvm")
endif()

#validation
add_subdirectory( tests )

# The conformance tests must be compiled with the headers located in
# ${OCL_CONFORMANCE_INCLUDE} and not the ones in ${OCL_GLOBAL_INCLUDE}.
# Therefore, it must be added to the include directories only after
# the tests subdirectory has been added.
include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )

add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE=1)

# externals
add_subdirectory( externals )

add_subdirectory( cl_api )
add_subdirectory( utils )

# Backend
add_subdirectory( backend )

#devices
add_subdirectory( devices )

# Front-end compiler
add_subdirectory( clang_compiler )

# Framework
add_subdirectory( framework )

# Common Runtime
if ( INCLUDE_CMRT )
    add_subdirectory ( common_runtime )
endif( INCLUDE_CMRT )

# Tools
if( INCLUDE_TOOLS )
    add_subdirectory ( tools )
endif( INCLUDE_TOOLS )

#workloads
#add_subdirectory( workloads )

#Linux samples
option(INCLUDE_SMPLS "Include Linux Samples into Build"  OFF)
if (INCLUDE_SMPLS AND NOT WIN32 AND NOT ANDROID)
    add_subdirectory( samples )
endif(INCLUDE_SMPLS AND NOT WIN32 AND NOT ANDROID)
