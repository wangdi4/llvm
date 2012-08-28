set(TARGET_NAME ShuffleCallToInst)

include_directories(${BACKEND_ROOT_DIR}/name_mangling)

set(SHUFFLECALLTOINST_INCLUDE_FILES
    ShuffleCallToInst.h
)

set(SHUFFLECALLTOINST_SOURCE_FILES
    ShuffleCallToInst.cpp
)

add_library(${TARGET_NAME}
    ${SHUFFLECALLTOINST_INCLUDE_FILES}
    ${SHUFFLECALLTOINST_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    name_mangle
)

add_dependencies(${TARGET_NAME}
    name_mangle
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)