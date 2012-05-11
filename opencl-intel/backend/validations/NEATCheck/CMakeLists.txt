set (CMP_INCLUDE_FILES 
  )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
include_directories( ${VALIDATIONS_ROOT}/DataManager )
include_directories( ${VALIDATIONS_ROOT}/NEAT )

get_target_property( lib_dir NEAT_wrapper lib_dir )
set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (ICC_PLATFORM ia32)
else ()
  set (ICC_PLATFORM intel64)
endif ()
set( intel_lib_dir "${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}")
link_directories( ${lib_dir} ${intel_lib_dir} )

add_executable(NEATChecker
  main.cpp
)


target_link_libraries(NEATChecker 
LLVMSupport 
REFALU
NEAT
DataManager
ImathLibd
)

if (WIN32)
add_dependencies(NEATChecker 
LLVMSupport
REFALU_wrapper
NEAT_wrapper
DataManager
ImathLibd_wrapper
)
else ()
add_dependencies(NEATChecker 
LLVMSupport
REFALU
NEAT
DataManager
ImathLibd
)
endif ()
