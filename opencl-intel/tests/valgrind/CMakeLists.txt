#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set(TARGET valgrind )

set(VALTEST_DIR_NAME tests/valgrind)
set(VALTEST_FULL_DIR "${CMAKE_INSTALL_PREFIX}/${VALTEST_DIR_NAME}")
set(VALTEST_JENKINS_DIR "${VALTEST_FULL_DIR}/.offline_jenkins")

file(MAKE_DIRECTORY ${VALTEST_FULL_DIR})
file(MAKE_DIRECTORY ${VALTEST_JENKINS_DIR})

install(FILES "README.TXT" DESTINATION ${VALTEST_DIR_NAME})

install(DIRECTORY . 
    DESTINATION ${VALTEST_DIR_NAME}/.offline_jenkins
    FILES_MATCHING
    PATTERN "*.py" 
    PATTERN "*.expected" 
    PATTERN "*.supp"
    PATTERN "valRun*.py" EXCLUDE
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/valRun.py
    ${VALTEST_FULL_DIR}
    @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/valRunJenkins.py
    ${VALTEST_FULL_DIR} 
    @ONLY)
