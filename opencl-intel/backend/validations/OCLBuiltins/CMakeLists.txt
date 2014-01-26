#
# Global and platform settings
#
cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

# Settings for Linux compilation
#
set(LLVM_REQUIRES_EH 1)

if (NOT WIN32)

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")

set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
set (ADD_C_FLAGS_RELEASE "-O2 -g0 ")

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

endif (NOT WIN32)

set (TARGET_NAME "ref_cl_builtins")

project( ${TARGET_NAME} )

# remember OCL builtins source dir
set (REF_CL_BUILTIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (STATUS "Reference built-ins sources are in: ${REF_CL_BUILTIN_SOURCE_DIR}")

#------------------- Utility functions ---------------------------
function(tblgenrefoclblt OUTPUT_VAR OUTPUT_DIR IFN)
    string(REPLACE ".td" ".cpp" OFN ${IFN})
    file(GLOB_RECURSE depend_tds "${REF_CL_BUILTIN_SOURCE_DIR}/*.td")
    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${OFN}
        COMMAND  ref-ocl-tblgen ${ARGN} -I=${REF_CL_BUILTIN_SOURCE_DIR} -o ${OUTPUT_DIR}/${OFN} ${REF_CL_BUILTIN_SOURCE_DIR}/${IFN}
        DEPENDS  ref-ocl-tblgen ${depend_tds}
        COMMENT "Generating reference built-in file ${OFN}..."
        )
    set_source_files_properties(${OUTPUT_DIR}/${OFN} PROPERTIES GENERATED 1)
    set(${OUTPUT_VAR} ${OUTPUT_DIR}/${OFN} PARENT_SCOPE)
endfunction(tblgenrefoclblt)


#------------------ Main -----------------------

list (APPEND TD_FILES
  ./src/BLTAsyncCopiesAndPrefetch.td
  ./src/BLTAtomic.td
  ./src/BLTCommon.td
  ./src/BLTConversionsChar.td
  ./src/BLTConversionsShort.td
  ./src/BLTConversionsInt.td
  ./src/BLTConversionsLong.td
  ./src/BLTConversionsFPoint.td
  ./src/BLTExplMemFenceOps.td
  ./src/BLTGeometric.td
  ./src/BLTImages.td
  ./src/BLTInteger.td
  ./src/BLTMath.td
  ./src/BLTMiscellaneousVector.td
  ./src/BLTRelational.td
  ./src/BLTWorkGroup.td
  ./src/BLTWorkItem.td
  ./src/VLoadStore.td
)

list (APPEND OCL_INCLUDE_FILES
  BLTAsyncCopiesAndPrefetch.h
  BLTAtomic.h
  BLTCommon.h
  BLTConversion.h
  BLTExplMemFenceOps.h
  BLTGeometric.h
  BLTImages.h
  BLTInteger.h
  BLTMath.h
  BLTMiscellaneousVector.h
  BLTRelational.h
  BLTWorkGroup.h
  BLTWorkItem.h
  VLoadStore.h
)

foreach (FILE ${TD_FILES})
  tblgenrefoclblt(OCL_TABLEGEN_SOURCE ${REF_CL_BUILTIN_SOURCE_DIR} ${FILE} -gen-ocl-src)
  list (APPEND OCL_SOURCE_FILES ${OCL_TABLEGEN_SOURCE})
endforeach (FILE)

add_custom_target (${TARGET_NAME} ALL
  DEPENDS ${OCL_SOURCE_FILES} ref-ocl-tblgen
  COMMENT "Target ${TARGET_NAME} build completed"
  VERBATIM
  SOURCES ${OCL_SOURCE_FILES}
)

add_library(OCLBuiltins
  ${OCL_SOURCE_FILES}
  ${OCL_INCLUDE_FILES}
  Helpers.h
  Helpers.cpp
  OCLBuiltinParser.h
  OCLBuiltinParser.cpp
  IWorkItemBuiltins.h
)

add_dependencies(OCLBuiltins
  ${TARGET_NAME}
)

include_directories (
  ${BACKEND_ROOT_DIR}/../cl_api
  ${BACKEND_ROOT_DIR}/name_mangling
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/REFALU
  ${REF_CL_BUILTIN_SOURCE_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
