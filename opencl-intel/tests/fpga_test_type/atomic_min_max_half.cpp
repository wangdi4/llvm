//===--- atomic_min_max_half.cpp -                              -*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for half precision atomic_min/atomic_max builtins
//
// ===--------------------------------------------------------------------=== //
#include "CL/cl.h"
#include "CL/cl_half.h"
#include "gtest_wrapper.h"
#include "simple_fixture.h"

#include <string>
#include <vector>

class TestAtomicMinMaxHalf : public OCLFPGASimpleFixture {
protected:
  bool check_min_results(const std::vector<cl_half> &src_a,
                         const std::vector<cl_half> &src_b,
                         const std::vector<cl_half> &r_old,
                         const std::vector<cl_half> &r_cmp) {
    for (size_t i = 0; i < src_a.size(); i++) {
      if (src_a[i] != r_old[i] &&
          !((src_a[i] & 0x7fff) > 0x7c00 && (r_old[i] & 0x7fff) > 0x7c00)) {
        printf("Old value error at %zu: got 0x%04x, expected 0x%04x\n", i,
               r_old[i], src_a[i]);
        return false;
      }

      cl_half exp_cmp = cl_half_to_float(src_a[i]) < cl_half_to_float(src_b[i])
                            ? src_a[i]
                            : src_b[i];

      if (exp_cmp != r_cmp[i] &&
          !((src_a[i] & 0x7fff) > 0x7c00 && (r_old[i] & 0x7fff) > 0x7c00)) {
        printf("Comparison error at %zu: got 0x%04x, expected 0x%04x\n", i,
               r_cmp[i], exp_cmp);
        return false;
      }
    }
    return true;
  }

  bool check_max_results(const std::vector<cl_half> &src_a,
                         const std::vector<cl_half> &src_b,
                         const std::vector<cl_half> &r_old,
                         const std::vector<cl_half> &r_cmp) {
    for (size_t i = 0; i < src_a.size(); i++) {
      if (src_a[i] != r_old[i] &&
          !((src_a[i] & 0x7fff) > 0x7c00 && (r_old[i] & 0x7fff) > 0x7c00)) {
        printf("Old value error at %zu: got 0x%04x, expected 0x%04x\n", i,
               r_old[i], src_a[i]);
        return false;
      }

      cl_half exp_cmp = cl_half_to_float(src_a[i]) > cl_half_to_float(src_b[i])
                            ? src_a[i]
                            : src_b[i];

      if (exp_cmp != r_cmp[i] &&
          !((src_a[i] & 0x7fff) > 0x7c00 && (r_old[i] & 0x7fff) > 0x7c00)) {
        printf("Comparison error at %zu: got 0x%04x, expected 0x%04x\n", i,
               r_cmp[i], exp_cmp);
        return false;
      }
    }
    return true;
  }
};

static const std::vector<std::string> program_sources = {
    "#pragma OPENCL EXTENSION cl_khr_fp16 : enable                           \n\
    half __attribute__((overloadable))                                       \n\
    atomic_min(volatile __global half *p, half val);                         \n\
    __kernel void test_atomic_min_h(__global half *src_a,                    \n\
                                    __global half *src_b,                    \n\
                                    __global half *r_old,                    \n\
                                    __global half *r_cmp) {                  \n\
      int tid = get_global_id(0);                                            \n\
      r_cmp[tid] = src_a[tid];                                               \n\
      r_old[tid] = atomic_min(&(r_cmp[tid]), src_b[tid]);                    \n\
    }                                                                        \n\
                                                                             \n\
    half __attribute__((overloadable))                                       \n\
    atomic_max(volatile __global half *p, half val);                         \n\
    __kernel void test_atomic_max_h(__global half *src_a,                    \n\
                                    __global half *src_b,                    \n\
                                    __global half *r_old,                    \n\
                                    __global half *r_cmp) {                  \n\
      int tid = get_global_id(0);                                            \n\
      r_cmp[tid] = src_a[tid];                                               \n\
      r_old[tid] = atomic_max(&(r_cmp[tid]), src_b[tid]);                    \n\
    }                                                                        \n\
  ",
    "#pragma OPENCL EXTENSION cl_khr_fp16 : enable                           \n\
    __kernel void test_atomic_min_h(__global half *src_a,                    \n\
                                    __global half *src_b,                    \n\
                                    __global half *r_old,                    \n\
                                    __global atomic_half *r_cmp) {           \n\
      int tid = get_global_id(0);                                            \n\
      atomic_store_explicit(&(r_cmp[tid]), src_a[tid], memory_order_relaxed);\n\
      r_old[tid] = atomic_fetch_min_explicit(&(r_cmp[tid]), src_b[tid],      \n\
      memory_order_relaxed, memory_scope_device);                            \n\
    }                                                                        \n\
                                                                             \n\
    __kernel void test_atomic_max_h(__global half *src_a,                    \n\
                                    __global half *src_b,                    \n\
                                    __global half *r_old,                    \n\
                                    __global atomic_half *r_cmp) {           \n\
      int tid = get_global_id(0);                                            \n\
      atomic_store_explicit(&(r_cmp[tid]), src_a[tid], memory_order_relaxed);\n\
      r_old[tid] = atomic_fetch_max_explicit(&(r_cmp[tid]), src_b[tid],      \n\
      memory_order_relaxed, memory_scope_device);                            \n\
    }                                                                        \n\
    "};
