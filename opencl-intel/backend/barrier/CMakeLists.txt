set( TARGET_NAME LLVMBarrier)

file(GLOB_RECURSE TARGET_INCLUDE_FILES *.h)

include_directories(BEFORE "include")
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${BACKEND_ROOT_DIR}/../cl_api 
    ${CL_API_HEADERS}
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/name_mangling
    ${CMAKE_BINARY_DIR}/include/
    )

set(TARGET_SOURCE_FILES
  GroupBuiltinPass.cpp
  BarrierInFunctionPass.cpp
  BarrierMain.cpp
  BarrierPass.cpp
  BarrierUtils.cpp
  DataPerBarrierPass.cpp
  DataPerValuePass.cpp
  DuplicateCalledKernelsPass.cpp
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

add_dependencies(${TARGET_NAME} metadataapi LoopUtils)

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
