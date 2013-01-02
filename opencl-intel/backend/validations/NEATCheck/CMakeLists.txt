set (CMP_INCLUDE_FILES)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
include_directories( ${VALIDATIONS_ROOT}/DataManager )
include_directories( ${VALIDATIONS_ROOT}/NEAT )

if (WIN32)
    set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (ICC_PLATFORM ia32)
    else ()
      set (ICC_PLATFORM intel64)
    endif ()

    link_directories( "${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}" )
endif()

link_directories (
    ${LLVM_LIBRARY_DIR}
) 

add_executable(NEATChecker
  main.cpp
)

target_link_libraries(NEATChecker 
    REFALU
    NEAT
    DataManager
    ImathLibd
    tinyxml_STL
    ${LLVM_MODULE_LIBS}
)