static const std::vector<std::string> options = {"-cl-std=CL1.2",
                                                 "-cl-std=CL2.0"};

TEST_F(TestAtomicMinMaxHalf, Basic) {
  for (size_t idx = 0; idx < program_sources.size(); ++idx) {
    ASSERT_TRUE(createAndBuildProgram(program_sources[idx], options[idx]))
        << "createAndBuildProgram failed";

    std::vector<cl_half> src_a, src_b;
    const std::vector<float> src_a_f = {1.23f,      0.00023f, 213.3452f,
                                        -23.12213f, 0.f,      -0.f};
    const std::vector<float> src_b_f = {
        56.23f, 0.00621f, 0.0000023f, 245.345f, 10.f, 10.f, 1.f, 1.f, 1.f};
    for (size_t i = 0; i < src_a_f.size(); i++)
      src_a.push_back(cl_half_from_float(src_a_f[i], CL_HALF_RTE));
    for (size_t i = 0; i < src_b_f.size(); i++)
      src_b.push_back(cl_half_from_float(src_b_f[i], CL_HALF_RTE));
    std::vector<cl_half> limits_a = {0x7BFF, 0x0001, 0x7C00, 0x7C01};
    src_a.insert(src_a.end(), limits_a.begin(), limits_a.end());
    auto it = src_b.begin() + 7;
    src_b.insert(it, 0x8001);
    const size_t num = src_a.size();

    cl_mem input_buffer_a = createBuffer<cl_half>(
        num, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &src_a.front());
    ASSERT_NE(nullptr, input_buffer_a) << "createBuffer failed";
    cl_mem input_buffer_b = createBuffer<cl_half>(
        num, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &src_b.front());
    ASSERT_NE(nullptr, input_buffer_b) << "createBuffer failed";

    cl_mem min_o = createBuffer<cl_half>(num, CL_MEM_READ_WRITE);
    ASSERT_NE(nullptr, min_o) << "createBuffer failed";
    cl_mem min_cmp = createBuffer<cl_half>(num, CL_MEM_READ_WRITE);
    ASSERT_NE(nullptr, min_cmp) << "createBuffer failed";
    cl_mem max_o = createBuffer<cl_half>(num, CL_MEM_READ_WRITE);
    ASSERT_NE(nullptr, max_o) << "createBuffer failed";
    cl_mem max_cmp = createBuffer<cl_half>(num, CL_MEM_READ_WRITE);
    ASSERT_NE(nullptr, max_cmp) << "createBuffer failed";

    ASSERT_TRUE(enqueueNDRange("test_atomic_min_h", 1, &num, NULL,
                               /*event*/ nullptr, input_buffer_a,
                               input_buffer_b, min_o, min_cmp))
        << "enqueueNDRange failed";
    ASSERT_TRUE(enqueueNDRange("test_atomic_max_h", 1, &num, NULL,
                               /*event*/ nullptr, input_buffer_a,
                               input_buffer_b, max_o, max_cmp))
        << "enqueueNDRange failed";

    std::vector<cl_half> min_out(num);
    std::vector<cl_half> min_cmpare(num);
    std::vector<cl_half> max_out(num);
    std::vector<cl_half> max_cmpare(num);
    ASSERT_TRUE(
        readBuffer<cl_half>("test_atomic_min_h", min_o, num, &min_out.front()))
        << "readBuffer failed";
    ASSERT_TRUE(readBuffer<cl_half>("test_atomic_min_h", min_cmp, num,
                                    &min_cmpare.front()))
        << "readBuffer failed";
    ASSERT_TRUE(
        readBuffer<cl_half>("test_atomic_max_h", max_o, num, &max_out.front()))
        << "readBuffer failed";
    ASSERT_TRUE(readBuffer<cl_half>("test_atomic_max_h", max_cmp, num,
                                    &max_cmpare.front()))
        << "readBuffer failed";

    ASSERT_TRUE(check_min_results(src_a, src_b, min_out, min_cmpare))
        << "Results differ from expected in check_atomic_min\n";

    ASSERT_TRUE(check_max_results(src_a, src_b, max_out, max_cmpare))
        << "Results differ from expected in check_atomic_max\n";
  }
}
