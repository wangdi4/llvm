use_eh(TRUE)

set (DG_INCLUDE_FILES
  DataGenerator.h
  DGHelper.h
  )
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${VALIDATIONS_ROOT}/DataManager )
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_library(DataGenerator
  ${DG_INCLUDE_FILES}
  DataGenerator.cpp
  DGHelper.cpp
)
