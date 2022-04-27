include(FindPythonInterp)
if(PYTHONINTERP_FOUND)
  add_custom_target(check_pre_commit
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/pre_commit.py
      ${CMAKE_BINARY_DIR}
      ${CMAKE_CFG_INTDIR}
      -v
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running pre-commit tests. Please hold on..."
        SOURCES pre_commit.py)
	endif()
