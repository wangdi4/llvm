# Options common to all projects.
message(STATUS "** ** ** Enable Languages ** ** **")

enable_language( C )
enable_language( CXX )
enable_language( ASM )

if(BUILD_X64)
  set(ARCH_BIT -m64 )
  set(ASM_BIT  "-Xassembler --64" )
else()
  if (TARGET_CPU STREQUAL "Atom")
    # architecture will be according to ATOM
    set(ARCH_BIT -m32 )
    set(ASM_BIT  "-Xassembler --32" )
  else ()
    # need to force a more modern architecture than the degault m32 (i386).
    set(ARCH_BIT "-m32 -march=core2" )
    set(ASM_BIT  "-Wa,--32,-march=core2" )
  endif (TARGET_CPU STREQUAL "Atom")
endif()

# Warning level
add_definitions(-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter -Wno-deprecated-declarations)
# Additional warning switches for newer GCC.
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
  add_definitions(-Wno-int-to-pointer-cast -Wno-unused-but-set-variable)
endif ()

# Compiler switches that CANNOT be modified during makefile generation
if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
  set (FSTACK_PROTECTOR_C_FLAGS       "-fstack-protector-strong" )
else (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
  set (FSTACK_PROTECTOR_C_FLAGS       "-fstack-protector" )
endif ()

set (ADD_COMMON_C_FLAGS         "-msse3 -mssse3 ${SSE4_VAL} ${ARCH_BIT} -fPIC -fdiagnostics-show-option -funsigned-bitfields -Wformat -Wformat-security ${FSTACK_PROTECTOR_C_FLAGS}" )

set (ADD_C_FLAGS                "${ADD_COMMON_C_FLAGS} -std=gnu99" )
set (ADD_C_FLAGS_DEBUG          "-O0 -ggdb3 -D _DEBUG" )
set (ADD_C_FLAGS_RELEASE        "-O2 -ggdb2 -U _DEBUG")
set (ADD_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -U _DEBUG")

set (ADD_CXX_FLAGS              "${ADD_COMMON_C_FLAGS}" )
if (DEFINED UBUNTU64_BUILD)
  set (ADD_CXX_FLAGS            "${ADD_COMMON_C_FLAGS}  -Wno-conversion-null -Wno-unused-result -Wno-unused-local-typedefs")
endif()

# Linker switches
set (INIT_LINKER_FLAGS          "-Wl,--enable-new-dtags -z noexecstack -z relro -z now" ) # --enable-new-dtags sets RUNPATH to the same value as RPATH

# embed RPATH and RUNPATH to the binaries that assumes that everything is installed in the same directory
#
# Description:
#   RPATH is used to locate dynamically load shared libraries/objects (DLLs) for the non-standard OS
#   locations without need of relinking DLLs during installation. The algorithm is the following:
#
#   1. If RPATH is present in the EXE/DLL and RUNPATH is NOT present, search through it.
#   2. If LD_LIBRARY_PATH env variable is present, search through it
#   3. If RUNPATH is present in the EXE/DLL, search through it
#   4. Search through locations, configured by system admin and cached in /etc/ld.so.cache
#   5. Search through /lib and /usr/lib
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
  SET(CMAKE_SKIP_BUILD_RPATH          FALSE )  # add pointers to the build tree, so it can be used during compilation
else ()
  SET(CMAKE_SKIP_BUILD_RPATH          TRUE )   # do not add pointers to the build tree
endif (BUILD_FOR_COMPILATION_PURPOSES_ONLY)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH    TRUE )   # build rpath as if already installed
SET(CMAKE_INSTALL_RPATH               "$ORIGIN::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" ) # the rpath to use - search through installation dir only
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)   # do not use static link paths as rpath

# C switches
set( CMAKE_C_FLAGS                          "${CMAKE_C_FLAGS}                         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG                    "${CMAKE_C_FLAGS_DEBUG}                   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}                 ${ADD_C_FLAGS_RELEASE}")
set( CMAKE_C_FLAGS_RELWITHDEBINFO           "${CMAKE_C_FLAGS_RELWITHDEBINFO}          ${ADD_C_FLAGS_RELWITHDEBINFO}")

# C++ switches
set( CMAKE_CXX_FLAGS                        "${CMAKE_CXX_FLAGS}                       ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG                  "${CMAKE_CXX_FLAGS_DEBUG}                 ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE}               ${ADD_C_FLAGS_RELEASE}")
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}        ${ADD_CXX_FLAGS_RELWITHDEBINFO}")

# ASM switches
set( CMAKE_ASM_FLAGS                        ${CMAKE_ASM_FLAGS}                        ${ASM_BIT} )
set( CMAKE_ASM_INCLUDE_DIR_FLAG             -I )
set( CMAKE_ASM_OUTPUT_NAME_FLAG             -o )

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS                 "${INIT_LINKER_FLAGS}                     ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_DEBUG           "${CMAKE_EXE_LINKER_FLAGS_DEBUG}          ${ADD_LINKER_FLAGS_DEBUG}           ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE         "${CMAKE_EXE_LINKER_FLAGS_RELEASE}        ${ADD_LINKER_FLAGS_RELEASE}         ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${ADD_LINKER_FLAGS_RELWITHDEBINFO}  ${ADD_CMAKE_EXE_LINKER_FLAGS}")

# Linker switches - DLL
set( CMAKE_SHARED_LINKER_FLAGS              "${INIT_LINKER_FLAGS}                     ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_SHARED_LINKER_FLAGS_DEBUG        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}       ${ADD_LINKER_FLAGS_DEBUG}           ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE      "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}     ${ADD_LINKER_FLAGS_RELEASE}         ${ADD_CMAKE_EXE_LINKER_FLAGS}")

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

