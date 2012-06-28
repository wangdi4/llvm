cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

set (TARGET_NAME "NEAT")
set (TARGET_NAME_WRAPPER "NEAT_wrapper")

if (WIN32)

project( ${TARGET_NAME_WRAPPER} )

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bit
  set (PLATFORM 32)
else ()
  # 64 bit
  set (PLATFORM 64)
endif ()

  set (PLATFORM Win${PLATFORM})


# icc executables and env scripts
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (ICC_PLATFORM ia32)
else ()
  set (ICC_PLATFORM intel64)
endif ()

  set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})
  if (NOT DEFINED ICC_WIN32_PATH)
    message (FATAL_ERROR "ICC is not installed in the system")
  endif ()

  set (ICC_CPU_ENV_SCRIPT ${ICC_WIN32_PATH}/bin/iclvars.bat)
  if (NOT EXISTS ${ICC_CPU_ENV_SCRIPT})
    set (ICC_CPU_ENV_SCRIPT ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/iclvars_${ICC_PLATFORM}.bat)
  endif()
  set (ICC_CPU_CL ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/icl.exe)
  set (ICC_CPU_LD ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/xilib.exe)



  set (ICC_CPU_COMPILER_SCRIPT compile_win.bat)
  set (ICC_CPU_LINKER_SCRIPT   link_win.bat)

# remember NEAT source dir
set (NEAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (" ${TARGET_NAME} sources are in: ${NEAT_SOURCE_DIR}")

set(LLVM_TOOLS_LIB_DIR ${LLVM_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR})

# icc

  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Debug "/Qvc10 /Od /D_DEBUG /RTC1 /MDd")
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Release "/Qvc10 /Od /DNDEBUG /MD /Gy")

#
# Compile single C source file
#
# compile_c( OUTPUT_FILE_VAR INPUT_FILE OUTPUT_DIR )
#
#   COMPILER_SCRIPT - the compilation script which will be used
#   OUTPUT_FILE_VAR - output, contains full path to object file
#   INPUT_FILE      - full path to input file
#   OUTPUT_DIR      - directory for output file
#

function( compile_c COMPILER_SCRIPT OUTPUT_FILE_VAR INPUT_FILE OUTPUT_DIR )

  get_filename_component (NAMEF ${INPUT_FILE} NAME_WE )
  set (OUTPUT_FILE ${NAMEF}${CMAKE_C_OUTPUT_EXTENSION} )

  add_custom_command (
    OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
    COMMAND ${COMPILER_SCRIPT} $<CONFIGURATION> ${INPUT_FILE} ${OUTPUT_DIR}/${OUTPUT_FILE}
    MAIN_DEPENDENCY ${INPUT_FILE}
    DEPENDS ${COMPILER_SCRIPT} ${HEADER_FILES}
    IMPLICIT_DEPENDS CXX ${INPUT_FILE}
    WORKING_DIRECTORY ${NEAT_SOURCE_DIR}
    COMMENT "C Compiling ${INPUT_FILE}"
    VERBATIM
    )

  set (${OUTPUT_FILE_VAR} ${OUTPUT_DIR}/${OUTPUT_FILE} PARENT_SCOPE )

endfunction( compile_c )

#
# add C library built from set of C source files
#
# add_c_library( OUTPUT_FILE_VAR TARGET_NAME ARCH CPP_DEFINE OUTPUT_DIR ...FILES... )
#
#   LINKER_SCRIPT   - the linking script which will be used
#   COMPILER_SCRIPT - the compilation script which will be used
#   OUTPUT_FILE_VAR - output, contains generated library
#   TARGET_NAME     - name of the target and base name of the lib. Extension is CMAKE_SHARED_LIBRARY_SUFFIX
#   OUTPUT_DIR      - output directory for the intermediate files
#   FILES           - input files list
#

