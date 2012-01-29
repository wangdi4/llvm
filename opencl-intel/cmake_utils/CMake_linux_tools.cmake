# binary output directory suffix

# note - the search is done only when the CMake cache is built
find_program( IWHICH_FOUND iwhich NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH )

option(OCL_BUILD32 "Are we building for 32bit? Default is no, i.e. 64 bit (or native)." OFF)

set(OS Lin )
set(IMPLIB_SUBDIR bin )
set(IMPLIB_PREFIX lib )
set(IMPLIB_SUFFIX .so )

# C/CXX Compiler and cross-compilation flags
# defined by toolchain
# Linux-Intel.cmake and Linux-GNU.cmake
include("${OCL_TOOLCHAIN_FILE}")

if (TARGET_CPU STREQUAL "Atom")
    set(CMAKE_EXE_LINKER_FLAGS        "${CMAKE_EXE_LINKER_FLAGS}    ${ATOM_SPECIFIC_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS     "${CMAKE_SHARED_LINKER_FLAGS} ${ATOM_SPECIFIC_FLAGS}")
    set(CMAKE_CXX_FLAGS               "${CMAKE_CXX_FLAGS} ${ATOM_SPECIFIC_FLAGS}")
    set(CMAKE_C_FLAGS                 "${CMAKE_C_FLAGS}   ${ATOM_SPECIFIC_FLAGS}")
    set(CMAKE_ASM_FLAGS               ${CMAKE_ASM_FLAGS} ${ATOM_SPECIFIC_FLAGS_ASM}) # Do not quote (CREATE_ASM_RULES)!
endif (TARGET_CPU STREQUAL "Atom")

message("** ** ** Enable Languages ** ** **")

enable_language( C )
enable_language( CXX )
enable_language( ASM )


# The sysroot command is different for each compiler, so just set it now - after the toolchain is defined.
if (OCL_MEEGO)
    set(BIN_OUTPUT_DIR_SUFFIX "lin") # Problems using Meego suffix, since it is invalid.
    set(CMAKE_EXE_LINKER_FLAGS        "${SET_XCOMPILE_SYSROOT_CMD} ${CMAKE_EXE_LINKER_FLAGS}" )
    set(CMAKE_SHARED_LINKER_FLAGS     "${SET_XCOMPILE_SYSROOT_CMD} ${CMAKE_SHARED_LINKER_FLAGS}" )
    set(CMAKE_CXX_FLAGS               "${SET_XCOMPILE_SYSROOT_CMD} ${CMAKE_CXX_FLAGS}" )
    set(CMAKE_C_FLAGS                 "${SET_XCOMPILE_SYSROOT_CMD} ${CMAKE_C_FLAGS}" )
    set(ADD_CMAKE_EXE_LINKER_FLAGS    "${SET_XCOMPILE_SYSROOT_CMD} ${ATOM_SPECIFIC_FLAGS}")
else()
    set(BIN_OUTPUT_DIR_SUFFIX "lin")
endif(OCL_MEEGO)


if (OCL_BUILD32)
    set(BIN_OUTPUT_DIR_SUFFIX "${BIN_OUTPUT_DIR_SUFFIX}32" )
    add_definitions(-D OCL_BUILD32)
    if (TARGET_CPU STREQUAL "Atom")
        # architecture will be according to ATOM
        set(ARCH_BIT -m32 )
        set(ASM_BIT  --32 )
    else ()
        # need to force a more modern architecture than the degault m32 (i386).
        set(ARCH_BIT "-m32 -march=core2" )
        set(ASM_BIT  --32 -march=core2 )
    endif (TARGET_CPU STREQUAL "Atom")
else()
    set(BIN_OUTPUT_DIR_SUFFIX "${BIN_OUTPUT_DIR_SUFFIX}64" )
    set(ARCH_BIT -m64 )
    set(ASM_BIT  --64 )
endif (OCL_BUILD32)

# set CMAKE SVN client to be first SVN client in the PATH
execute_process(COMMAND which svn OUTPUT_VARIABLE Subversion_SVN_EXECUTABLE OUTPUT_STRIP_TRAILING_WHITESPACE )	

if (NOT "${IWHICH_FOUND}" STREQUAL IWHICH_FOUND-NOTFOUND)
	execute_process(COMMAND ${IWHICH_FOUND}  ${CMAKE_C_COMPILER} OUTPUT_VARIABLE INTEL_IT_COMPILER_FOUND OUTPUT_STRIP_TRAILING_WHITESPACE )
	if (NOT "${INTEL_IT_COMPILER_FOUND}" STREQUAL "")		
		set( INTEL_IT_BUILD_ENV_FOUND ON )
	endif (NOT "${INTEL_IT_COMPILER_FOUND}" STREQUAL "")
endif (NOT "${IWHICH_FOUND}" STREQUAL IWHICH_FOUND-NOTFOUND)

if (DEFINED INTEL_IT_BUILD_ENV_FOUND)
    # setup Intel IT tools versions database	
    set( ENV{USER_ITOOLS} ${CMAKE_SOURCE_DIR}/cmake_utils/intel_it_linux_tool_versions.txt )

    # find required compiler setup    	
    execute_process(COMMAND ${IWHICH_FOUND} ${CMAKE_C_COMPILER} OUTPUT_VARIABLE INTEL_IT_COMPILER_PATH OUTPUT_STRIP_TRAILING_WHITESPACE )	
    string( REPLACE /bin/${CMAKE_C_COMPILER} "" INTEL_IT_COMPILER_PATH  ${INTEL_IT_COMPILER_PATH} )

    # prepend all cmake paths intel IT path
    set( CMAKE_PREFIX_PATH  /usr/intel ${INTEL_IT_COMPILER_PATH})
endif (DEFINED INTEL_IT_BUILD_ENV_FOUND)

# Warning level
set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter" )

# Compiler switches that CANNOT be modified during makefile generation

set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 ${SSE4_VAL} ${ARCH_BIT} -fPIC -fdiagnostics-show-option -funsigned-bitfields" )

set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99" )
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS}" )

