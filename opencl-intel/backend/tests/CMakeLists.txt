add_subdirectory(type_conversion)
add_subdirectory(debugger_test_type)

if (NOT OPENCL_INTREE_BUILD)
  include(${LLVM_PATH}/lib/cmake/llvm/AddLLVM.cmake)
endif()

# We need to pass some cmake variables into lit .cfg scripts
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

list(APPEND BE_TEST_DEPS
  oclopt
  # count not
  # llvm-as llvm-dis FileCheck
)

if (BACKEND_BUILD_VERIFICATION_LIB)
  list(APPEND BE_TEST_DEPS SATest)
  add_subdirectory(recorder)
endif (BACKEND_BUILD_VERIFICATION_LIB)

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
  if (NOT OPENCL_INTREE_BUILD)
    set(CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})
    set(CMAKE_CONFIGURATION_TYPES "" CACHE STRING
        "Available build configurations." FORCE)
  endif()

  add_lit_testsuites(ocl-backend
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARAMS ${BE_TEST_PARAMS}
    DEPENDS ${BE_TEST_DEPS}
    ARGS ${BE_TEST_ARGS}
  )

  if (NOT OPENCL_INTREE_BUILD)
    set(CMAKE_CONFIGURATION_TYPES ${CONFIGURATION_TYPES} CACHE STRING
        "Available build configurations." FORCE)
  endif()
endif(ADD_NICE_BE_LIT_TARGETS)
