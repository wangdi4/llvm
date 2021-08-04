set(TARGET_NAME ProgramConfiguration)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${VALIDATIONS_ROOT}/SATest
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/../external/tinyxml
  ${BACKEND_ROOT_DIR}/../cl_api
  ${CL_API_HEADERS}
  ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
  ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
)

add_opencl_library(${TARGET_NAME} STATIC
  EXCLUDE_FROM_ALL_BUILD

  IProgramConfiguration.h
  OpenCLProgramConfiguration.h
  OpenCLProgramConfiguration.cpp
  DXProgramConfiguration.h
  DXProgramConfiguration.cpp
)

target_link_libraries(${TARGET_NAME}
    OCLKernelDataGenerator
	)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
