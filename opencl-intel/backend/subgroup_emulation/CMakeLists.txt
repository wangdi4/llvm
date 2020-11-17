set(TARGET_NAME SGEmulation)

file(GLOB_RECURSE TARGET_INCLUDE_FILES *.h)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/passes
  ${BACKEND_ROOT_DIR}/utils
  ${BACKEND_ROOT_DIR}/barrier/include
  ${OCL_BINARY_DIR}/backend/vectorizer
)

set(
  TARGET_SOURCE_FILES
  SGBarrierPropagate.cpp
  SGBarrierSimplify.cpp
  SGBuiltin.cpp
  SGHelper.cpp
  SGSizeAnalysis.cpp
  SGValueWiden.cpp
  SGFunctionWiden.cpp
  SGLoopConstruct.cpp
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} LLVMVectorizer LoopUtils)

set(BACKEND_PASSES_FOLDER_NAME ${BACKEND_FOLDER_NAME}/passes)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
