
set(
    sources
    conversions.c
    errorHelpers.c
    genericThread.cpp
    imageHelpers.cpp
    kernelHelpers.c
    mingw_compat.c
    msvc9.c
    mt19937.c
    os_helpers.cpp
    rounding_mode.c
    testHarness.c
    test_mt19937.c
    ThreadPool.c
    threadTesting.c
    typeWrappers.cpp
)

add_library(
    clconf_harness${CLConf_SUFFIX}
    STATIC
    ${sources}
)
if( DEFINED CLConf_MSVS_FOLDER )
    set_target_properties( "clconf_harness${CLConf_SUFFIX}" PROPERTIES FOLDER "${CLConf_MSVS_FOLDER}" )
endif()

set_source_files_properties( ${sources} PROPERTIES LANGUAGE CXX )

# end of file #
