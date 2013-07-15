cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

set(TARGET_NAME ProgramConfiguration)

include_directories(
    ${VALIDATIONS_ROOT}/SATest
    ${VALIDATIONS_ROOT}/Common
    ${VALIDATIONS_ROOT}/../external/tinyxml
    ${BACKEND_ROOT_DIR}/../cl_api
)

add_library(${TARGET_NAME} STATIC
    IProgramConfiguration.h
    OpenCLProgramConfiguration.h
    OpenCLProgramConfiguration.cpp
    DXProgramConfiguration.h
    DXProgramConfiguration.cpp
)

target_link_libraries(${TARGET_NAME}
    tinyxml_STL
    ${LLVM_MODULE_LIBS}
    )
