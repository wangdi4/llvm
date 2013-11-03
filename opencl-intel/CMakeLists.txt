#
# Global setup file for OpenCL CMake
#
cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

# CMAKE variables 
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}/cmake/Modules/"
    "${CMAKE_SOURCE_DIR}/cmake_utils/"
)

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

include( cmake_utils/CMakeFuncs.txt )
if (WIN32)
    include( cmake_utils/CMake_windows_tools.cmake )
elseif(APPLE)
    include( cmake_utils/CMake_apple_tools.cmake )
elseif( ANDROID)
    include( CMake_android_tools )
else()
    include( cmake_utils/CMake_linux_tools.cmake )
endif ()

if(CMAKE_CROSSCOMPILING)
  set(CX_NATIVE_TG_DIR "${CMAKE_BINARY_DIR}/native")

  add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CX_NATIVE_TG_DIR}
    COMMENT "Creating ${CX_NATIVE_TG_DIR}...")

  add_custom_command(OUTPUT ${CX_NATIVE_TG_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -UCMAKE_TOOLCHAIN_FILE 
                             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                              // We are cross compiling when building for android, hence we are
                              // dropping off all non required build components, such tests/verification                            
                             -DBACKEND_BUILD_VERIFICATION_LIB=OFF 
                             -DLLVM_PATH=${LLVM_PATH}/native
                             -DBUILD_X64=True
                             ${CMAKE_SOURCE_DIR} 
    WORKING_DIRECTORY ${CX_NATIVE_TG_DIR}
    DEPENDS ${CX_NATIVE_TG_DIR}
    COMMENT "Configuring native targets...")

  add_custom_target(ConfigureNativeTargets DEPENDS ${CX_NATIVE_TG_DIR}/CMakeCache.txt)
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CX_NATIVE_TG_DIR})
endif()

if(INCLUDE_TOOLS)
    if(WIN32)
        if(BUILD_X64)
            set(BOOST_LIB_DIR_SUFFIX x64)
        else(BUILD_X64)
            set(BOOST_LIB_DIR_SUFFIX x86)
        endif(BUILD_X64)
    else(WIN32)
        if(ANDROID)
            set(BOOST_LIB_DIR_SUFFIX android32)
        else(ANDROID)
            set(BOOST_LIB_DIR_SUFFIX x64)
        endif(ANDROID)
    endif(WIN32)
    
    set (BOOST_LIBRARYDIR ${BOOST_ROOT}/lib/${BOOST_LIB_DIR_SUFFIX})
    set (Boost_NO_SYSTEM_PATHS ON) # Don't search PATH
    
