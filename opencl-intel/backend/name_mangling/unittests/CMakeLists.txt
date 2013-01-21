cmake_minimum_required(VERSION 2.8.4)

set(LLVM_REQUIRES_EH 1)

set(TARGET_NAME NameManglingTests)

if(WIN32)
  set (OCL_TBLGEN_EXE "${LLVM_TOOLS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/ocl-tblgen.exe")
else(WIN32)
  if( LLVM_USE_NATIVE )
    set (OCL_TBLGEN_EXE "${CMAKE_BINARY_DIR}/backend/llvm/bin/ocl-tblgen")
  elseif( CMAKE_CROSSCOMPILING OR ANDROID )
    set (OCL_TBLGEN_EXE "${CMAKE_BINARY_DIR}/native/backend/llvm/bin/ocl-tblgen")
  else( )
    set (OCL_TBLGEN_EXE "${LLVM_TOOLS_BINARY_DIR}/ocl-tblgen")
  endif( LLVM_USE_NATIVE )
endif(WIN32)

set(GenCommand ${OCL_TBLGEN_EXE} -gen-ocl-biheader 
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/KNCni/target-b2.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  COMMAND ${OCL_TBLGEN_EXE} -gen-ocl-biheader 
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/KNCni/target-b2.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  DEPENDS ocl-tblgen ${OCL_TBLGEN_EXE}
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
  TestRefCount.cpp
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

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
