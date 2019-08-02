use_eh(TRUE)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${VALIDATIONS_ROOT}/DataVersion
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/NEAT
  ${CL_API_HEADERS}
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

if (OPENCL_INTREE_BUILD)
  add_dependencies(DataVersion intrinsics_gen)
endif()
