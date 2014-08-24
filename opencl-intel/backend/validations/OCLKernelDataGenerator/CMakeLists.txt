use_eh(TRUE)

set (TARGET_NAME OCLKernelDataGenerator)

# TinyXML library is used for serialization to XML.
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
include_directories(${VALIDATIONS_ROOT}/DataManager )
include_directories(${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser )

# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_library(${TARGET_NAME}
OCLKernelDataGenerator.h
OCLKernelDataGenerator.cpp
RandomUniformProvider.h
RandomUniformProvider.cpp
Generators.cpp
Generators.h
GeneratorConfig.cpp
GeneratorConfig.h
OCLKernelBufferContainerListAllocator.h
OCLKernelBufferContainerListAllocator.cpp
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_VALIDATION_FOLDER_NAME} )
