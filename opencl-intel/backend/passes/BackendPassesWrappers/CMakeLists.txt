set(TARGET_NAME BackendPassesWrappers)

include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/passes/AddImplicitArgs
    ${BACKEND_ROOT_DIR}/passes/LocalBuffAnalysis
    ${BACKEND_ROOT_DIR}/passes/LocalBuffers
)

set (PASSESWRAPPERS_SOURCE_FILES
    PassesWrappers.cpp
)

add_library(${TARGET_NAME}
    ${PASSESWRAPPERS_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
