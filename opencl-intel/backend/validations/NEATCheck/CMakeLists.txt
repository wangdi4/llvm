set (CMP_INCLUDE_FILES)
set (TARGET_NAME NEATChecker)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/NEAT
)

link_directories (
  ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}
)

add_executable(${TARGET_NAME}
  main.cpp
)

target_link_libraries(${TARGET_NAME}
    DataManager
    REFALU
    NEAT
    ImathLibd
    tinyxml_STL
    ${LLVM_MODULE_LIBS}
)

if(NOT WIN32)
    add_dependencies(${TARGET_NAME} ImathLibd_wrapper)
endif(NOT WIN32)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests)
