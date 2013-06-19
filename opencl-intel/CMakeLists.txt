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
        if(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2010-64 /f)
        else(BUILD_X64)
            execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v InstallDir /d c:\\qt\\${QT_VERSION}\\vs2010 /f)
        endif(BUILD_X64)
    else()
        set(BOOST_LIB_DIR_SUFFIX x64)
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

# write a file with the BUILDVERSION define
file(WRITE ${OCL_BINARY_DIR}/buildversion.h.txt "#define BUILDVERSION ${BUILDVER_WC_REVISION}\n#ifndef\ _DEBUG\n#define BUILDVERSIONSTR "\"(Build\ ${BUILDVER_WC_REVISION})"\"\n#else\n#define BUILDVERSIONSTR "\"(Build[DEBUG]\ ${BUILDVER_WC_REVISION})"\"\n#endif")
file(WRITE ${OCL_BINARY_DIR}/version.txt "3.0.${BUILDVER_WC_REVISION}")
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


# paths
#set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES )
include_directories( BEFORE
        utils/cl_logger/export
        utils/cl_hw_utils/export
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
	if(BUILD_LLVM_FROM_SOURCE)
		add_subdirectory(${LLVM_SRC_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/llvm")
	endif()

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
    if ( INCLUDE_CMRT )
      add_subdirectory ( common_runtime )
    endif( INCLUDE_CMRT )
    #tools
    if( INCLUDE_TOOLS )
        add_subdirectory ( tools )
    endif( INCLUDE_TOOLS )

    #validation
    add_subdirectory( tests )

    #workloads
    add_subdirectory( workloads )
endif(APPLE)

# install addional files requiresd for the installer
set(MISC_INSTALLER_FOLDER "bin/sdk")
if (WIN32)	

	install(FILES externals/crt/x86/msvcp100.dll DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES externals/crt/x86/msvcr100.dll DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES setup/llvm_release_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
	

	install(FILES externals/crt/x64/msvcp100.dll DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES externals/crt/x64/msvcr100.dll DESTINATION ${MISC_INSTALLER_FOLDER})
	
	install(FILES backend/libraries/ocl_builtins/bin/svml/Win32/__ocl_svml_n8.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES backend/libraries/ocl_builtins/bin/svml/Win32/__ocl_svml_g9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES backend/libraries/ocl_builtins/bin/svml/Win32/__ocl_svml_s9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
	
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL/OpenCLFile.cl DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL/OpenCLFile.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL/OpenCLFile.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojects/OpenCL.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojects/OpenCL/OpenCLProj.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojects/OpenCL/OpenCLProj.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL/OpenCLFile.cl DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL/OpenCLFile.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojectitems/OpenCL/OpenCLFile.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojects/OpenCL.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojects/OpenCL/OpenCLProj.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/vcprojects/OpenCL/OpenCLProj.vsdir DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/html/1033/AppSettings.htm DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/html/1033/default.htm DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/ConsoleApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/DLLApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/opencl.png DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/StaticLibraryApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/Win32_1.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/Win32_Background.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/WindowsApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2008/VCWizards/OpenCL/scripts/1033/default.js DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/dllmain.cpp DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/readme.txt DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/resource.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.cpp DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.rc DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/small.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/stdafx.cpp DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/stdafx.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/targetver.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/Templates.inf DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2008/VCProjectDefaults/IntelOpenCL.rules DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2008/vcprojects/OpenCL/OpenCLProj.vsz DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2010/VCProjectDefaults/IntelOpenCL.props DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2010/VCProjectDefaults/IntelOpenCL.targets DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2010/VCProjectDefaults/IntelOpenCL.xml DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/html/1033/AppSettings.htm DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/html/1033/default.htm DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/ConsoleApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/DLLApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/opencl.png DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/StaticLibraryApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/Win32_1.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/Win32_Background.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/images/WindowsApp.gif DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2010/VCWizards/OpenCL/scripts/1033/default.js DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/dllmain.cpp DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/readme.txt DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/resource.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.cpp DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/root.rc DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/small.ico DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/stdafx.cpp DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/stdafx.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/targetver.h DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC/VCWizards/OpenCL/templates/1033/Templates.inf DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Tools/Sources/VC2010/vcprojects/OpenCL/OpenCLProj.vsz DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Extension/bin/Release/extension.vsixmanifest DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES tools/ocl_dev/OpenCL_Extension/bin/Release/IntelOCLSupport.dll DESTINATION ${MISC_INSTALLER_FOLDER})

	install(FILES backend/libraries/ocl_builtins/bin/svml/Win64/__ocl_svml_h8.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES backend/libraries/ocl_builtins/bin/svml/Win64/__ocl_svml_e9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES backend/libraries/ocl_builtins/bin/svml/Win64/__ocl_svml_l9.pdb DESTINATION ${MISC_INSTALLER_FOLDER})
	
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_Documentation.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Align_Pointers_to_the_DX10_Buffers_Upon_Mapping.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Applying_Shared_Local_Memory_(SLM).htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Avoid_Extracting_Vector_Components.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Avoid_Handling_Edge_Conditions_in_Kernels.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Avoid_Needless_Synchronization.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Avoid_Spurious_Operations_in_Kernel_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Background.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Basic_Concepts.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Basic_Frequency_Considerations.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Benefitting_From_Implicit_Vectorization.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Built-In_Functions.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Comparing_OpenCL_Kernel_Performance_with_Performance_of_Native_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Considering_native_Versions_of_Math_Built-Ins.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Eliminate_Device_Starvation.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Execution_of_OpenCL_Work_Items_the_SIMD_Machine.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Gather4_Rule_of_Thumb__Loading_Storing_Data_in_Greatest_Chunks.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Getting_Credible_Performance_Numbers.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Global_Memory_Size.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Granularity.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Host-Side_Timing.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Intel_SDK_for_OpenCL_Applications_Optimization_Guide.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Interoperability_with_Other_APIs.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Introduction.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Keeping_Kernel_Sources_the_Same.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Kernel_Memory_Access_Optimization_Summary.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Legal_Information.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Limitations_of_Shared_Context_with_Respect_to_Extensions.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Local_Memory_Considerations.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Local_Memory_Usage.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Mapping_Memory_Objects_(USE_HOST_PTR).htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Memory_Access_Considerations.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Minimize_Data_Copying.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/NavScript.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Note_on_Intel_Quick_Sync_Video.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Note_on_Loops.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Notes_on_Branching_Loops.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Optimizing_Utilization_of_Execution_Units.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Partitioning_the_Work.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Perform_Initialization_in_a_Separate_Task.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Prefer_Row-Wise_Data_Acceses.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Prefer_Signed_Integer_Data_Types_Over_Unsigned.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Profiling_Operations_Using_OpenCL_Profiling_Events.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Related_Documents.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Reusing_Compilation_Results_with_clCreateProgramWithBinary.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Sharing_Resources_Efficiently.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Synchronization_Caveats.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Task-Parallel_Programming_Model_Hints.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Tips_for_Auto-Vectorization_Module.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Turbo_Boost_Technology.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Use_D3D10_RESOURCE_MISC_SHARED_flag.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Use_the_Preprocessor_for_Constants.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Using_Buffers_and_Images_Appropriately.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Using_Data_Parallelism.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Using_Floating_Point_for_Calculations.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Using_Multiple_OpenCL_Devices.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Using_Tools.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Using_Vector_Data_Types.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Why_Optimizing_Kernel_Code_Is_Important.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Work-Group_Level_Parallelism.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Work-Group_Size_Considerations.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Work-Group_Size_Recommendations_Summary.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Working_with_the_OpenCL_Installable_Client_Driver.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Writing_Kernels_to_Directly_Target_the_Intel_Architecture_Vector_Processors.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Writing_OpenCL_Code_for_Intel(R)_Architecture_Processors.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/Writing_to_a_Shared_Resource.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/__private_Memory.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/ehlpdhtm.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/global_Memory_and___constant_Memory.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/hh_goto.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/hh_index.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/hh_search.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/hh_toc.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/index.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/intel_css_styles.css" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/local_Memory.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/print.css" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/search.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/searchdata.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/tree.css" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/tree.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/treenodedot.gif" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/treenodeminus.gif" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/treenodeplus.gif" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/Basic_Concepts.jpg" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/Execution_of_OpenCL_Work_Items.jpg" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM1.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM10.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM11.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM3.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM4.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM5.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM6.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM7.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM8.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/GM9.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_OpenCL_Optimization_Guide/OG_files/title-software-tools.jpg" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Analyzing_the_OpenCL_Kernel_Performance.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Best_Worst_Configuration_Results.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Building_the_OpenCL_Kernels.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Building_the_OpenCL_Project.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Choosing_a_Different_Target_Instruction_Set_Architecture.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Choosing_a_Target_Device.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Choosing_a_Target_Device_VS.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Choosing_the_Target_OpenCL_Kernel.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Compiling_You_OpenCL_Kernels.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Components_and_Packages.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Configuring_Intel_SDK_for_OpenCL_Debugger.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Configuring_Microsoft_Visual_Studio.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Configuring_the_Build_Options.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Configuring_the_Build_Options_VS.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Creating_new_OpenCL_Project_Template.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Debugging_Your_OpenCL_Kernel_with_Intel_SDK_for_OpenCL_Debugger.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Develop_with_the_Intel_OpenCL_Runtime_on_Linux_Operating_System.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Develop_with_the_Intel_OpenCL__Platform_on_Microsoft_Windows__Operating_Systems.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/DirectX_10_Sharing.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/DirectX_11_Sharing.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Double_Precision_Floating_Point.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Efficient_Highly_Scalable_Threading_System.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Enabling_OpenCL_Source_Level_Profiling.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Enabling_OpenCL__API_Tracing.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Generating_Intermediate_Program_Binaries.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Generating_Intermediate_Program_Binaries_VS.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Generating_a_Trace_File.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Generating_and_Viewing_Assembly_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Generating_and_Viewing_LLVM_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Graphical_View_of_Execution_Timelines.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Image_Support.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Intel_Device_Fission_Extension.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Intel_DirectX_9_Media_Sharing_Extension.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Intel_Immediate_Command_Execution_Extension.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Intel_SDK_for_OpenCL_Applications_Packages.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Intel_SDK_for_OpenCL_Applications_User_Guide.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Interoperability_with_Media_and_Graphics_APIs_on_Processor_Graphics.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Interoperability_with_the_Intel(R)_VTune(tm)_Amplifier_XE_on_Linux__OS.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Introducing_the_Intel_SDK_for_OpenCL_-_Debugger_for_Linux_OS.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Introduction.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Legal_Information.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Linking_OpenCL_Program_Binaries.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Math_Intrinsics_Options.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Native_Kernels.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/NavScript.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/OpenCL_1.1_Full_Profile.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/OpenCL_Context_View.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/OpenCL_Device_View.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/OpenCL_Installable_Client_Driver_(ICD).htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/OpenCL__Extensions_and_Optional_Features.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/OpenGL_Sharing.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Optimization_Options.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Options_Controlling_the_OpenCL_C_version.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Options_to_Request_or_Support__Warnings.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Out-Of-Order_Execution.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Preprocessor_Options.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Programming_with_the_Intel_SDK_for_OpenCL_Applications_Tools.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Read-Only_Surface_Formats.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Read-Write_Surface_Formats.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Related_Information.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Resource_Sharing.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Saving_Your_Code_to_a_Text_File.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Seamless_Vectorization_Using_the_Implicit_CPU_Vectorization_Module.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Setting_a_New_Profiling_Project.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Shared_Context_for_CPU_and_Processor_Graphics.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Source_Level_Profiling_of_Your_OpenCL_Kernel.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Statistics_for_Each_Configuration.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Statistics_per_Iteration.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/The_OpenCL_Platform_on_Intel_Processors.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Troubleshooting_the_Intel_SDK_for_OpenCL_Debugger.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Tuning_with_the_Intel_Graphics_Performance_Analyzers.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Tuning_with_the_Intel_VTune_Amplifier_XE.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_Offline_Compiler_Integration_with_Microsoft_Visual_Studio.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_Offline_Complier_Command_Line_Interface.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_the_Intel(R)_Graphics_Performance_Analyzers_(Intel(R)_GPA)_Platform_Analyzer.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_the_Intel_Graphics_Performance_Analyzers_System_Analyzer.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_the_Intel_SDK_for_OpenCL_Debugger.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_the_Intel_SDK_for_OpenCL_Offline_Compiler_Standalone_Tool.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_the_Intel_SDK_for_OpenCL__Offline_Compiler.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Using_the_OpenCL_Build_Properties.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Variable_Statistics.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Viewing_the_Generated_Assembly_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Viewing_the_Generated_LLVM_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Viewing_the_OpenCL_Kernels_Assembly_Code.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Working_with_the_OpenCL_Installable_Client_Driver_(ICD).htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Write-Only_Surface_Formats.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/Writing_to_3D_image_memory_object.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/ehlpdhtm.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/hh_goto.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/hh_index.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/hh_search.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/hh_toc.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/index.htm" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/intel_css_styles.css" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/print.css" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/search.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/searchdata.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/tree.css" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/tree.js" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/treenodedot.gif" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/treenodeminus.gif" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/treenodeplus.gif" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/BW_results.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Create_Program_Binary.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Load_from_File.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler1.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler3.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler4.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler5.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler6.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Offline_compiler8.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Save_As.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Statistics_per_Iteration.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Tracing.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Tracing1.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Tracing2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration1.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration10.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration11.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration3.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration4.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration5.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration6.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration7.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration8.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/VS_integration9.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Viewing_Assembly_Code.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Viewing_Assembly_Code1.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/Viewing_Assembly_Code2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/analyze_user_tasks.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/analyzing_kernel.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/analyzing_kernel2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/analyzing_kernel3.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/analyzing_kernel4.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/bottom-up.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/build.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/compile_button.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/compiling_your_opencL_kernels.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/compiling_your_opencL_kernels2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/components.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/configuring_MSVS.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/configuring_MSVS1.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/debugger.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/device_view.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/interoperability.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/link.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/linking_program_binaries.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/linking_program_binaries2.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/offline_compiler7.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/platform_model.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/red.jpg" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/startbtn.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/statistics_for_each.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/task_CPU_time.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/task_analysis.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/title-software-tools.jpg" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/tracing3.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/tracing4.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/turquoise.jpg" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/variable_statistics.png" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES "setup/docs/Intel(R)_SDK_for_OpenCL_Applications_User_Guide/UG_files/view_assebly_code.png" DESTINATION ${MISC_INSTALLER_FOLDER})


else (WIN32)

	install(FILES "../utils/RPM_BUILDER/intelocl64.icd" DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES setup/llvm_release_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES setup/boost_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES setup/apache_license.txt DESTINATION ${MISC_INSTALLER_FOLDER})
	install(FILES setup/kernel_builder_readme.txt DESTINATION ${MISC_INSTALLER_FOLDER})

endif (WIN32)
