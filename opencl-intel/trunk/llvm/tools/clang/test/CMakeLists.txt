set(CLANG_TEST_DIRECTORIES
  "Analysis"
  "CodeCompletion"
  "CodeGen"
  "CodeGenCXX"
  "CodeGenObjC"
  "CodeGenOpenCL"
  "Coverage"
  "CXX"
  "Driver"
  "FixIt"
  "Frontend"
  "Headers"
  "Index"
  "Lexer"
  "Misc"
  "PCH"
  "Parser"
  "Preprocessor"
  "OpenCL_Restrictions"
  "Rewriter"
  "Sema"
  "SemaCXX"
  "SemaObjC"
  "SemaObjCXX"
  "SemaOpenCL"
  "SemaTemplate")

set(LLVM_SOURCE_DIR "${LLVM_MAIN_SRC_DIR}")
set(LLVM_BINARY_DIR "${LLVM_BINARY_DIR}")
set(LLVM_TOOLS_DIR "${LLVM_TOOLS_BINARY_DIR}/%(build_config)s")
set(LLVM_LIBS_DIR "${LLVM_BINARY_DIR}/lib/%(build_config)s")
set(CLANG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CLANG_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/..")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg)

include(FindPythonInterp)
if(PYTHONINTERP_FOUND)
  set(CLANG_TEST_EXTRA_ARGS)
  if (MSVC OR XCODE)
    set(CLANG_TEST_EXTRA_ARGS "--no-progress-bar")
  endif()

  option(CLANG_TEST_USE_VG "Run Clang tests under Valgrind" OFF)
  if(CLANG_TEST_USE_VG)
    set(CLANG_TEST_EXTRA_ARGS ${CLANG_TEST_EXTRA_ARGS} "--vg")
  endif ()

  foreach(testdir ${CLANG_TEST_DIRECTORIES})
    add_custom_target(clang-test-${testdir}
      COMMAND set CLANG=${LLVM_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/clang
      COMMAND ${PYTHON_EXECUTABLE}
                  ${LLVM_SOURCE_DIR}/utils/lit/lit.py
                  --param clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
                  --param build_config=${CMAKE_CFG_INTDIR}
                  -sv ${CLANG_TEST_EXTRA_ARGS}
                  ${CMAKE_CURRENT_BINARY_DIR}/${testdir}
                  DEPENDS clang c-index-test FileCheck not count
                  COMMENT "Running Clang regression tests in ${testdir}")
  endforeach()
  
  add_custom_target(clang-test-OpenCL
      COMMAND set CLANG=${LLVM_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/clang	  
	  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} 7z x OpenCL.7z
      COMMAND ${PYTHON_EXECUTABLE}
                  ${LLVM_SOURCE_DIR}/utils/lit/lit.py
                  --param clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
                  --param build_config=${CMAKE_CFG_INTDIR}
                  -sv ${CLANG_TEST_EXTRA_ARGS}
                  ${CMAKE_CURRENT_BINARY_DIR}/OpenCL
                  DEPENDS clang c-index-test FileCheck not count
                  COMMENT "Running Clang regression tests in ${testdir}"
	  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/OpenCL)
				  
  add_custom_target(clang-test
    COMMAND set CLANG=${LLVM_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/clang
    COMMAND ${PYTHON_EXECUTABLE}
                ${LLVM_SOURCE_DIR}/utils/lit/lit.py
                --param clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
                --param build_config=${CMAKE_CFG_INTDIR}
                -sv ${CLANG_TEST_EXTRA_ARGS}
                ${CMAKE_CURRENT_BINARY_DIR}
                DEPENDS clang c-index-test FileCheck not count
                COMMENT "Running Clang regression tests")

  add_custom_target(clang-c++tests
    COMMAND set CLANG=${LLVM_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/clang
    COMMAND ${PYTHON_EXECUTABLE}
                ${LLVM_SOURCE_DIR}/utils/lit/lit.py
                --param clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
                --param build_config=${CMAKE_CFG_INTDIR}
                -sv ${CLANG_TEST_EXTRA_ARGS}
                ${CMAKE_CURRENT_SOURCE_DIR}/../utils/C++Tests
                DEPENDS clang c-index-test FileCheck not count
                COMMENT "Running Clang regression tests")
endif()
