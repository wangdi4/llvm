cmake_minimum_required(VERSION 2.8.4)

set(LLVM_REQUIRES_EH 1)

set(TARGET_NAME NameManglingTests)

if(WIN32)
set (OCL_TBLGEN_EXE
  "${LLVM_TOOLS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/ocl-tblgen.exe"
)
else(WIN32)
set (OCL_TBLGEN_EXE
  "${LLVM_TOOLS_BINARY_DIR}/ocl-tblgen"
)
endif(WIN32)

set(GenCommand ${OCL_TBLGEN_EXE} -gen-ocl-biheader 
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/target-knc.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  COMMAND ${OCL_TBLGEN_EXE} -gen-ocl-biheader 
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/target-knc.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  DEPENDS ocl-tblgen
  COMMENT "genereating demangled headers... ${GenCommand}"
  VERBATIM
)

if (WIN32)
  add_definitions(/EHsc)
else(WIN32)
  add_definitions(-fexceptions)
endif(WIN32)

add_llvm_executable(
  ${TARGET_NAME}
  MangledNames.h
  MangleTest.cpp
)

include_directories(
  ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include
  ${NAME_MANGLE_ROOT_DIR}/antlruntime/include
  ../
)

target_link_libraries(
  ${TARGET_NAME}
  antlr
  name_mangle
  gtest
  gtest_main
  LLVMSupport
)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)