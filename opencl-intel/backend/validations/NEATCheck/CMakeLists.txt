set (CMP_INCLUDE_FILES)
set (TARGET_NAME NEATChecker)

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
    ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}
    ${LLVM_LIBRARY_DIR}
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

if( NOT WIN32)
    target_link_libraries(${TARGET_NAME}
        pthread
    )

    add_dependencies(${TARGET_NAME}
        ImathLibd_wrapper
    )
endif ()
