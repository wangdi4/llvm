//==--- max_min_num_workers.cpp - test for OCL_TBB_NUM_WORKER            ---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"

#include "FrameworkTest.h"
#include "common_utils.h"

#include <gtest/gtest.h>

extern cl_device_type gDeviceType;

/*
 * max_min_num_workers test.
 *
 * Test tries to create context with various values of the OCL_TBB_NUM_WORKERS
 * env variable.
 *
 * Expected result: context is created successfully without hanging of
 * application
 */

static void tryCreateContext()
{
    cl_int err = 0;
    cl_platform_id platform = nullptr;

    err = clGetPlatformIDs(1, &platform, nullptr);
    ASSERT_EQ(CL_SUCCESS, err) << "clGetPlatformIDs failed.";

    cl_device_id device = nullptr;
    err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
    ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceIDs failed.";

    cl_context context =
        clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    ASSERT_EQ(CL_SUCCESS, err) << "clCreateContext failed.";

    clReleaseContext(context);
}

void maxTBBNumWorkers()
{
    SETENV("OCL_TBB_NUM_WORKERS", "256");
    tryCreateContext();
    UNSETENV("OCL_TBB_NUM_WORKERS");
}

void minTBBNumWorkers()
{
    SETENV("OCL_TBB_NUM_WORKERS", "2");
    tryCreateContext();
    UNSETENV("OCL_TBB_NUM_WORKERS");
}

void moreThanMaxTBBNumWorkers()
{
    SETENV("OCL_TBB_NUM_WORKERS", "4096");
    tryCreateContext();
    UNSETENV("OCL_TBB_NUM_WORKERS");
}

void lessThanMinTBBNumWorkers()
{
    SETENV("OCL_TBB_NUM_WORKERS", "1");
    tryCreateContext();
    UNSETENV("OCL_TBB_NUM_WORKERS");
}

void invalidTBBNumWorkers()
{
    SETENV("OCL_TBB_NUM_WORKERS", "-42");
    tryCreateContext();
    UNSETENV("OCL_TBB_NUM_WORKERS");
}
