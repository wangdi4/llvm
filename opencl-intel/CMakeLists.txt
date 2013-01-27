#
# Global setup file for OpenCL CMake
#

cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

# CMAKE variables 
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}/cmake/Modules/"
    "${CMAKE_SOURCE_DIR}/cmake_utils/"
)

include( cmake_utils/CMakeFuncs.txt )
if (WIN32)
    include( cmake_utils/CMake_windows_tools.cmake )
elseif( ANDROID )
    include( cmake_utils/CMake_android_tools.cmake )
elseif (APPLE)
    include( cmake_utils/CMake_apple_tools.cmake )
else ()
    include( cmake_utils/CMake_linux_tools.cmake )
endif ()


if (INCLUDE_TOOLS)
    FIND_PACKAGE(Boost 1.52.0 REQUIRED)
endif(INCLUDE_TOOLS)
#PRINT_TOOLS()

# Project-wise options
option(SVN_REVISIONS "Make package version from svn revision"  ON)
option(BUILD_QTGUI "Build the QT related projects"  OFF)
option(INCLUDE_MIC_DEVICE "Include MIC device Into Build"  OFF)

# find the used external packages

# Subversion
if (SVN_REVISIONS)
    find_package(Subversion 1.6 REQUIRED)
endif ()
# LLVM 
find_package(LLVM REQUIRED)
# QTGUI
if (BUILD_QTGUI)
    set( QT_VERSION "4.8.1" )
    if(DEFINED QT_REVISION)
       set( QT_VERSION ${QT_REVISION})
    endif()

    #install the QT and boost libraries themselfs
    if(NOT WIN32)
        set(BOOST_LIB_DIR_SUFFIX x64)
    else( NOT WIN32)
        # special processing for QT library on windows:
        # the problem is that QT doesn't naturally supports installing 
        # both 32 and 64 bits libraries on the same machine (there is 
        # only a single registry settings for QT library per machine )
        # this makes it difficult to rely on default QT Cmake support
        # to find the appropriate version. 
        # The trick is to set the correct installation directory on the fly
        # by changing the registry information and removing the default paths
        remove_path("c:\\Qt\\${QT_VERSION}\\VS2010-64\\bin")
        remove_path("c:\\Qt\\${QT_VERSION}\\VS2010\\bin")
        if(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2010-64 /f)
        else(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2010 /f)
        endif(BUILD_X64)
    endif(NOT WIN32)
    
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
        INSTALL(FILES ${SHARED_DEBUG_LIBS} DESTINATION bin CONFIGURATIONS Debug)
        INSTALL(FILES ${SHARED_RELEASE_LIBS} DESTINATION bin CONFIGURATIONS Release)
    else(WIN32)
        SET( SHARED_LIBS 
            ${QT_LIBRARY_DIR}/libQtCore.so.4
            ${QT_LIBRARY_DIR}/libQtGui.so.4
        )
        SET( SQUISH_LIBS 
            ${QT_LIBRARY_DIR}/libQt3Support.so.4
            ${QT_LIBRARY_DIR}/libQtNetwork.so.4
            ${QT_LIBRARY_DIR}/libQtSql.so.4
            ${QT_LIBRARY_DIR}/libQtXml.so.4
        )
        INSTALL(FILES ${SHARED_LIBS} DESTINATION bin)
        INSTALL(FILES ${SQUISH_LIBS} DESTINATION bin)
        #install(FILES  ${QT_LIBRARY_DIR}/externals/boost_1_46_1/lib/linux/${BOOST_LIB_DIR_SUFFIX}/libboost_filesystem.so DESTINATION bin)
        #install(FILES  ${QT_LIBRARY_DIR}/externals/boost_1_46_1/lib/linux/${BOOST_LIB_DIR_SUFFIX}/libboost_system.so DESTINATION bin)
        #install(FILES  ${QT_LIBRARY_DIR}/externals/boost_1_46_1/lib/linux/${BOOST_LIB_DIR_SUFFIX}/libboost_filesystem.so.1.46.1 DESTINATION bin)
        #install(FILES  ${QT_LIBRARY_DIR}/externals/boost_1_46_1/lib/linux/${BOOST_LIB_DIR_SUFFIX}/libboost_system.so.1.46.1 DESTINATION bin)
    endif(WIN32)
