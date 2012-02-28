add_definitions(-DGTEST_HAS_RTTI=0)

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

include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${VALIDATIONS_ROOT}/Common)
include_directories(${VALIDATIONS_ROOT}/DataManager)
include_directories(${VALIDATIONS_ROOT}/Comparator)
include_directories(${VALIDATIONS_ROOT}/NEAT)
include_directories(${VALIDATIONS_ROOT}/REFALU)
include_directories(${VALIDATIONS_ROOT}/DataGenerator)
include_directories(${VALIDATIONS_ROOT}/OCLBuiltins)
include_directories(${LLVM_MAIN_SRC_DIR}/../external/tinyxml)
include_directories(${VALIDATIONS_ROOT}/PlugInNEAT)
link_directories(${VALIDATIONS_ROOT}/../validations)
link_directories(${LLVM_MAIN_SRC_DIR}/../external/tinyxml)
link_directories(${VALIDATIONS_ROOT}/NEAT)
link_directories(${VALIDATIONS_ROOT}/PlugInNEAT)

set(DATA_MANAGER_LIBS DataManager tinyxml_STL)
set(COMPARATOR_LIBS Comparator)
set(DATA_GENERATOR_LIBS DataGenerator)
SET(NEAT_ALU_LIBS NEAT REFALU)
SET(PLUGIN_NEAT_LIBS PlugInNEAT OCLBuiltins)
set(UNIT_TEST_LIBS
  gtest
  gtest_main
  LLVMSupport # gtest needs it for raw_ostream.
)

target_link_libraries(ValidationTests ${DATA_MANAGER_LIBS} ${COMPARATOR_LIBS} ${DATA_GENERATOR_LIBS}  ${UNIT_TEST_LIBS} ${NEAT_ALU_LIBS} ${PLUGIN_NEAT_LIBS})
add_dependencies(ValidationTests ${DATA_MANAGER_LIBS} ${COMPARATOR_LIBS} ${DATA_GENERATOR_LIBS}  ${UNIT_TEST_LIBS} ${NEAT_ALU_LIBS} ${PLUGIN_NEAT_LIBS})
add_dependencies(UnitTests ValidationTests)
set_target_properties(ValidationTests PROPERTIES FOLDER "Tests")

