#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

# Test runner infrastructure for DTrans. This configures the DTrans test trees
# for use by Lit, and delegates to LLVM's lit test handlers.

set(DTRANS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  set(DTRANS_ENABLED 1)
else()
  set(DTRANS_ENABLED 0)
endif()

set(DTRANS_TEST_PARAMS
  dtrans_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  )

set(DTRANS_TEST_DEPENDS count
                        FileCheck
                        lli
                        llvm-as
                        llvm-config
                        llvm-lto
                        llvm-profdata
                        not
                        opt)

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  )

add_lit_testsuite(check-dtrans "Running the DTrans regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${DTRANS_TEST_DEPENDS}
  PARAMS ${DTRANS_TEST_PARAMS})
add_lit_testsuites(DTRANS ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${DTRANS_TEST_DEPENDS}
  PARAMS ${DTRANS_TEST_PARAMS})
set_target_properties(check-dtrans PROPERTIES FOLDER "DTrans tests")
