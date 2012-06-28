cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

set (TARGET_NAME "ValidationTests")
if (WIN32)
project( ${TARGET_NAME} )

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bit
  set (PLATFORM 32)
else ()
  # 64 bit
  set (PLATFORM 64)
endif ()

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
  set (ICC_CPU_LD ${ICC_WIN32_PATH}/bin/${ICC_PLATFORM}/xilink.exe)

  set (ICC_CPU_COMPILER_SCRIPT compile_win.bat)
  set (ICC_CPU_LINKER_SCRIPT   link_win.bat)

  set (VS100_WIN32_PATH $ENV{VS100COMNTOOLS})

  if (NOT DEFINED VS100_WIN32_PATH)
    message (FATAL_ERROR "VS100 is not installed in the system")
  endif ()  
  
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (VS100_CPU_ENV_SCRIPT ${VS100_WIN32_PATH}/../../VC/bin/vcvars32.bat)
  set (VS100_CPU_CL ${VS100_WIN32_PATH}/../../VC/bin/cl.exe)
else ()
  set (VS100_CPU_ENV_SCRIPT ${VS100_WIN32_PATH}/../../VC/bin/amd64/vcvars64.bat)
  set (VS100_CPU_CL ${VS100_WIN32_PATH}/../../VC/bin/amd64/cl.exe)
endif ()

  set (VS100_CPU_COMPILER_SCRIPT compile_win_vs100.bat)

# remember ValidationTests source dir
set (TARGETEXE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (" ${TARGET_NAME} sources are in: ${TARGETEXE_SOURCE_DIR}")


# target directory for exe cutable
set (TARGETEXE_INSTALL_DIR ${LLVM_TOOLS_BINARY_DIR}/${CMAKE_CFG_INTDIR})
message ("${TARGET_NAME} executable will be in: ${TARGETEXE_INSTALL_DIR}")

set(LLVM_TOOLS_LIB_DIR ${LLVM_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR})

# icc
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Debug "/Qvc10 /Od /D_DEBUG /RTC1 /MDd")
  set (BUILD_TYPE_ICC_COMPILER_FLAGS_Release "/Qvc10 /Od /DNDEBUG /MD /Gy")

  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "/MACHINE:X86 /DEBUG")
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "/MACHINE:X86 /DEBUG /OPT:REF /OPT:ICF")  
  else ()
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Debug "/MACHINE:X64 /DEBUG")
    set (BUILD_TYPE_ICC_LINKER_FLAGS_Release "/MACHINE:X64 /DEBUG /OPT:REF /OPT:ICF")    
  endif ()

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
    WORKING_DIRECTORY ${TARGETEXE_SOURCE_DIR}
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

function( add_c_exe LINKER_SCRIPT COMPILER_SCRIPT_1 COMPILER_SCRIPT_2 OUTPUT_FILE_VAR TARGET_NAME OUTPUT_DIR)

  set (OUTPUT_FILE ${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX})
  set (IMPLIB_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})

  # PDB file will be generated for Win32
  set (PDB_FILE ${TARGET_NAME}.pdb)

  # Compile all files and prepare a 'TMP_FILES' list of object files
  foreach (FILE ${ARGN})
    compile_c( ${COMPILER_SCRIPT_1} TMP_OBJ ${FILE} ${OUTPUT_DIR})
    set (TMP_FILES "${TMP_FILES} ${TMP_OBJ}")
    list (APPEND TMP_FILES_DEPEND ${TMP_OBJ})
  endforeach (FILE)

# some files can not be compiled by Intel compiler, because ICC crashes with internal error,
# so we have to compile them by Microsoft and than link together with files compiled by ICC
list (APPEND C_SOURCE_FILES_2
  SourceRecorder.cpp
  ClangSA.cpp
)  

  foreach (FILE ${C_SOURCE_FILES_2})
    compile_c( ${COMPILER_SCRIPT_2} TMP_OBJ ${FILE} ${OUTPUT_DIR})
    set (TMP_FILES "${TMP_FILES} ${TMP_OBJ}")
    list (APPEND TMP_FILES_DEPEND ${TMP_OBJ})

  endforeach (FILE) 

  # link executable 
  add_custom_command (
    OUTPUT ${TARGETEXE_INSTALL_DIR}/${OUTPUT_FILE}
    COMMAND ${LINKER_SCRIPT} $<CONFIGURATION> ${TARGETEXE_INSTALL_DIR}/${OUTPUT_FILE} ${TARGETEXE_INSTALL_DIR}/${PDB_FILE} ${LLVM_TOOLS_LIB_DIR}/${IMPLIB_FILE}  ${LLVM_TOOLS_LIB_DIR} ${TMP_FILES}
    # copy exe to folder which is not depends on $<CONFIGURATION>, i.e. Debug or Release
	# becasue install command doesn't support using $<CONFIGURATION> or CMAKE_CFG_INTDIR	
	COMMAND ${CMAKE_COMMAND} -E copy  "${TARGETEXE_INSTALL_DIR}/${OUTPUT_FILE}" ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${LINKER_SCRIPT} ${TMP_FILES_DEPEND}
    COMMENT "C Linking ${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
    VERBATIM
    )

  set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE)

