if (WIN32)
    set (TBB_BINARIES_OUTPUT_SRC_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/VS2010${TBB_BINARIES_POSTFIX}/${OS}${ADDR}_${CMAKE_CFG_INTDIR}/)
    set (TBB_BINARIES_OUTPUT_DIR ${OCL_OUTPUT_LIBRARY_DIR}/tbb)

    # don't try to copy it using copy_to with Visual Studio Generator
    add_custom_target(tbb ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TBB_BINARIES_OUTPUT_SRC_DIR} ${TBB_BINARIES_OUTPUT_DIR}
        COMMENT "Copying tbb libraries from ${TBB_BINARIES_OUTPUT_SRC_DIR} to ${TBB_BINARIES_OUTPUT_DIR}")

    # TODO: remove DRY_RUN when build scripts will be aligned with new layout
    if (DRY_RUN)
        install_to (${TBB_BINARIES_OUTPUT_DIR} DESTINATION bin)
    else ()
        install_to (${TBB_BINARIES_OUTPUT_DIR} DESTINATION lib)
    endif (DRY_RUN)
else (WIN32)
    copy_to (${CMAKE_CURRENT_SOURCE_DIR}/bin/${OS}${ADDR}_${CMAKE_BUILD_TYPE}/ DESTINATION lib)
endif (WIN32)

