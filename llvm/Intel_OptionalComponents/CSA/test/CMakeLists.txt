#
#    Copyright (C) 2019 Intel Corporation. All rights reserved.
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

# Test runner infrastructure for CSA.

set(CSA_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CSA_TEST_PARAMS
  csa_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  )

set(CSA_TEST_DEPENDS opt)

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  )

add_lit_testsuite(check-csa "Running the CSA regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CSA_TEST_DEPENDS}
  PARAMS ${CSA_TEST_PARAMS})
add_lit_testsuites(CSA ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CSA_TEST_DEPENDS}
  PARAMS ${CSA_TEST_PARAMS})
set_target_properties(check-csa PROPERTIES FOLDER "CSA tests")
