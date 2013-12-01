# include blocker
if(__LINUX_MEEGO_CROSS)
  return()
endif()
set(__LINUX_MEEGO_CROSS 1)

if (BUILD_X64)
    set(SYSROOT_NAME "linux-x86_64")
else()
    set(SYSROOT_NAME "linux-i686")
endif()

# Not setting to Meego, since it causes problems with many libraries
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1.08)
set(UNIX true)

message(STATUS "Setting up Meego ${CMAKE_SYSTEM_VERSION} cross-system toolchain for ${SYSROOT_NAME}")


# where is the target environment 
execute_process(COMMAND find /usr/lib/madde/${SYSROOT_NAME}/sysroots -name *madde-sysroot-${CMAKE_SYSTEM_VERSION}*  -type d
    OUTPUT_VARIABLE _SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT IS_DIRECTORY ${_SYSROOT})
    message(FATAL_ERROR "Can't find Meego SDK in /usr/lib/madde/${SYSROOT_NAME}/sysroots")
endif (NOT IS_DIRECTORY ${_SYSROOT})

set(CMAKE_FIND_ROOT_PATH  ${_SYSROOT})
set(SYSROOT ${_SYSROOT})

message(STATUS "Goig to take Meego SDK from: ${SYSROOT}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
