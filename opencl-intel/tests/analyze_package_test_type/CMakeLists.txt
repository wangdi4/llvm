
if(WIN32)
    set( CREATE_DIR_NAME "tests/analyze_package_test_type" )

    install(FILES AnalysingSystemTester.dll DESTINATION ${CREATE_DIR_NAME})

    add_subdirectory( CallAllApi )

endif(WIN32)