set(TARGET_NAME SetPreferVectorWidth)

add_library(${TARGET_NAME} STATIC
  SetPreferVectorWidth.cpp
  SetPreferVectorWidth.h)

add_dependencies(${TARGET_NAME} intrinsics_gen)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