# To add new components for Boost simply add to BOOST_COMPONENTS below
    set (BOOST_COMPONENTS regex system filesystem date_time signals)
    
    if(NOT ANDROID)
        FIND_PACKAGE(Boost 1.52.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
        set(Boost_USE_STATIC_RUNTIME OFF)
        set(BOOST_ALL_DYN_LINK ON)
    else(NOT ANDROID)
        set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set (CMAKE_FIND_LIBRARY_PREFIXES "lib")
        find_host_package(Boost 1.52.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
        set (BOOST_LIBRARYDIR ${BOOST_ROOT}/lib/${BOOST_LIB_DIR_SUFFIX})
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
                           )
    list(REMOVE_ITEM BOOST_INSTALL_LIBS debug optimized)
    if(WIN32)
        string(REPLACE ".lib" ".dll;" BOOST_INSTALL_LIBS ${BOOST_INSTALL_LIBS} )
        set (BOOST_INSTALL_LIBS_RELEASE ${BOOST_INSTALL_LIBS})
        set (BOOST_INSTALL_LIBS_DEBUG ${BOOST_INSTALL_LIBS})
        # If adding more libs just add the right index to be removed in the 2 following lines:
        list(REMOVE_AT BOOST_INSTALL_LIBS_RELEASE 1 3 5 7 9) # Debug libs have 'gd' so they apear in the odd spaces
        list(REMOVE_AT BOOST_INSTALL_LIBS_DEBUG 0 2 4 6 8) # Release libs don't have 'gd' so they apear in the even spaces
        INSTALL(FILES ${BOOST_INSTALL_LIBS_RELEASE} DESTINATION bin CONFIGURATIONS Release)
        INSTALL(FILES ${BOOST_INSTALL_LIBS_DEBUG} DESTINATION bin CONFIGURATIONS Debug)
    else(WIN32)
        if(NOT ANDROID)
            set(BOOST_INSTALL_LIBS_LINKS ${BOOST_INSTALL_LIBS})
            SET(BOOST_VER ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
            string(REPLACE ".so" ".so.${BOOST_VER};" BOOST_INSTALL_LIBS_LINKS ${BOOST_INSTALL_LIBS_LINKS} )
            INSTALL(FILES ${BOOST_INSTALL_LIBS} DESTINATION bin)
            INSTALL(FILES ${BOOST_INSTALL_LIBS_LINKS} DESTINATION bin)
        endif(NOT ANDROID)
    endif(WIN32)
endif(INCLUDE_TOOLS)

# Project-wise options
option(SVN_REVISIONS "Make package version from svn revision"  ON)
option(BUILD_QTGUI "Build the QT related projects"  OFF)
option(INCLUDE_MIC_DEVICE "Include MIC device Into Build"  OFF)
option(USE_ITT_INTERNAL "Report internal tasks start/stop to VTune" OFF)
option(USE_HARD_TRAPPING "Use hard trapping for workers on CPU device"  ON)

# find the used external packages

# Subversion
if (SVN_REVISIONS)
    find_package(Subversion 1.6 REQUIRED)
endif ()
# LLVM 
find_package(LLVM REQUIRED)
# QTGUI
if (BUILD_QTGUI AND NOT ANDROID)
    set( QT_VERSION "4.8.1" )
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
        remove_path("c:\\Qt\\${QT_VERSION}\\VS2010-64\\bin")
        remove_path("c:\\Qt\\${QT_VERSION}\\VS2010\\bin")
    execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions /v DefaultQtVersion /d ${QT_VERSION} /f)
        if(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2010-64 /f)
        else(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2010 /f)
        endif(BUILD_X64)
    endif()
    
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

project(OCL)


# extract working copy information for all components into variables
if (SVN_REVISIONS)
    Subversion_WC_INFO(${OCL_SOURCE_DIR} BUILDVER)
else ()
    set( BUILDVER_WC_LAST_CHANGED_REV 0)
    set( BUILDVER_WC_REVISION 0)
endif (SVN_REVISIONS)
message(STATUS "Current build version is ${BUILDVER_WC_LAST_CHANGED_REV}")

math(EXPR BUILDVER_16_BIT_MAJOR "${BUILDVER_WC_REVISION} / 65536")
math(EXPR BUILDVER_16_BIT_MINOR "${BUILDVER_WC_REVISION} % 65536")

# write a file with the BUILDVERSION define
file(WRITE ${OCL_BINARY_DIR}/buildversion.h.txt "#define BUILDVERSION ${BUILDVER_WC_REVISION}\n#ifndef\ _DEBUG\n#define BUILDVERSIONSTR "\"(Build\ ${BUILDVER_WC_REVISION})"\"\n#else\n#define BUILDVERSIONSTR "\"(Build[DEBUG]\ ${BUILDVER_WC_REVISION})"\"\n#endif\n\n#define BUILDVERSION16BIT_MAJOR ${BUILDVER_16_BIT_MAJOR}\n#define BUILDVERSION16BIT_MINOR ${BUILDVER_16_BIT_MINOR}\n#define VERSIONSTRING \"4.0.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}\"\n\n")
if(WIN32)
    file(WRITE ${OCL_BINARY_DIR}/version.ini "[Version]\nPackVer=3.1.${BUILDVER_WC_REVISION}")
    install(FILES ${OCL_BINARY_DIR}/version.ini DESTINATION ./)
endif()
file(WRITE ${OCL_BINARY_DIR}/version.txt "4.0.${BUILDVER_16_BIT_MAJOR}.${BUILDVER_16_BIT_MINOR}")
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
set( OCL_1_2_INCLUDE ${OCL_SOURCE_DIR}/cl_api/CL-1.2 )
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
elseif (APPLE)
    set(OS Mac)
    set(IMPLIB_SUBDIR bin)
    set(IMPLIB_PREFIX lib)
    set(IMPLIB_SUFFIX .so)
    set(IMPLIB_STATIC_SUFFIX .a)
    set(DLL_SUFIX .so)
    set(DYNAMIC_PREFIX "" )
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
# currenlty use absolute path
#    set     ( COI_SDK_PATH           $ENV{INTEL_MPSS_HOST_SDK} )
#    # Remove last backslash from the path and replace long name with the short one
#    string  ( REGEX REPLACE "[\\]$"  "" COI_SDK_PATH ${COI_SDK_PATH} )
#    string  ( REGEX REPLACE "Program Files"  "PROGRA~1" COI_SDK_PATH ${COI_SDK_PATH} )

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

    message ( "COI host include directory  : ${COI_HOST_INCLUDE_PATH}")
    message ( "COI host lib directory      : ${COI_HOST_LIB_PATH}")
    message ( "COI device include directory: ${COI_DEVICE_INCLUDE_PATH}")
    message ( "COI device lib directory    : ${COI_DEVICE_LIB_PATH}")
endif (INCLUDE_MIC_DEVICE)

# paths
#set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES )
include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_hw_utils/export
        utils/cl_sys_utils/export
        devices/device_commands/export
        externals/tbb/include
        externals/tbb/enhancements
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
    include_directories( BEFORE ${OCL_GLOBAL_INCLUDE} )
    add_subdirectory( backend )
else(APPLE)

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

    # Common Runtime
    option(INCLUDE_CMRT "Include Common Runtime into Build"  OFF)
    if ( INCLUDE_CMRT )
      add_subdirectory ( common_runtime )
    endif( INCLUDE_CMRT )
    #tools
    if( INCLUDE_TOOLS )
        add_subdirectory ( tools )
    endif( INCLUDE_TOOLS )

    #workloads
    add_subdirectory( workloads )
endif(APPLE)

# install additional files required by the installer
set(MISC_INSTALLER_FOLDER "bin/sdk")
if (WIN32)  

    install(DIRECTORY tools/ocl_dev/OpenCL_Tools/Sources DESTINATION ${MISC_INSTALLER_FOLDER}/VS_Integration PATTERN .svn EXCLUDE)
    install(DIRECTORY tools/ocl_dev/OpenCL_Extension/bin/Release DESTINATION ${MISC_INSTALLER_FOLDER}/VS_Integration/OpenCL_Extension PATTERN .svn EXCLUDE)
    install(DIRECTORY tools/ocl_dev/OCLConverter/bin DESTINATION ${MISC_INSTALLER_FOLDER}/VS_Integration/OCLConverter PATTERN .svn EXCLUDE)
    install(DIRECTORY tools/ocl_dev/OCLConverter2012/bin DESTINATION ${MISC_INSTALLER_FOLDER}/VS_Integration/OCLConverter2012 PATTERN .svn EXCLUDE)

    install(DIRECTORY samples DESTINATION ${MISC_INSTALLER_FOLDER}/samples PATTERN .svn EXCLUDE)

    install(DIRECTORY setup/AddFiles/PTAC_Tool DESTINATION ${MISC_INSTALLER_FOLDER}/PTAC_Tool)  
    install(FILES setup/redist.txt DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/LicenseAgreement.rtf DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/llvm_release_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES setup/readme.txt DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_Documentation.htm" DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/Android/OpenCL_Android_Install.bat DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/Android/Android_Setup.bat DESTINATION ${MISC_INSTALLER_FOLDER}/setup)

    install(FILES externals/crt/x86/msvcp100.dll DESTINATION ${MISC_INSTALLER_FOLDER}/externals/crt/x86)
    install(FILES externals/crt/x86/msvcr100.dll DESTINATION ${MISC_INSTALLER_FOLDER}/externals/crt/x86)
    install(FILES externals/crt/x64/msvcp100.dll DESTINATION ${MISC_INSTALLER_FOLDER}/externals/crt/x64)
    install(FILES externals/crt/x64/msvcr100.dll DESTINATION ${MISC_INSTALLER_FOLDER}/externals/crt/x64)

    # maybe can delete below
    install(FILES tools/OfflineCompiler/KernelBuilder/kernelbuilder.ico DESTINATION ${MISC_INSTALLER_FOLDER})
        
    install(DIRECTORY cl_api DESTINATION ${MISC_INSTALLER_FOLDER}/cl_api PATTERN .svn EXCLUDE)  
    install(DIRECTORY utils/cl_sys_utils/export DESTINATION ${MISC_INSTALLER_FOLDER}/cl_sys_utils/export PATTERN .svn EXCLUDE)

    install(FILES backend/libraries/ocl_builtins/bin/svml/Win64/__ocl_svml_h8.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES backend/libraries/ocl_builtins/bin/svml/Win64/__ocl_svml_e9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES backend/libraries/ocl_builtins/bin/svml/Win64/__ocl_svml_l9.pdb DESTINATION ${MISC_INSTALLER_FOLDER}) 

    install(FILES backend/libraries/ocl_builtins/bin/svml/Win32/__ocl_svml_n8.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES backend/libraries/ocl_builtins/bin/svml/Win32/__ocl_svml_g9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES backend/libraries/ocl_builtins/bin/svml/Win32/__ocl_svml_s9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})


else (WIN32)

    install(FILES setup/llvm_release_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES setup/apache_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES setup/boost_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
    install(FILES setup/kernel_builder_readme.txt DESTINATION ${MISC_INSTALLER_FOLDER})

    install(FILES setup/Android/OpenCL_Android_Install DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/Android/OpenCL_Android_Uninstall DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/Android/Android_Setup DESTINATION ${MISC_INSTALLER_FOLDER}/setup)
    install(FILES setup/LicenseAgreement_Linux.txt DESTINATION ${MISC_INSTALLER_FOLDER}/setup)

    install(FILES "../utils/RPM_BUILDER/intelocl64.icd" DESTINATION ${MISC_INSTALLER_FOLDER})


endif (WIN32)