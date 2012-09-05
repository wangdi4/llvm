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