endif ()




#
# Project defs
#

project(OCL)


# extract working copy information for all components into variables
if (SVN_REVISIONS)
    Subversion_WC_INFO(${OCL_SOURCE_DIR} BUILDVER)
    message(STATUS "Current build version is ${BUILDVER_WC_LAST_CHANGED_REV}")
    
    # write a file with the BUILDVERSION define
    file(WRITE ${OCL_BINARY_DIR}/buildversion.h.txt "#define BUILDVERSION ${BUILDVER_WC_REVISION}\n#ifndef\ _DEBUG\n#define BUILDVERSIONSTR "\"(Build\ ${BUILDVER_WC_REVISION})"\"\n#else\n#define BUILDVERSIONSTR "\"(Build[DEBUG]\ ${BUILDVER_WC_REVISION})"\"\n#endif")
    file(WRITE ${OCL_BINARY_DIR}/version.txt "3.0.${BUILDVER_WC_REVISION}")
    #copy the version file to install folder
    install(FILES ${OCL_BINARY_DIR}/version.txt DESTINATION ./)
    # copy the file to the final header only if the version changes
    # reduces needless rebuilds
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OCL_BINARY_DIR}/buildversion.h.txt ${OCL_BINARY_DIR}/buildversion.h)
endif (SVN_REVISIONS)

# vars
string(REPLACE " " ";" CONFORMANCE_LIST ${CONFORMANCE_LIST})

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
elseif (APPLE)
    set(OS Mac)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
    set(IMPLIB_STATIC_SUFFIX .a)
    set(DLL_SUFIX .so)
else ()
    set(OS Lin)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
    set(IMPLIB_STATIC_SUFFIX .a)
    set(DLL_SUFIX .so)
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


# paths
#set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES )
include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_sys_utils/export
        externals/tbb/include
        ${OCL_GLOBAL_INCLUDE}
        ${LLVM_INCLUDE_DIR}
    )

# Framework IDE Folders Names
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set( FRAMEWORK_FOLDER_NAME      Framework )
set( API_FOLDER_NAME            ${FRAMEWORK_FOLDER_NAME}/API )
set( DEVICES_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Devices )
set( RUNTIME_FOLDER_NAME        ${FRAMEWORK_FOLDER_NAME}/Runtime )
set( UTILS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/utils )
set( TOOLS_FOLDER_NAME          ${FRAMEWORK_FOLDER_NAME}/Tools )
set( FE_COMPILER_FOLDER_NAME    "${FRAMEWORK_FOLDER_NAME}/Front-end Compiler" )
set( EXTERNAL_FOLDER_NAME       External )

if (APPLE)
    add_subdirectory( backend )
else(APPLE)
    # externals
    add_subdirectory(externals)

    add_subdirectory( cl_api )
    add_subdirectory( utils )

    # Backend
    add_subdirectory( backend )

    #devices
    add_subdirectory( devices )

    #FE compilers
    add_subdirectory( fe_compilers )

    # Framework
    add_subdirectory( framework )

    # Common Runime
    option(INCLUDE_CMRT "Include Common Runtime into Build"  OFF)
    if (WIN32 AND INCLUDE_CMRT)
        add_subdirectory ( common_runtime )
    endif(WIN32 AND INCLUDE_CMRT)
    #tools
	if( INCLUDE_TOOLS )
    add_subdirectory ( tools )
	endif( INCLUDE_TOOLS )


    #validation
    add_subdirectory( tests )

    #workloads
    add_subdirectory( workloads )
endif(APPLE)
