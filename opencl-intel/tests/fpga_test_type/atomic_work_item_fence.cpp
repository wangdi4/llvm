//===--- atomic_work_item_fence.cpp -                           -*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for memory scope on FPGA-emu
//
// ===--------------------------------------------------------------------=== //
#include "CL/cl.h"
#include "gtest_wrapper.h"
#include "simple_fixture.h"
#include "test_utils.h"

#include <string>

class TestAtomicWorkItemFence : public OCLFPGASimpleFixture {};

// kernel which use unsupported memory scope on FPGA-emu
static const std::vector<std::string> program_sources01 = {
    "__kernel void saxpy_kernel(__global float *A,                                              \n\
                                __global float *B,                                              \n\
                                __global float *C){                                             \n\
        int index = get_global_id(0);                                                           \n\
        C[index] = A[index] + B[index];                                                         \n\
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release                       \n\
        , memory_scope_all_svm_devices);                                                        \n\
    }                                                                                           \n\
    "};

// kernels which use supported memory scope on FPGA-emu
// memory_scope_sub_group is missing before version 3.0.
static const std::vector<std::string> program_sources02 = {
    "__kernel void saxpy_kernel(__global float *A,                                              \n\
                                __global float *B,                                              \n\
                                __global float *C){                                             \n\
        int index = get_global_id(0);                                                           \n\
        C[index] = A[index] + B[index];                                                         \n\
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release                       \n\
        , memory_scope_work_item);                                                              \n\
    }                                                                                           \n\
    ",
    "__kernel void saxpy_kernel(__global float *A,                                              \n\
                                __global float *B,                                              \n\
                                __global float *C){                                             \n\
        int index = get_global_id(0);                                                           \n\
        C[index] = A[index] + B[index];                                                         \n\
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release                       \n\
        , memory_scope_work_group);                                                             \n\
    }                                                                                           \n\
    ",
    "__kernel void saxpy_kernel(__global float *A,                                              \n\
                                __global float *B,                                              \n\
                                __global float *C){                                             \n\
        int index = get_global_id(0);                                                           \n\
        C[index] = A[index] + B[index];                                                         \n\
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release                       \n\
        , memory_scope_device);                                                                 \n\
    }                                                                                           \n\
    "};

static const char *error_msg =
    "memory_scope::system are not supported on FPGA emulator platform!";

TEST_F(TestAtomicWorkItemFence, UnsupportedFPGAMemoryScope) {
  for (size_t idx = 0; idx < program_sources01.size(); ++idx) {
    cl_int error = CL_SUCCESS;

    const char *sources_str = program_sources01[idx].c_str();
    cl_program program_with_source = clCreateProgramWithSource(
        getContext(), 1, &sources_str, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clCreateProgramWithSource failed with error " << ErrToStr(error);

    // Build the program which use unsupported memory scope on FPGA-emu
    EXPECT_NO_FATAL_FAILURE(error = clBuildProgram(program_with_source, 0,
                                                   nullptr, "-cl-std=CL2.0",
                                                   nullptr, nullptr));

    // Get build log and check if the log contains the expected string
    std::string build_log;
    GetBuildLog(getDevice(), program_with_source, build_log);

    ASSERT_NE(std::string::npos, build_log.find(error_msg));

    // Release the program
    clReleaseProgram(program_with_source);
  }
}

TEST_F(TestAtomicWorkItemFence, SupportedFPGAMemoryScope) {
  for (size_t idx = 0; idx < program_sources02.size(); ++idx) {
    cl_int error = CL_SUCCESS;

    const char *sources_str = program_sources02[idx].c_str();
    cl_program program_with_source = clCreateProgramWithSource(
        getContext(), 1, &sources_str, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clCreateProgramWithSource failed with error " << ErrToStr(error);

    // Build the program which use supported memory scope on FPGA-emu
    error = clBuildProgram(program_with_source, 0, nullptr, "-cl-std=CL2.0",
                           nullptr, nullptr);

    // Expect compilation to be successful
    ASSERT_EQ(CL_SUCCESS, error)
        << "clBuildProgram failed with error " << ErrToStr(error);

    // Release the program
    clReleaseProgram(program_with_source);
  }
}
