set(LLVM_REQUIRES_EH 1)

include_directories( ${VALIDATIONS_ROOT}/DataManager )
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_llvm_library(OpenCLKernelArgumentsParser
  IOpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.cpp
)
