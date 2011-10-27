set (CMP_INCLUDE_FILES 
  )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
include_directories( ${VALIDATIONS_ROOT}/DataManager )
include_directories( ${VALIDATIONS_ROOT}/NEAT )

add_llvm_executable(NEATChecker
  main.cpp
)

target_link_libraries(NEATChecker LLVMSupport NEAT DataManager )
add_dependencies(NEATChecker LLVMSupport NEAT DataManager )
