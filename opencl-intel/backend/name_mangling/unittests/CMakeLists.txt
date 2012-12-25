cmake_minimum_required(VERSION 2.8.4)
use_eh(TRUE)

set(TARGET_NAME NameManglingTests)

set (OCL_TBLGEN_EXE  "${OCL_TOOLS_BINARY_DIR}/${CMAKE_CFG_INTDIR}/ocl-tblgen.exe")

set(GenCommand ${OCL_TBLGEN_EXE} -gen-ocl-biheader 
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/target-knc.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h)

include_directories(
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${NAME_MANGLE_ROOT_DIR}/antlruntime/include
  ../
)

link_directories(
    ${LLVM_LIBRARY_DIR}
)

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

add_executable(
  ${TARGET_NAME}
  MangledNames.h
  MangleTest.cpp
  TestRefCount.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  antlr
  name_mangle
  googletest
  LLVMSupport
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