function( add_c_library LINKER_SCRIPT COMPILER_SCRIPT OUTPUT_FILE_VAR TARGET_NAME OUTPUT_DIR)

  set (OUTPUT_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})

  # Compile all files and prepare a 'TMP_FILES' list of object files
  foreach (FILE ${ARGN})
    compile_c( ${COMPILER_SCRIPT} TMP_OBJ ${FILE} ${OUTPUT_DIR})
    set (TMP_FILES "${TMP_FILES} ${TMP_OBJ}")
    list (APPEND TMP_FILES_DEPEND ${TMP_OBJ})
  endforeach (FILE)


  # link C library
    add_custom_command (
      OUTPUT ${LLVM_TOOLS_LIB_DIR}/${OUTPUT_FILE}
      COMMAND ${LINKER_SCRIPT} $<CONFIGURATION> ${LLVM_TOOLS_LIB_DIR}/${OUTPUT_FILE} ${TMP_FILES}
      DEPENDS ${LINKER_SCRIPT} ${TMP_FILES_DEPEND}
      COMMENT "C Linking ${OUTPUT_FILE}"
      VERBATIM
      )

    set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE)

endfunction( add_c_library )


#------------------ Main -----------------------
# C source code
list (APPEND C_SOURCE_FILES
NEATALU.cpp
NEATValue.cpp
NEATVector.cpp
NEAT_WRAP.cpp
)

 # project header files
list (APPEND HEADER_FILES
NEATALU.h
NEATValue.h
NEATVector.h
NEAT_WRAP.h
)
set (SAVE_C_SOURCE_FILES ${C_SOURCE_FILES})
unset (C_SOURCE_FILES)
foreach (FILE ${SAVE_C_SOURCE_FILES})
  list (APPEND C_SOURCE_FILES ${NEAT_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_HEADER_FILES ${HEADER_FILES})
unset (HEADER_FILES)
foreach (FILE ${SAVE_HEADER_FILES})
  list (APPEND HEADER_FILES ${NEAT_SOURCE_DIR}/${FILE})
endforeach (FILE)

# conversion of filepaths used during configuration of ICC compiler and linker scripts to native format (as they will be directly used by these scripts)
file (TO_NATIVE_PATH ${ICC_CPU_ENV_SCRIPT} ICC_CPU_ENV_SCRIPT_NATIVE)
file (TO_NATIVE_PATH ${ICC_CPU_CL}         ICC_CPU_CL_NATIVE)
file (TO_NATIVE_PATH ${ICC_CPU_LD}         ICC_CPU_LD_NATIVE)

# configuring ICC compiler and linker scripts (the configured scripts are in ${CMAKE_CURRENT_BINARY_DIR})
configure_file (${ICC_CPU_COMPILER_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_COMPILER_SCRIPT} @ONLY)
configure_file (${ICC_CPU_LINKER_SCRIPT}   ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_LINKER_SCRIPT} @ONLY)

# redefine ICC compiler and linker scripts to their new locations
set (ICC_CPU_COMPILER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_COMPILER_SCRIPT})
set (ICC_CPU_LINKER_SCRIPT   ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_LINKER_SCRIPT})

# intermediate directory for OpenCL and ICC compilation/link
# note that platform is already accounted by ${CMAKE_CURRENT_BINARY_DIR}

    set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})


add_c_library( ${ICC_CPU_LINKER_SCRIPT} ${ICC_CPU_COMPILER_SCRIPT} OUTPUT_C_LIB ${TARGET_NAME} ${INTERMEDIATE_OUTPUT_DIR} ${C_SOURCE_FILES})

# top-level target for C target
add_custom_target (${TARGET_NAME_WRAPPER} ALL
  DEPENDS ${LLVM_TOOLS_LIB_DIR}/${OUTPUT_C_LIB}
  COMMENT "Target ${TARGET_NAME} build completed"
  VERBATIM
  SOURCES ${C_SOURCE_FILES} ${HEADER_FILES}
  )

add_dependencies(${TARGET_NAME_WRAPPER} ImathLibd)

set_target_properties( ${TARGET_NAME_WRAPPER} PROPERTIES lib_dir  ${LLVM_TOOLS_LIB_DIR} )

else(WIN32)
include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
        ../REFALU
        ../ImathLibd
    )
set(LLVM_REQUIRES_EH 1)
set(LLVM_REQUIRES_EH 1)

add_llvm_library(${TARGET_NAME} STATIC
NEATALU.cpp
NEATValue.cpp
NEATVector.cpp
NEAT_WRAP.cpp
)

add_dependencies(${TARGET_NAME} REFALU)
target_link_libraries(${TARGET_NAME} REFALU)
endif(WIN32)



