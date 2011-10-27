set (CMP_INCLUDE_FILES 
  Comparator.h
  StatisticsCollector.h
  IComparisonResults.h
  ComparisonResults.h
  ../Common/dxfloat.h
  ../Common/Exception.h
  )

include_directories( 
    ${VALIDATIONS_ROOT}/DataManager
    ${VALIDATIONS_ROOT}/NEAT
    ${CMAKE_SOURCE_DIR}/../cl_api
    ${CMAKE_SOURCE_DIR}/../ocl_cpu_backend
    ${CMAKE_SOURCE_DIR}/../ocl_cpu_backend/export
    )
add_llvm_library(Comparator STATIC
  ${CMP_INCLUDE_FILES}
  Comparator.cpp
  ComparisonResults.cpp
  StatisticsCollector.cpp
)

add_dependencies(Comparator DataManager NEAT)