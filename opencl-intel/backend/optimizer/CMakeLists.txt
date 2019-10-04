set(TARGET_NAME OclOptimizer)

set(TARGET_INCLUDE_FILES
    Optimizer.h
)

set(TARGET_SOURCE_FILES
    Optimizer.cpp
)

include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${CL_API_HEADERS}
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/vectorizer/Utils
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/passes/ChannelPipeTransformation
    ${CMAKE_BINARY_DIR}/include/
)

include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/passes/PrintIRPass
    ${BACKEND_ROOT_DIR}/passes/
)
link_libraries(
    BackendPassesWrappers
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
  add_dependencies(${TARGET_NAME} directives_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
