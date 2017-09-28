add_subdirectory(type_conversion)
add_subdirectory(recorder)
add_subdirectory(debugger_test_type)
add_subdirectory(metadata_api_test_type)

include(${LLVM_PATH}/lib/cmake/llvm/AddLLVM.cmake)

# ugly if becaus ${CMAKE_CFG_INTDIR} doesn't work as expected
if (WIN32)
  set(OPENCL_BINARY_DIR ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
else(WIN32)
  set(OPENCL_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
endif(WIN32)
set(LLVM_BINARY_DIR ${LLVM_PATH}/bin)

# Let's help CMake to find llvm/utils/lit/lit.py
set(LLVM_MAIN_SRC_DIR "${OCL_SOURCE_DIR}/../../")

# We need to pass some cmake variables into lit .cfg scripts
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

list(APPEND BE_TEST_DEPS
  oclopt SATest
  # count not
  # llvm-as llvm-dis FileCheck
)

# It is used by lit.cfg to find lit.site.cfg when user launches LIT tests from a
# certain sub-directory instead of launching 'check-ocl-backend' target
set(BE_TEST_PARAMS
  be_lit_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

# -s: Show less output, for example, don't show information on tests that pass.
# -v: Show more information on tests failures, for example the entrie test
# output instead of just the test result.
set(BE_TEST_ARGS ${BE_TEST_ARGS} "-s" "-v")

add_custom_target(be-lit-deps DEPENDS ${BE_TEST_DEPS})
set_target_properties(be-lit-deps PROPERTIES FOLDER "Tests")

add_lit_testsuite(check-ocl-backend "Running the OpenCL Backend LIT tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS ${BE_TEST_PARAMS}
  DEPENDS ${BE_TEST_DEPS}
  ARGS ${BE_TEST_ARGS}
)
set_target_properties(check-ocl-backend PROPERTIES FOLDER "Tests")

option(ADD_NICE_BE_LIT_TARGETS "Generate cmake targets for launching BE LIT
  tests from each subdirectory separately" ON)

if (ADD_NICE_BE_LIT_TARGETS)
  # Note: These targets are not generated during multi-configuration
  # generators (i.e. Xcode and Visual Studio) because target clutter impacts
  # UI usability.
  #
  # Multi-configurations means that CMAKE_CONFIGURATION_TYPES is not empty.
  # Our root CMakeLists.txt set CMAKE_CONFIGURATION_TYPES to "Release;Debug",
  # so add_lit_testsuites does nothing.
  # To workaround this we need to reset CMAKE_CONFIGURATION_TYPES before call
  # to add_lit_testsuites and restore it after.
  set(CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})
  set(CMAKE_CONFIGURATION_TYPES "" CACHE STRING
      "Available build configurations." FORCE)
  add_lit_testsuites(ocl-backend
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARAMS ${BE_TEST_PARAMS}
    DEPENDS ${BE_TEST_DEPS}
    ARGS ${BE_TEST_ARGS}
  )
  set(CMAKE_CONFIGURATION_TYPES ${CONFIGURATION_TYPES} CACHE STRING
      "Available build configurations." FORCE)
endif(ADD_NICE_BE_LIT_TARGETS)
