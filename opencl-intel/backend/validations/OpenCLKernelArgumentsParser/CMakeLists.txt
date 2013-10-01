use_eh(TRUE)

include_directories( ${VALIDATIONS_ROOT}/DataManager )
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_library(OpenCLKernelArgumentsParser
  IOpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.cpp
)

set_target_properties( OpenCLKernelArgumentsParser PROPERTIES FOLDER "Backend" )
