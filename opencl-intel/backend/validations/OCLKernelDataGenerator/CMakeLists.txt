use_eh(TRUE)

# TinyXML library is used for serialization to XML.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
include_directories(${VALIDATIONS_ROOT}/DataManager )
include_directories(${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser )

# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_library(OCLKernelDataGenerator
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
target_link_libraries(OCLKernelDataGenerator
DataManager)

set_target_properties( OCLKernelDataGenerator PROPERTIES FOLDER "Backend" )
