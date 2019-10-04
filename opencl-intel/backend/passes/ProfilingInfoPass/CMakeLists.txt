set(TARGET_NAME ProfilingInfoPass)

set(PROFILINGINFOPASS_SOURCE_FILES
    ProfilingInfoPass.cpp
)

add_library(${TARGET_NAME}
    ${PROFILINGINFOPASS_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
