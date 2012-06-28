cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

set (TARGET_NAME "ImathLibd")

# icc executables and env scripts
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (ICC_PLATFORM ia32)
else ()
  set (ICC_PLATFORM intel64)
endif ()

if (WIN32)

project( ${TARGET_NAME} )

  set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})
  if (NOT DEFINED ICC_WIN32_PATH)
    message (FATAL_ERROR "ICC is not installed in the system")
  endif ()
  set (ICC_CPU_ENV_SCRIPT ${ICC_WIN32_PATH}/bin/iclvars.bat)
  if (NOT EXISTS ${ICC_CPU_ENV_SCRIPT})
    set (ICC_CPU_ENV_SCRIPT ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/iclvars_${ICC_PLATFORM}.bat)
  endif()
  set (ICC_CPU_CL ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/icl.exe)
  set (ICC_CPU_LD ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/xilink.exe)


  set (ICC_CPU_COMPILER_SCRIPT compile_win.bat)
  set (ICC_CPU_LINKER_SCRIPT   link_win.bat)


# icc
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Debug "/Od /D_DEBUG /RTC1 /MDd")
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Release "/Od /Oi /Qipo /DNDEBUG /MD /Gy")
  
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "/MACHINE:X86 /DEBUG")
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "/MACHINE:X86 /DEBUG /OPT:REF /OPT:ICF")  
  else ()
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "/MACHINE:X64 /DEBUG")
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "/MACHINE:X64 /DEBUG /OPT:REF /OPT:ICF")    
  endif ()

else (WIN32)

 # search for the icc cpu compiler
  set (ICC_CPU_CL icpc)
  set (ICC_CPU_LD icpc)

  find_program (ICC_CPU_COMPILER_PROGRAM ${ICC_CPU_CL} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
  if (${ICC_CPU_COMPILER_PROGRAM} STREQUAL "ICC_CPU_COMPILER_PROGRAM-NOTFOUND")
    message (FATAL_ERROR "ICC(CPU) ${ICC_CPU_COMPILER_PROGRAM} is not installed in the system")
  endif()

  # search for the compilervars script (intel compiler 12)
  set (ICC_COMPILER_VARS compilervars.sh)
  string(REPLACE "${ICC_PLATFORM}/${ICC_CPU_CL}" ${ICC_COMPILER_VARS} ICC_CPU_ENV_SCRIPT "${ICC_CPU_COMPILER_PROGRAM}")
  if(EXISTS ${ICC_CPU_ENV_SCRIPT})
    message ("ICC(CPU) ${ICC_CPU_ENV_SCRIPT} exists")
  else()
    message ("ICC(CPU) ${ICC_COMPILER_VARS} is not installed in the system")
    # search for the iccvars script (intel compiler 11)
    set (ICC_CPU_ENV_SCRIPT iccvars.sh)
	message ("ICC(CPU) looking for ${ICC_CPU_ENV_SCRIPT}")
    find_program (ICC_CPU_COMPILER_SCRIPT ${ICC_CPU_ENV_SCRIPT} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
    if (${ICC_CPU_COMPILER_SCRIPT} STREQUAL "ICC_CPU_COMPILER_SCRIPT-NOTFOUND")
      set (ICC_CPU_ENV_SCRIPT iccvars_${ICC_PLATFORM}.sh)
    endif()
    find_program (ICC_CPU_COMPILER_SCRIPT ${ICC_CPU_ENV_SCRIPT} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
    if (${ICC_CPU_COMPILER_SCRIPT} STREQUAL "ICC_CPU_COMPILER_SCRIPT-NOTFOUND")
      message (FATAL_ERROR "ICC(CPU) (${ICC_CPU_ENV_SCRIPT}) is not installed in the system")
    endif()
    set(ICC_CPU_ENV_SCRIPT ${ICC_CPU_COMPILER_SCRIPT})
  endif()
  
  message ( "ICC(CPU) ${ICC_CPU_ENV_SCRIPT}")
  message ("ICC CPU Compiler: ${ICC_CPU_CL}")
  message ("ICC CPU Linker: ${ICC_CPU_LD}")
  
  set (ICC_CPU_COMPILER_SCRIPT compile_linux.sh)
  set (ICC_CPU_LINKER_SCRIPT   link_linux.sh)
  
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Debug "-g -O0")
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Release "-g -O0")

  set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "-g")
  set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "-g")
  
endif (WIN32)
 
