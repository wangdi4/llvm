# unittests for validation
add_llvm_executable(ValidationTests
  DMTest.cpp
  DMImageTest.cpp
  CMPTest.cpp
  DGTest.cpp
  Float16Test.cpp
  NEATALUUtils.h
  NEATALUUtils.cpp
  NEATALUTest.cpp
  NEATALUCommonTest.cpp
  NEATALUConversionTest.cpp
  NEATALUVLoadStoreTest.cpp
  NEATValueTest.cpp
  NEATMathTest.cpp
  OCLBuiltinParserTest.cpp
  NEATALUImageTest.cpp
  )

include_directories(${CMAKE_SOURCE_DIR}/../cl_api)
include_directories(${CMAKE_SOURCE_DIR}/../build/Win32/include)
include_directories(${CMAKE_SOURCE_DIR}/../validations/Common)
include_directories(${CMAKE_SOURCE_DIR}/../validations/DataManager)
include_directories(${CMAKE_SOURCE_DIR}/../validations/Comparator)
include_directories(${CMAKE_SOURCE_DIR}/../validations/NEAT)
include_directories(${CMAKE_SOURCE_DIR}/../validations/REFALU)
include_directories(${CMAKE_SOURCE_DIR}/../validations/DataGenerator)
include_directories(${CMAKE_SOURCE_DIR}/../validations/OCLBuiltins)
include_directories(${CMAKE_SOURCE_DIR}/../external/tinyxml)
include_directories(${CMAKE_SOURCE_DIR}/lib/ExecutionEngine/PlugInNEAT)
link_directories(${CMAKE_SOURCE_DIR}/../validations)
link_directories(${CMAKE_SOURCE_DIR}/../external/tinyxml)
link_directories(${CMAKE_SOURCE_DIR}/../validations/NEAT)
link_directories(${CMAKE_SOURCE_DIR}/lib/ExecutionEngine/PlugInNEAT)

set(DATA_MANAGER_LIBS DataManager tinyxml_STL)
set(COMPARATOR_LIBS Comparator)
set(DATA_GENERATOR_LIBS DataGenerator)
SET(NEAT_ALU_LIBS NEAT REFALU)
SET(PLUGIN_NEAT_LIBS PlugInNEAT OCLBuiltins)

target_link_libraries(ValidationTests ${DATA_MANAGER_LIBS} ${COMPARATOR_LIBS} ${DATA_GENERATOR_LIBS}  ${UNIT_TEST_LIBS} ${NEAT_ALU_LIBS} ${PLUGIN_NEAT_LIBS})
add_dependencies(ValidationTests ${DATA_MANAGER_LIBS} ${COMPARATOR_LIBS} ${DATA_GENERATOR_LIBS}  ${UNIT_TEST_LIBS} ${NEAT_ALU_LIBS} ${PLUGIN_NEAT_LIBS})