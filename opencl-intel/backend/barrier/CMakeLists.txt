set( TARGET_NAME LLVMBarrier)

file(GLOB_RECURSE TARGET_INCLUDE_FILES *.h)

include_directories(BEFORE "include")
include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api 
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/metadata_api/gen
    ${BACKEND_ROOT_DIR}/name_mangling
    )

set(TARGET_SOURCE_FILES
  GroupBuiltinPass.cpp
  BarrierInFunctionPass.cpp
  BarrierMain.cpp
  BarrierPass.cpp
  BarrierUtils.cpp
  DataPerBarrierPass.cpp
  DataPerInternalFunctionPass.cpp
  DataPerValuePass.cpp
  ImplicitGIDPass.cpp
  RedundantPhiNodePass.cpp
  RemoveDuplicationBarrierPass.cpp
  SplitBBonBarrierPass.cpp
  WIRelatedValuePass.cpp
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