endfunction( add_c_exe )

#------------------ Main -----------------------
# C source code
list (APPEND C_SOURCE_FILES
  DMTest.cpp
  DMImageTest.cpp
  CMPTest.cpp
  DGTest.cpp
  Float16Test.cpp
  NEATALUUtils.cpp
  NEATALUTest.cpp
  NEATALUCommonTest.cpp
  NEATALUConversionTest.cpp
  NEATALUVLoadStoreTest.cpp
  NEATValueTest.cpp
  NEATMathTest.cpp
  OCLBuiltinParserTest.cpp
  NEATALUImageTest.cpp
  )

 # project header files
list (APPEND HEADER_FILES
  ALUTest.h
  NEATALUUtils.h
  )

set (SAVE_C_SOURCE_FILES ${C_SOURCE_FILES})
unset (C_SOURCE_FILES)
foreach (FILE ${SAVE_C_SOURCE_FILES})
  list (APPEND C_SOURCE_FILES ${TARGETEXE_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_HEADER_FILES ${HEADER_FILES})
unset (HEADER_FILES)
foreach (FILE ${SAVE_HEADER_FILES})
  list (APPEND HEADER_FILES ${TARGETEXE_SOURCE_DIR}/${FILE})
endforeach (FILE)

# conversion of filepaths used during configuration of ICC compiler and linker scripts to native format (as they will be directly used by these scripts)
file (TO_NATIVE_PATH ${ICC_CPU_ENV_SCRIPT} ICC_CPU_ENV_SCRIPT_NATIVE)
file (TO_NATIVE_PATH ${ICC_CPU_CL}         ICC_CPU_CL_NATIVE)
file (TO_NATIVE_PATH ${ICC_CPU_LD}         ICC_CPU_LD_NATIVE)
file (TO_NATIVE_PATH ${VS100_CPU_ENV_SCRIPT} VS100_CPU_ENV_SCRIPT_NATIVE)
file (TO_NATIVE_PATH ${VS100_CPU_CL}         VS100_CPU_CL_NATIVE)

# configuring ICC compiler and linker scripts (the configured scripts are in ${CMAKE_CURRENT_BINARY_DIR})
configure_file (${ICC_CPU_COMPILER_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_COMPILER_SCRIPT} @ONLY)
configure_file (${ICC_CPU_LINKER_SCRIPT}   ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_LINKER_SCRIPT} @ONLY) 
configure_file (${VS100_CPU_COMPILER_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${VS100_CPU_COMPILER_SCRIPT} @ONLY)

# redefine ICC compiler and linker scripts to their new locations
set (ICC_CPU_COMPILER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_COMPILER_SCRIPT})
set (ICC_CPU_LINKER_SCRIPT   ${CMAKE_CURRENT_BINARY_DIR}/${ICC_CPU_LINKER_SCRIPT})
set (VS100_CPU_COMPILER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${VS100_CPU_COMPILER_SCRIPT})

# intermediate directory for OpenCL and ICC compilation/link
# note that platform is already accounted by ${CMAKE_CURRENT_BINARY_DIR}
    set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})


# build exe file out of C code (by ICC) and copy it to install directory
add_c_exe( ${ICC_CPU_LINKER_SCRIPT} ${ICC_CPU_COMPILER_SCRIPT} ${VS100_CPU_COMPILER_SCRIPT} OUTPUT_C_EXE ${TARGET_NAME} ${INTERMEDIATE_OUTPUT_DIR} ${C_SOURCE_FILES})

# top-level target for C target
add_custom_target (${TARGET_NAME} ALL
  DEPENDS ${TARGETEXE_INSTALL_DIR}/${OUTPUT_C_EXE}
  COMMENT "Target ${TARGET_NAME} build completed"
  VERBATIM
  SOURCES ${C_SOURCE_FILES} ${HEADER_FILES}
  )	

