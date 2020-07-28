# Options common to all projects.

if(BUILD_X64)
  set(ARCH_BIT -m64 )
  set(ASM_BIT  -m64 )
else()
  if (TARGET_CPU STREQUAL "Atom")
    # architecture will be according to ATOM
    set(ARCH_BIT -m32 )
    set(ASM_BIT  -m32 )
  else ()
    # need to force a more modern architecture than the degault m32 (i386).
    set(ARCH_BIT "-m32 -march=core2" )
    set(ASM_BIT  "-Wa,--32,-march=core2" )
  endif (TARGET_CPU STREQUAL "Atom")
endif()

macro(ocl_add_flags_if_supported)
  foreach(flag ${ARGN})
    string(REGEX REPLACE "^-" "" flag_name ${flag})
    string(REPLACE "-" "_" flag_name ${flag_name})
    string(TOUPPER ${flag_name} flag_name)
    add_flag_if_supported(${flag} "${flag_name}")
  endforeach()
endmacro()

# Warning level
ocl_add_flags_if_supported(
  -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-strict-aliasing
  -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter -Wno-deprecated-declarations -Werror
  -Wno-unused-function -Wno-implicit-fallthrough
  -Wno-int-to-pointer-cast -Wno-unused-but-set-variable)

# Compiler switches that CANNOT be modified during makefile generation
set(FSTACK_PROTECTOR_STRONG_FLAG -fstack-protector-strong)
set(FSTACK_PROTECTOR_FLAG -fstack-protector)
check_c_compiler_flag("-Werror ${FSTACK_PROTECTOR_STRONG_FLAG}" SUPPORTS_FSTACK_PROTECTOR_STRONG_FLAG)
if (SUPPORTS_FSTACK_PROTECTOR_STRONG_FLAG)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FSTACK_PROTECTOR_STRONG_FLAG}")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FSTACK_PROTECTOR_STRONG_FLAG}")
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FSTACK_PROTECTOR_FLAG}")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FSTACK_PROTECTOR_FLAG}")
endif()

set (ADD_COMMON_C_FLAGS         "-msse3 -mssse3 ${SSE4_VAL} ${ARCH_BIT} -fPIC -fdiagnostics-show-option -funsigned-bitfields -Wformat -Wformat-security" )

set (ADD_C_FLAGS                "${ADD_COMMON_C_FLAGS}" )
set (ADD_C_FLAGS_DEBUG          "-O0 -ggdb3 -D _DEBUG" )

set (ADD_CXX_FLAGS              "${ADD_COMMON_C_FLAGS}" )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set (ADD_C_FLAGS_RELEASE        "-O2 -U _DEBUG")
else()
  set (ADD_C_FLAGS_RELEASE        "-O2 -U _DEBUG -Wno-error")
endif()

set (ADD_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -U _DEBUG")

# Linker switches
set (ADD_CMAKE_EXE_LINKER_FLAGS          "-z noexecstack -z relro -z now" )

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
set( CMAKE_ASM_FLAGS                        "${CMAKE_ASM_FLAGS}                        ${ASM_BIT}")
set( CMAKE_ASM_INCLUDE_DIR_FLAG             -I )
set( CMAKE_ASM_OUTPUT_NAME_FLAG             -o )
# We use GCC as an assembler on Linux, so we have to specify '-c'
# to the custom command that compiles the assembly sources.
set( CMAKE_ASM_COMPILE_TO_OBJ_FLAG          -c )

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS                 "${CMAKE_EXE_LINKER_FLAGS}                ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_DEBUG           "${CMAKE_EXE_LINKER_FLAGS_DEBUG}          ${ADD_LINKER_FLAGS_DEBUG}           ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE         "${CMAKE_EXE_LINKER_FLAGS_RELEASE}        ${ADD_LINKER_FLAGS_RELEASE}         ${ADD_CMAKE_EXE_LINKER_FLAGS}")
set( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${ADD_LINKER_FLAGS_RELWITHDEBINFO}  ${ADD_CMAKE_EXE_LINKER_FLAGS}")

# Linker switches - DLL
set( CMAKE_SHARED_LINKER_FLAGS              "${CMAKE_EXE_LINKER_FLAGS}                ${ADD_CMAKE_EXE_LINKER_FLAGS}")
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
