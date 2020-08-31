set(TARGET_NAME AddFastMath)

add_library(${TARGET_NAME}
  AddFastMath.cpp
  AddFastMath.h)

add_dependencies(${TARGET_NAME} intrinsics_gen)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
