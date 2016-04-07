if (WIN32)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        install(DIRECTORY bin/VS2008/${OS}${ADDR}_\${CMAKE_INSTALL_CONFIG_NAME}/ DESTINATION bin/tbb)
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010
        install(DIRECTORY bin/VS2010${EXPERIMENTAL_POSTFIX}/${OS}${ADDR}_\${CMAKE_INSTALL_CONFIG_NAME}/ DESTINATION bin/tbb)
    endif (${MSVC_VERSION} EQUAL 1500)
else (WIN32)
    install(DIRECTORY bin/${OS}${ADDR}_\${CMAKE_INSTALL_CONFIG_NAME}/ DESTINATION bin)
endif (WIN32)
