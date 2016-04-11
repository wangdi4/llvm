# ----------------------------------------------------------------------------
#  Android x86 CMake toolchain file, for use with the ndk r6b
#  See home page: http://code.google.com/p/android-cmake/
#

set( CMAKE_SYSTEM_NAME Linux )
set( CMAKE_CROSSCOMPILING True )
set( CMAKE_SYSTEM_VERSION 1 )
set( ANDROID_NDK_DEFAULT_SEARCH_PATH ${ANDROID_NDK} )
set( ANDROID_NDK_SUPPORTED_VERSIONS -r8 "")
set( ANDROID_NDK_TOOLCHAIN_DEFAULT_SEARCH_PATH ${ANDROID_NDK_TOOLCHAIN_ROOT} )
set( TOOL_OS_SUFFIX "" )
set( GCC_VERSION "" )
set( GCC_VERSION_OUTPUT "" )

set ( TARGET_ARCH x86_64 )
set ( CMAKE_SKIP_RPATH ON ) # do not embed RPATH/RUNPATH for Android builds
set ( TOOLCHAIN_DIRECTORY_NAME android-toolchain-x86_64 )

message(STATUS "Android Toolchain target arhitecture ${TARGET_ARCH}")

set( ANDROID_NDK_TOOLCHAIN_ROOT $ENV{ANDROID_STANDALONE_TOOLCHAIN64} )
if( NOT ANDROID_NDK_TOOLCHAIN_ROOT )
  execute_process(COMMAND find "/usr/local/" -name ${TOOLCHAIN_DIRECTORY_NAME}  -type d OUTPUT_VARIABLE ANDROID_NDK_TOOLCHAIN_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
endif( NOT ANDROID_NDK_TOOLCHAIN_ROOT )

if (NOT IS_DIRECTORY ${ANDROID_NDK_TOOLCHAIN_ROOT})
    message(FATAL_ERROR "Can't find Android ToolChain Root ${ANDROID_NDK_TOOLCHAIN_ROOT}")
endif (NOT IS_DIRECTORY ${ANDROID_NDK_TOOLCHAIN_ROOT})

set(CMAKE_FIND_ROOT_PATH  ${ANDROID_NDK_TOOLCHAIN_ROOT})
message(STATUS "Android Toolchain dir  ${ANDROID_NDK_TOOLCHAIN_ROOT}")

# specify gcc version
execute_process(COMMAND "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-gcc${TOOL_OS_SUFFIX}" --version
                OUTPUT_VARIABLE GCC_VERSION_OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE )
STRING( REGEX MATCH "[0-9](\\.[0-9])+" GCC_VERSION "${GCC_VERSION_OUTPUT}" )

##############################################################
#        Detect Android API Level
##############################################################

macro( __TOOLCHAIN_DETECT_API_LEVEL _path )
 SET( _expected ${ARGV1} )
 if( NOT EXISTS ${_path} )
  message( FATAL_ERROR "Could not verify Android API level. Probably you have specified invalid level value, or your copy of NDK/toolchain is broken." )
 endif()
 SET( API_LEVEL_REGEX "^[\t ]*#define[\t ]+__ANDROID_API__[\t ]+([0-9]+)[\t ]*$" )
 FILE( STRINGS ${_path} API_FILE_CONTENT REGEX "${API_LEVEL_REGEX}")
 if( NOT API_FILE_CONTENT )
  message( FATAL_ERROR "Could not verify Android API level. Probably you have specified invalid level value, or your copy of NDK/toolchain is broken." )
 endif()
 string( REGEX REPLACE "${API_LEVEL_REGEX}" "\\1" ANDROID_LEVEL_FOUND "${API_FILE_CONTENT}" )
 if( DEFINED _expected )
  if( NOT ${ANDROID_LEVEL_FOUND} EQUAL ${_expected} )
   message( FATAL_ERROR "Specified Android API level does not match level found. Probably your copy of NDK/toolchain is broken." )
  endif()
 endif()
 set( ANDROID_API_LEVEL ${ANDROID_LEVEL_FOUND} CACHE STRING "android API level" FORCE )
endmacro()
message( STATUS "ANDROID_API_LEVEL is ${ANDROID_API_LEVEL}" )

if( NOT DEFINED ANDROID_NDK )
 set( ANDROID_NDK $ENV{ANDROID_NDK} )
endif()
message( STATUS "ANDROID_NDK is ${ANDROID_NDK}" )

if( NOT DEFINED ANDROID_NDK_TOOLCHAIN_ROOT )
 set( ANDROID_NDK_TOOLCHAIN_ROOT $ENV{ANDROID_NDK_TOOLCHAIN_ROOT} )
endif()
message( STATUS "ANDROID_NDK_TOOLCHAIN_ROOT is ${ANDROID_NDK_TOOLCHAIN_ROOT}" )

#set path for android NDK -- look
if( NOT EXISTS "${ANDROID_NDK}" AND NOT DEFINED ANDROID_NDK_TOOLCHAIN_ROOT )
 foreach(ndk_version ${ANDROID_NDK_SUPPORTED_VERSIONS})
  if( EXISTS ${ANDROID_NDK_DEFAULT_SEARCH_PATH}${ndk_version} )
   set ( ANDROID_NDK ${ANDROID_NDK_DEFAULT  _SEARCH_PATH}${ndk_version} )
   message( STATUS "Using default path for android NDK ${ANDROID_NDK}" )
   message( STATUS "  If you prefer to use a different location, please define the variable: ANDROID_NDK" )
   break()
  endif()
 endforeach()
endif()

if( EXISTS "${ANDROID_NDK}" )
 set( ANDROID_NDK "${ANDROID_NDK}" CACHE PATH "root of the android ndk" FORCE )

 if( APPLE )
  set( NDKSYSTEM "darwin-x86" )
 elseif( WIN32 )
  set( NDKSYSTEM "windows" )
  set( TOOL_OS_SUFFIX ".exe" )
 elseif( UNIX )
  set( NDKSYSTEM "linux-x86" )
 else()
  message( FATAL_ERROR "Your platform is not supported" )
 endif()

 set( ANDROID_API_LEVEL $ENV{ANDROID_API_LEVEL} )
 string( REGEX REPLACE "[\t ]*android-([0-9]+)[\t ]*" "\\1" ANDROID_API_LEVEL "${ANDROID_API_LEVEL}" )
 string( REGEX REPLACE "[\t ]*([0-9]+)[\t ]*" "\\1" ANDROID_API_LEVEL "${ANDROID_API_LEVEL}" )

# set( PossibleAndroidLevels "9;10" )
# set( ANDROID_API_LEVEL ${ANDROID_API_LEVEL} TEST STRING "android API level" )
# set_property( TEST ANDROID_API_LEVEL PROPERTY STRINGS ${PossibleAndroidLevels} )

 if( NOT ANDROID_API_LEVEL GREATER 2 )
  set( ANDROID_API_LEVEL 9)
  message( STATUS "Using default android API level android-${ANDROID_API_LEVEL}" )
  message( STATUS "  If you prefer to use a different API level, please define the variable: ANDROID_API_LEVEL" )
 endif()

 set( ANDROID_NDK_SYSROOT "${ANDROID_NDK_TOOLCHAIN_ROOT}/sysroot" )
# set( ANDROID_NDK_SYSROOT "${ANDROID_NDK}/platforms/android-${ANDROID_API_LEVEL}/arch-x86" )
 message( STATUS "ANDROID_NDK_SYSROOT is ${ANDROID_NDK_SYSROOT}" )

 __TOOLCHAIN_DETECT_API_LEVEL( "${ANDROID_NDK_SYSROOT}/usr/include/android/api-level.h" ${ANDROID_API_LEVEL} )

 #message( STATUS "Using android NDK from ${ANDROID_NDK}" )
 set( BUILD_WITH_ANDROID_NDK True )
else()
 #try to find toolchain
 if( NOT EXISTS "${ANDROID_NDK_TOOLCHAIN_ROOT}" )
  set( ANDROID_NDK_TOOLCHAIN_ROOT "${ANDROID_NDK_TOOLCHAIN_DEFAULT_SEARCH_PATH}" )
  message( STATUS "Using default path for toolchain ${ANDROID_NDK_TOOLCHAIN_ROOT}" )
  message( STATUS "  If you prefer to use a different location, please define the variable: ANDROID_NDK_TOOLCHAIN_ROOT" )
 endif()

 set( ANDROID_NDK_TOOLCHAIN_ROOT "${ANDROID_NDK_TOOLCHAIN_ROOT}" CACHE PATH "root of the Android NDK standalone toolchain" FORCE )
 set( ANDROID_NDK_SYSROOT "${ANDROID_NDK_TOOLCHAIN_ROOT}/sysroot/" )

 if( NOT EXISTS "${ANDROID_NDK_TOOLCHAIN_ROOT}" )
  message( FATAL_ERROR "neither ${ANDROID_NDK} nor ${ANDROID_NDK_TOOLCHAIN_ROOT} does not exist!
    You should either set an environment variable:
      export ANDROID_NDK=~/my-android-ndk
    or
      export ANDROID_NDK_TOOLCHAIN_ROOT=~/my-android-toolchain
    or put the toolchain or NDK in the default path:
      sudo ln -s ~/my-android-ndk ${ANDROID_NDK_DEFAULT_SEARCH_PATH}
      sudo ln -s ~/my-android-toolchain ${ANDROID_NDK_TOOLCHAIN_DEFAULT_SEARCH_PATH}" )
 endif()

 __TOOLCHAIN_DETECT_API_LEVEL( "${ANDROID_NDK_SYSROOT}/usr/include/android/api-level.h" )

 message( STATUS "Using android NDK standalone toolchain from ${ANDROID_NDK_TOOLCHAIN_ROOT}" )
 set( BUILD_WITH_ANDROID_NDK_TOOLCHAIN True )
endif()

# specify the cross compiler
set( CMAKE_C_COMPILER   "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-gcc${TOOL_OS_SUFFIX}"     CACHE PATH "gcc" FORCE )
set( CMAKE_CXX_COMPILER "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-g++${TOOL_OS_SUFFIX}"     CACHE PATH "g++" FORCE )
set( CMAKE_AR           "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-ar${TOOL_OS_SUFFIX}"      CACHE PATH "archive" FORCE )
set( CMAKE_LINKER       "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-ld${TOOL_OS_SUFFIX}"      CACHE PATH "linker" FORCE )
set( CMAKE_NM           "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-nm${TOOL_OS_SUFFIX}"      CACHE PATH "nm" FORCE )
set( CMAKE_OBJCOPY      "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-objcopy${TOOL_OS_SUFFIX}" CACHE PATH "objcopy" FORCE )
set( CMAKE_OBJDUMP      "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-objdump${TOOL_OS_SUFFIX}" CACHE PATH "objdump" FORCE )
set( CMAKE_STRIP        "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-strip${TOOL_OS_SUFFIX}"   CACHE PATH "strip" FORCE )
set( CMAKE_RANLIB       "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-ranlib${TOOL_OS_SUFFIX}"  CACHE PATH "ranlib" FORCE )

# Assembly support
set( CMAKE_ASM_COMPILER "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/x86_64-linux-android-gcc${TOOL_OS_SUFFIX}"  CACHE PATH "gcc" FORCE )
set( CMAKE_ASM_INCLUDE_DIR_FLAG "-I" )
set( CMAKE_ASM_OUTPUT_NAME_FLAG "-o" )
enable_language(ASM)

##############################################################
#        Set OUTPUT directories
##############################################################

set(BIN_OUTPUT_DIR_SUFFIX "lin")

#setup output directories
if( 0 )  # DISABLED PART
set( LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_SOURCE_DIR} CACHE PATH "root for library output, set this to change where android libs are installed to" )

SET( DO_NOT_CHANGE_OUTPUT_PATHS_ON_FIRST_PASS OFF CACHE BOOL "")
if( DO_NOT_CHANGE_OUTPUT_PATHS_ON_FIRST_PASS )
 if( EXISTS "${CMAKE_SOURCE_DIR}/jni/CMakeLists.txt" )
  set( EXECUTABLE_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH_ROOT}/bin/${ARMEABI_NDK_NAME}" CACHE PATH "Output directory for applications")
 else()
  set( EXECUTABLE_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH_ROOT}/bin" CACHE PATH "Output directory for applications")
 endif()
 set( LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ARMEABI_NDK_NAME}" CACHE PATH "path for android libs")
 set( CMAKE_INSTALL_PREFIX "${ANDROID_NDK_TOOLCHAIN_ROOT}/user" CACHE STRING "path for installing" )
endif()
SET( DO_NOT_CHANGE_OUTPUT_PATHS_ON_FIRST_PASS ON CACHE INTERNAL "" FORCE)
endif( ) # DISABLED PART

# where is the target environment
set( CMAKE_FIND_ROOT_PATH "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin" "${ANDROID_NDK_TOOLCHAIN_ROOT}/x86_64-linux-android" "${ANDROID_NDK_SYSROOT}" "${CMAKE_INSTALL_PREFIX}" "${CMAKE_INSTALL_PREFIX}/share" )

if( BUILD_WITH_ANDROID_NDK )
 set( STL_PATH "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++" )
 set( STL_LIBRARIES_PATH "${STL_PATH}/libs/${ARMEABI_NDK_NAME}" )
 include_directories(SYSTEM "${STL_PATH}/include" "${STL_LIBRARIES_PATH}/include" )
 if ( NOT ARMEABI AND NOT FORCE_ARM )
  set( STL_LIBRARIES_PATH "${ANDROID_NDK_TOOLCHAIN_ROOT}/x86_64-linux-android/lib64/${CMAKE_SYSTEM_PROCESSOR}" )
 endif()
 #ARK: Had to add this to find STL headers
 include_directories(SYSTEM "${ANDROID_NDK_TOOLCHAIN_ROOT}/lib/gcc/x86_64-linux-android/${GCC_VERSION}/include/" )
 include_directories(SYSTEM "${ANDROID_NDK_TOOLCHAIN_ROOT}/x86_64-linux-android/include/c++/${GCC_VERSION}/" )
 include_directories(SYSTEM "${ANDROID_NDK_TOOLCHAIN_ROOT}/x86_64-linux-android/include/c++/${GCC_VERSION}/x86_64-linux-android" )
endif()

if( BUILD_WITH_ANDROID_NDK_TOOLCHAIN )
 set( STL_LIBRARIES_PATH "${ANDROID_NDK_TOOLCHAIN_ROOT}/x86_64-linux-android/lib64" )
 if( NOT ARMEABI )
  set( STL_LIBRARIES_PATH "${STL_LIBRARIES_PATH}/${CMAKE_SYSTEM_PROCESSOR}" )
 endif()
 if( NOT FORCE_ARM )
  set( STL_LIBRARIES_PATH "${STL_LIBRARIES_PATH}" )
 endif()
 #for some reason this is needed? TODO figure out why...
 include_directories(SYSTEM "${ANDROID_NDK_TOOLCHAIN_ROOT}/x86_64-linux-android/include/c++/${GCC_VERSION}/x86_64-linux-android" )
endif()
message( STATUS "STL_LIBRARIES_PATH is ${STL_LIBRARIES_PATH}" )

# only search for libraries and includes in the ndk toolchain
# ARK - Had the change the CMAKE_FIND_ROOT_PATH_MODE_PROGRAM variable to FIRST
# so that Perl could be found
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )

if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
  set (FSTACK_PROTECTOR_C_FLAGS       "-fstack-protector-strong" )
else (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
  set (FSTACK_PROTECTOR_C_FLAGS       "-fstack-protector" )
endif ()

set( CMAKE_CXX_FLAGS " -m64 -fPIC -DANDROID -Wno-psabi -fsigned-char -Wformat -Wformat-security ${FSTACK_PROTECTOR_C_FLAGS}" )
set( CMAKE_C_FLAGS " -m64 -fPIC -DANDROID -Wno-psabi -fsigned-char -Wformat -Wformat-security ${FSTACK_PROTECTOR_C_FLAGS}" )

if( BUILD_WITH_ANDROID_NDK )
 set( CMAKE_CXX_FLAGS "--sysroot=${ANDROID_NDK_SYSROOT} ${CMAKE_CXX_FLAGS}" )
 set( CMAKE_C_FLAGS "--sysroot=${ANDROID_NDK_SYSROOT} ${CMAKE_C_FLAGS}" )

 # workaround for ugly cmake bug - compiler identification replaces all spaces (and somethimes " !!!) in compiler flags with ; symbol
 # as result identification fails if ANDROID_NDK_SYSROOT contain spaces
 include(CMakeForceCompiler)
 CMAKE_FORCE_C_COMPILER("${CMAKE_C_COMPILER}" GNU)
 CMAKE_FORCE_CXX_COMPILER("${CMAKE_CXX_COMPILER}" GNU)
endif()

if( INCLUDE_ATOM )
 set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3" )
 set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mssse3" )
endif()

# -O3 generates unaligned movdqa instructions.
set ( CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" )
set ( CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "c++ flags" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "c flags" )

set( CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Xassembler --64" )

#-------------------------------------------------
# My add Rami
#-------------------------------------------------
set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99" )
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS}" )

set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb3 -D _DEBUG" )
set (ADD_C_FLAGS_RELEASE "-O2 -ggdb2 -U _DEBUG")
set (ADD_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -U _DEBUG")

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")
set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${ADD_C_FLAGS_RELWITHDEBINFO}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${ADD_CXX_FLAGS_RELWITHDEBINFO}")
#-------------------------------------------------
# My add Rami - End
#-------------------------------------------------

set( LINKER_FLAGS "-L${STL_LIBRARIES_PATH} -lc++ -z noexecstack -z relro -z now" )
add_definitions( -D GTEST_USE_OWN_TR1_TUPLE=1 )

set( NO_UNDEFINED ON CACHE BOOL "Don't allow undefined symbols" )
if( NO_UNDEFINED )
 set( LINKER_FLAGS "-Wl,--no-undefined ${LINKER_FLAGS}" )
endif()

set( VERBOSE_LINKER_OUTPUT OFF CACHE BOOL "Show linker errors" )
if( VERBOSE_LINKER_OUTPUT )
 set( LINKER_FLAGS "-Wl,-V ${LINKER_FLAGS}" )
endif()

set( CMAKE_SHARED_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE )
set( CMAKE_MODULE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE )
set( CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE )

#set these global flags for cmake client scripts to change behavior
set( ANDROID True )
set( BUILD_ANDROID True )

#macro to find packages on the host OS
macro(find_host_package)
 set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM_SAVE ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM})
 set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY_SAVE ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
 set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE_SAVE ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
 set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
 set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
 set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )
 if( CMAKE_HOST_WIN32 )
  SET( WIN32 1 )
  SET( UNIX )
 elseif( CMAKE_HOST_APPLE )
  SET( APPLE 1 )
  SET( UNIX )
 endif()
 find_package( ${ARGN} )
 SET( WIN32 )
 SET( APPLE )
 SET( UNIX 1)
 set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM_SAVE} )
 set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY_SAVE} )
 set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE_SAVE} )
