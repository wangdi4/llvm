if (WIN32)
    if (${CMAKE_CFG_INTDIR} STREQUAL ".")
        string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
        if (${uppercase_CMAKE_BUILD_TYPE} STREQUAL "RELEASE")
            set(build_suffix "Release")
        elseif (${uppercase_CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
            set(build_suffix "Debug")
        else()
            message(FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE for TBB selection.")
        endif()

        set (TBB_BINARIES_OUTPUT_SRC_DIR
            ${CMAKE_CURRENT_SOURCE_DIR}/bin/VS2010${TBB_BINARIES_POSTFIX}/${OS}${ADDR}_${build_suffix}/)
    else()
        set (TBB_BINARIES_OUTPUT_SRC_DIR
            ${CMAKE_CURRENT_SOURCE_DIR}/bin/VS2010${TBB_BINARIES_POSTFIX}/${OS}${ADDR}_${CMAKE_CFG_INTDIR}/)
    endif()
    set (TBB_BINARIES_OUTPUT_DIR ${OCL_OUTPUT_LIBRARY_DIR}/tbb)

    # don't try to copy it using copy_to with Visual Studio Generator
    add_custom_target(tbb ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TBB_BINARIES_OUTPUT_SRC_DIR} ${TBB_BINARIES_OUTPUT_DIR}
        COMMENT "Copying tbb libraries from ${TBB_BINARIES_OUTPUT_SRC_DIR} to ${TBB_BINARIES_OUTPUT_DIR}")
else (WIN32)
    copy_to (${CMAKE_CURRENT_SOURCE_DIR}/bin/${OS}${ADDR}_${CMAKE_BUILD_TYPE}/ DESTINATION lib)
endif (WIN32)

