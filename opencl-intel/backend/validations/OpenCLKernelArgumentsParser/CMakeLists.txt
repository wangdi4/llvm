use_eh(TRUE)

include_directories(${LLVM_INCLUDE_DIRS}
                    ${VALIDATIONS_ROOT}/DataManager
)
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_library(OpenCLKernelArgumentsParser
  IOpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.cpp
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(OpenCLKernelArgumentsParser intrinsics_gen)
endif()

set_target_properties( OpenCLKernelArgumentsParser PROPERTIES FOLDER "Backend" )