set(DATA_MANAGER_LIBS DataManager tinyxml_STL)
set(COMPARATOR_LIBS Comparator)
set(DATA_GENERATOR_LIBS DataGenerator)
SET(NEAT_ALU_LIBS NEAT_wrapper REFALU_wrapper)
SET(PLUGIN_NEAT_LIBS PlugInNEAT OCLBuiltins)
set(UNIT_TEST_LIBS
  gtest
  gtest_main
  LLVMSupport # gtest needs it for raw_ostream.
)
  
add_dependencies(${TARGET_NAME} 
  ${DATA_MANAGER_LIBS} 
  ${COMPARATOR_LIBS} 
  ${DATA_GENERATOR_LIBS}
  ${UNIT_TEST_LIBS} 
  ${NEAT_ALU_LIBS} 
  ${PLUGIN_NEAT_LIBS}
  ImathLibd_wrapper
  OclPluginManager
  dynamic_load
  OCLRecorderStat
  OCLBuilder
  )

add_dependencies(UnitTests ${TARGET_NAME})
add_subdirectory(fe_plugin_mock)
  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_C_EXE} DESTINATION bin)
else ()


set(LLVM_REQUIRES_EH 1)
# unittests for validation
add_llvm_executable(${TARGET_NAME}
  DMTest.cpp
  DMImageTest.cpp
  CMPTest.cpp
  DGTest.cpp
  Float16Test.cpp
  NEATALUUtils.h
  NEATALUUtils.cpp
  NEATALUTest.cpp
  NEATALUCommonTest.cpp
  NEATALUConversionTest.cpp
  NEATALUVLoadStoreTest.cpp
  NEATValueTest.cpp
  NEATMathTest.cpp
  OCLBuiltinParserTest.cpp
  NEATALUImageTest.cpp
  SourceRecorder.cpp
  ClangSA.cpp
  )

add_subdirectory(fe_plugin_mock)

include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/plugin_manager)
include_directories(${BACKEND_ROOT_DIR}/dynamic_lib)
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export)
include_directories(${VALIDATIONS_ROOT}/Common)
include_directories(${VALIDATIONS_ROOT}/DataManager)
include_directories(${VALIDATIONS_ROOT}/Comparator)
include_directories(${VALIDATIONS_ROOT}/ImathLibd)
include_directories(${VALIDATIONS_ROOT}/NEAT)
include_directories(${VALIDATIONS_ROOT}/REFALU)
include_directories(${VALIDATIONS_ROOT}/DataGenerator)
include_directories(${VALIDATIONS_ROOT}/OCLBuiltins)
include_directories(${LLVM_MAIN_SRC_DIR}/../external/tinyxml)
include_directories(${VALIDATIONS_ROOT}/PlugInNEAT)
include_directories(${VALIDATIONS_ROOT}/OCLBuilder)
include_directories(${VALIDATIONS_ROOT}/OCLRecorder)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Common )

link_directories(${VALIDATIONS_ROOT}/../validations)
link_directories(${LLVM_MAIN_SRC_DIR}/../external/tinyxml)
link_directories(${VALIDATIONS_ROOT}/NEAT)
link_directories(${VALIDATIONS_ROOT}/PlugInNEAT)

set(DATA_MANAGER_LIBS DataManager tinyxml_STL)
set(COMPARATOR_LIBS Comparator)
set(DATA_GENERATOR_LIBS DataGenerator)
SET(NEAT_ALU_LIBS NEAT REFALU)
SET(PLUGIN_NEAT_LIBS PlugInNEAT OCLBuiltins)
set(UNIT_TEST_LIBS
  gtest
  gtest_main
  LLVMSupport # gtest needs it for raw_ostream.
)


target_link_libraries(${TARGET_NAME}
  ${DATA_MANAGER_LIBS}
  ${COMPARATOR_LIBS}
  ${DATA_GENERATOR_LIBS}
  ${UNIT_TEST_LIBS}
  ${NEAT_ALU_LIBS}
  ${PLUGIN_NEAT_LIBS}
  ${LLVM_BINARY_DIR}/lib/libImathLibd.so
  OclPluginManager
  dynamic_load
  OCLRecorderStat
  OCLBuilder
)

add_dependencies(${TARGET_NAME} 
  ${DATA_MANAGER_LIBS} 
  ${COMPARATOR_LIBS} 
  ${DATA_GENERATOR_LIBS}
  ${UNIT_TEST_LIBS} 
  ${NEAT_ALU_LIBS} 
  ${PLUGIN_NEAT_LIBS}
  ImathLibd_wrapper
  OclPluginManager
  dynamic_load
  OCLRecorderStat
  OCLBuilder
  )

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

endif (WIN32)

