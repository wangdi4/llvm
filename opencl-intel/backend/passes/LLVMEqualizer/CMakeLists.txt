set(TARGET_NAME LLVMEqualizer)

set(TARGET_SOURCE_FILES LLVMEqualizer.cpp)
set(TARGET_INCLUDE_FILES LLVMEqualizer.h)

include_directories(
  ${OCL_SOURCE_DIR}/backend/metadata_api
  ${OCL_SOURCE_DIR}/backend/name_mangling
  ${OCL_SOURCE_DIR}/backend/passes/BuiltinLibInfo
  ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)
  
if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

