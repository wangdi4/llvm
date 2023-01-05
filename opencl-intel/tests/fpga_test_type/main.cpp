//==--- main.cpp - tests for both FPGA HW and Emu              -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "common_utils.h"
#include "gtest_wrapper.h"
#include "options.hpp"
#include <CL/cl.h>

bool gCaptureMode = false;

cl_device_type gDeviceType = CL_DEVICE_TYPE_ACCELERATOR;

int main(int argc, char **argv) {
  CommandLineOption<bool> captureOption("--capture");
  // Pretend to be FPGA emulator anyway
  SETENV("CL_CONFIG_DEVICES", "fpga-emu");

  for (int i = 1; i < argc; i++) {
    if (captureOption.isMatch(argv[i])) {
      gCaptureMode = captureOption.getValue(argv[i]);
    }
  }

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
