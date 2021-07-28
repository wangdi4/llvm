set (CMP_INCLUDE_FILES
  Comparator.h
  StatisticsCollector.h
  IComparisonResults.h
  ComparisonResults.h
  ../Common/dxfloat.h
  ../Common/Exception.h
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/NEAT
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}
  ${BACKEND_ROOT_DIR}/export
)
add_opencl_library(Comparator STATIC
  EXCLUDE_FROM_ALL_BUILD

  ${CMP_INCLUDE_FILES}
  Comparator.cpp
  ComparisonResults.cpp
  StatisticsCollector.cpp
)

add_dependencies(Comparator DataManager NEAT)
