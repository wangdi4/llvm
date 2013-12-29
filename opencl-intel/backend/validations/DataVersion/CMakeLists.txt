use_eh(TRUE)

include_directories(
${VALIDATIONS_ROOT}/DataVersion
${VALIDATIONS_ROOT}/DataManager
${VALIDATIONS_ROOT}/NEAT
${BACKEND_ROOT_DIR}/../cl_api
)

# Set useful variable for all header files to manipulate them all together.
set (DV_INCLUDE_FILES
  DataVersion.h
  )

# Data Version Library
add_library(DataVersion
  ${DV_INCLUDE_FILES}
  DataVersion.cpp
)
