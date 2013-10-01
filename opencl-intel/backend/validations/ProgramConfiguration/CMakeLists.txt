cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

set(TARGET_NAME ProgramConfiguration)

include_directories(
    ${VALIDATIONS_ROOT}/SATest
    ${VALIDATIONS_ROOT}/NEAT
    ${VALIDATIONS_ROOT}/Common
    ${VALIDATIONS_ROOT}/../external/tinyxml
    ${BACKEND_ROOT_DIR}/../cl_api
    ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
    ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
)

add_library(${TARGET_NAME} STATIC
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