set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb3 -D _DEBUG" )
set (ADD_C_FLAGS_RELEASE "-O2 -ggdb2 -U _DEBUG")
set (ADD_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -U _DEBUG")

# Compiler switches that CAN be modified during makefile generation and configuration-independent
add_definitions( ${WARNING_LEVEL} )
# gcc 4.1 complains about missing virtual dtor for interface classes. this is bad and wrong, we should supress it.
add_definitions(${VIRTUAL_DISTRUCTOR_WARNING})

# Linker switches
set (INIT_LINKER_FLAGS        "-Wl,--enable-new-dtags" ) # --enable-new-dtags sets RUNPATH to the same value as RPATH
set (ADD_LINKER_FLAGS_DEBUG        )
set (ADD_LINKER_FLAGS_RELEASE "-s ")

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
if (BUILD_FOR_COMPILATION_PURPOSES_ONLY)
    SET(CMAKE_SKIP_BUILD_RPATH                  FALSE )   # add pointers to the build tree, so it can be used during compilation
else ()
    SET(CMAKE_SKIP_BUILD_RPATH                  TRUE )   # do not add pointers to the build tree
endif (BUILD_FOR_COMPILATION_PURPOSES_ONLY)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH          TRUE )   # build rpath as if already installed
SET(CMAKE_INSTALL_RPATH                     "$ORIGIN" ) # the rpath to use - search through installation dir only
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH       FALSE)  # do not use static link paths as rpath

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

# ASM switches
set( CMAKE_ASM_FLAGS               ${CMAKE_ASM_FLAGS} ${ASM_BIT} ) # do not quote, because of CREATE_ASM_RULES
set( CMAKE_ASM_INCLUDE_DIR_FLAG    -I )
set( CMAKE_ASM_OUTPUT_NAME_FLAG    -o )

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS           "${INIT_LINKER_FLAGS} ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG} ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE} ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO   "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${ADD_LINKER_FLAGS_RELWITHDEBINFO} ${ADD_CMAKE_EXE_LINKER_FLAGS}")

# Linker switches - DLL
set( CMAKE_SHARED_LINKER_FLAGS         "${INIT_LINKER_FLAGS} ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG} ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE} ${ADD_CMAKE_EXE_LINKER_FLAGS}")

set( CMAKE_BUILD_TOOL "$(MAKE)")

message("\n\n** ** ** COMPILER Definitions ** ** **")
message("CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
message("CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
message("")
message("CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message("")
message("CMAKE_ASM_COMPILER = ${CMAKE_ASM_COMPILER}")
message("CMAKE_ASM_FLAGS = ${CMAKE_ASM_FLAGS}")
message("")
message("CMAKE_FIND_ROOT_PATH = ${CMAKE_FIND_ROOT_PATH}")
message("CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
message("")
message("CMAKE_BUILD_TOOL = ${CMAKE_BUILD_TOOL}")


