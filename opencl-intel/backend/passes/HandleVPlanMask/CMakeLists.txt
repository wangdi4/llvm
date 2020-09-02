set(TARGET_NAME HandleVPlanMask)

set(TARGET_INCLUDE_FILES
    HandleVPlanMask.h
)

set(TARGET_SOURCE_FILES
    HandleVPlanMask.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen LLVMVectorizer)
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${OCL_BINARY_DIR}/backend/vectorizer)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