endmacro()
#macro to find programs on the host OS
macro(find_host_program)
 set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM_SAVE ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM})
 set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY_SAVE ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
 set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE_SAVE ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
 set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
 set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
 set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )
 if( CMAKE_HOST_WIN32 )
  SET( WIN32 1 )
  SET( UNIX )
 elseif( CMAKE_HOST_APPLE )
  SET( APPLE 1 )
  SET( UNIX )
 endif()
 find_program( ${ARGN} )
 SET( WIN32 )
 SET( APPLE )
 SET( UNIX 1)
 set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM_SAVE} )
 set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY_SAVE} )
 set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE_SAVE} )
endmacro()

MARK_AS_ADVANCED(FORCE_ARM NO_UNDEFINED)

message(STATUS "\n\n** ** ** COMPILER Definitions ** ** **")
message(STATUS "CMAKE_C_COMPILER        = ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_FLAGS           = ${CMAKE_C_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_CXX_COMPILER      = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS         = ${CMAKE_CXX_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_ASM_COMPILER      = ${CMAKE_ASM_COMPILER}")
message(STATUS "CMAKE_ASM_FLAGS         = ${CMAKE_ASM_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_EXE_LINKER_FLAGS  = ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_BUILD_TOOL        = ${CMAKE_BUILD_TOOL}")
