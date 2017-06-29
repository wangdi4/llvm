cmake_minimum_required(VERSION 2.8.7)

if( ANDROID )
  set( BACKEND_BUILD_TESTS            OFF  CACHE STRING "")
  set( BACKEND_BUILD_VERIFICATION_LIB OFF  CACHE STRING "")
else( ANDROID )
  set( BACKEND_BUILD_TESTS            ON  CACHE STRING "")
  set( BACKEND_BUILD_VERIFICATION_LIB ON  CACHE STRING "")
endif( ANDROID )

set(TARGET_NAME name_mangle)

project (${TARGET_NAME})

if (NOT APPLE)
  add_subdirectory(unittests)
endif (NOT APPLE)

add_definitions(-DSUBSTITUTE_OPENCL_TYPES)

set(TARGET_SOURCE_FILES
  ParameterType.cpp
  Demangler.cpp
  Mangler.cpp
  FunctionDescriptor.cpp
  DemangleParser.cpp
  Utils.cpp
)

set(TARGET_INCLUDE_FILES
  ParameterType.h
  NameMangleAPI.h
  FunctionDescriptor.h
  Refcount.h
  DemangleParser.h
  Utils.h
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

if (NOT APPLE)
  include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
endif (NOT APPLE)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
