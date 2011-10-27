set (DG_INCLUDE_FILES
  DataGenerator.h
  DGHelper.h
  )

include_directories( ${VALIDATIONS_ROOT}/DataManager )
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_llvm_library(DataGenerator STATIC
  ${DG_INCLUDE_FILES}
  DataGenerator.cpp
  DGHelper.cpp
)

add_dependencies(DataGenerator DataManager)