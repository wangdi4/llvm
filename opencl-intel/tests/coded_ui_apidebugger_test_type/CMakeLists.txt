# CMakeLists.txt #

if( WIN32 )

    # Building APIDebuggerTests.sln by VS 2013

    if ( DEBUG )
       set( MODE "debug" )
       set( OUT_SUB_DIR "Debug" )
    else()
       set( MODE "release" )
       set( OUT_SUB_DIR "Release" )
    endif()
    
    set( SRC_DIR "coded_ui_apidebugger_test_type" )
    # VS 2013 currently is not supported by build-sln.pl
    set( BUILD_CMD perl build-sln.pl "--${MODE}" --Win32 APIDebuggerTests.sln )

    execute_process(
        COMMAND ${BUILD_CMD}
        WORKING_DIRECTORY ${SRC_DIR}
        TIMEOUT 300
        RESULT_VARIABLE RESULT
    )

    # Check result of Build
    if ( "${RESULT}" STREQUAL "0" )
    else ()
         message( FATAL_ERROR "BUILD FAILLED: ${RESULT}" )
    endif()

    # Copying target files

    install(
        DIRECTORY bin/${OUT_SUB_DIR}/
        DESTINATION tests/APIDebugger
    )

endif( WIN32 )

# end of file #
