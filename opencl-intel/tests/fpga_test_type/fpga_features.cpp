//===--- fpga_features.cpp -                                    -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for basic fpga features
//
// The test checks that we are able to compile program with FPGA stuff(channels,
// attrs), to create buffer with CL_CHANNEL_5_INTELFPGA flag.
//
// ===--------------------------------------------------------------------=== //

#include "base_fixture.h"

#include <gtest/gtest.h>
#include <CL/cl.h>

#include <string>

class TestFPGABasic : public OCLFPGABaseFixture {};

TEST_F(TestFPGABasic, ContextProperties) {
  cl_context context = createContext(devices());
  ASSERT_NE(nullptr, context) << "createContext failed";
}

TEST_F(TestFPGABasic, MemFlags) {
  cl_context context = createContext(device());
  ASSERT_NE(nullptr, context) << "createContext failed";

  cl_mem buf1 =
      createBuffer(context, sizeof(cl_int), CL_CHANNEL_AUTO_INTELFPGA);
  ASSERT_NE(nullptr, buf1) << "createBuffer failed";

  cl_mem buf2 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_1_INTELFPGA);
  ASSERT_NE(nullptr, buf2) << "createBuffer failed";

  cl_mem buf3 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_2_INTELFPGA);
  ASSERT_NE(nullptr, buf3) << "createBuffer failed";

  cl_mem buf4 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_3_INTELFPGA);
  ASSERT_NE(nullptr, buf4) << "createBuffer failed";

  cl_mem buf5 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_4_INTELFPGA);
  ASSERT_NE(nullptr, buf5) << "createBuffer failed";

  cl_mem buf6 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_5_INTELFPGA);
  ASSERT_NE(nullptr, buf6) << "createBuffer failed";

  cl_mem buf7 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_6_INTELFPGA);
  ASSERT_NE(nullptr, buf7) << "createBuffer failed";

  cl_mem buf8 = createBuffer(context, sizeof(cl_int), CL_CHANNEL_7_INTELFPGA);
  ASSERT_NE(nullptr, buf8) << "createBuffer failed";

  cl_mem buf9 =
      createBuffer(context, sizeof(cl_int), CL_MEM_HETEROGENEOUS_INTELFPGA);
  ASSERT_NE(nullptr, buf9) << "createBuffer failed";
}