# remember imathLibd source dir
set (IMATHLIBD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (" ${TARGET_NAME} sources are in: ${IMATHLIBD_SOURCE_DIR}")

# target directory for .dll/.so 
if(WIN32)
    set (IMATHLIBD_INSTALL_DIR ${LLVM_TOOLS_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    set(LLVM_TOOLS_LIB_DIR ${LLVM_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR})	
else(WIN32)
    set (IMATHLIBD_INSTALL_DIR ${LLVM_BINARY_DIR}/lib)
    set(LLVM_TOOLS_LIB_DIR ${LLVM_BINARY_DIR}/lib)	
endif(WIN32)
message ("IMATHLIBD library will be in: ${IMATHLIBD_INSTALL_DIR}")

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
    WORKING_DIRECTORY ${IMATHLIBD_SOURCE_DIR}
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

  set (OUTPUT_FILE ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set (IMPLIB_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})

  # PDB file will be generated for Win32
  set (PDB_FILE ${TARGET_NAME}.pdb)

  # Compile all files and prepare a 'TMP_FILES' list of object files
  foreach (FILE ${ARGN})   
    compile_c( ${COMPILER_SCRIPT} TMP_OBJ ${FILE} ${OUTPUT_DIR})
    set (TMP_FILES "${TMP_FILES} ${TMP_OBJ}")
    list (APPEND TMP_FILES_DEPEND ${TMP_OBJ})
  endforeach (FILE)

  # link C library 
if(WIN32)
    add_custom_command (
	  OUTPUT ${IMATHLIBD_INSTALL_DIR}/${OUTPUT_FILE}
      COMMAND ${LINKER_SCRIPT} $<CONFIGURATION> ${IMATHLIBD_INSTALL_DIR}/${OUTPUT_FILE} ${IMATHLIBD_INSTALL_DIR}/${PDB_FILE} ${LLVM_TOOLS_LIB_DIR}/${IMPLIB_FILE} ${TMP_FILES}
      # copy dll to folder which is not depends on $<CONFIGURATION>, i.e. Debug or Release
	  # becasue install command doesn't support using $<CONFIGURATION> or CMAKE_CFG_INTDIR
	  COMMAND ${CMAKE_COMMAND} -E copy "${IMATHLIBD_INSTALL_DIR}/${OUTPUT_FILE}" ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${LINKER_SCRIPT} ${TMP_FILES_DEPEND}
      COMMENT "C Linking ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
      VERBATIM
      )
else()
    add_custom_command (
	  OUTPUT ${IMATHLIBD_INSTALL_DIR}/${OUTPUT_FILE}
      COMMAND ${LINKER_SCRIPT} $<CONFIGURATION> ${IMATHLIBD_INSTALL_DIR}/${OUTPUT_FILE} ${IMATHLIBD_INSTALL_DIR}/${PDB_FILE} ${LLVM_TOOLS_LIB_DIR}/${IMPLIB_FILE} ${TMP_FILES}
      DEPENDS ${LINKER_SCRIPT} ${TMP_FILES_DEPEND}
      COMMENT "C Linking ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
      VERBATIM
      )
endif()
  
    set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE)

endfunction( add_c_library )


#------------------ Main -----------------------
# C source code
list (APPEND C_SOURCE_FILES
  dllmain.cpp
  imathLibd.cpp
  stdafx.cpp
  )
  
 # project header files
list (APPEND HEADER_FILES
  stdafx.h
  targetver.h
  imathLibd.h
  )

set (SAVE_C_SOURCE_FILES ${C_SOURCE_FILES})
unset (C_SOURCE_FILES)
foreach (FILE ${SAVE_C_SOURCE_FILES})
  list (APPEND C_SOURCE_FILES ${IMATHLIBD_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_HEADER_FILES ${HEADER_FILES})
unset (HEADER_FILES)
foreach (FILE ${SAVE_HEADER_FILES})
  list (APPEND HEADER_FILES ${IMATHLIBD_SOURCE_DIR}/${FILE})
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
if(WIN32)  
    set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
else(WIN32)
    set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif(WIN32)
 
  # build DLL/.so file out of C code (by ICC) and copy it to install directory
  add_c_library( ${ICC_CPU_LINKER_SCRIPT} ${ICC_CPU_COMPILER_SCRIPT} OUTPUT_C_LIB ${TARGET_NAME} ${INTERMEDIATE_OUTPUT_DIR} ${C_SOURCE_FILES})


set (TARGET_NAME_WRAPPER "ImathLibd_wrapper")

  # top-level target for C target
  add_custom_target (${TARGET_NAME_WRAPPER} ALL
    DEPENDS ${IMATHLIBD_INSTALL_DIR}/${OUTPUT_C_LIB}
    COMMENT "Target ${TARGET_NAME} build completed"
    VERBATIM
    SOURCES ${C_SOURCE_FILES} ${HEADER_FILES}
    )

if(WIN32)
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_C_LIB} DESTINATION bin)
else(WIN32)
    install( FILES ${LLVM_TOOLS_LIB_DIR}/${OUTPUT_C_LIB} DESTINATION bin)
    install( FILES ${LLVM_TOOLS_LIB_DIR}/${OUTPUT_C_LIB} DESTINATION ${LLVM_TOOLS_BINARY_DIR})
endif(WIN32)	
    set_target_properties( ${TARGET_NAME_WRAPPER} PROPERTIES lib_dir  ${LLVM_TOOLS_LIB_DIR} ) 
