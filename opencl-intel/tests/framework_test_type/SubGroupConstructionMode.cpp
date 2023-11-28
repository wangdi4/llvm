// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"

#define SG_SIZE 8

extern cl_device_type gDeviceType;

struct KernelDataOutput {
  KernelDataOutput(size_t Size)
      : Size(Size), WorkDim(Size), GlobalSize(Size), GlobalID(Size),
        LocalSize(Size), EnqueuedLocalSize(Size), LocalID(Size),
        NumGroups(Size), GroupID(Size), GlobalOffset(Size), LocalLinearID(Size),
        SGLocalID(Size) {}

  size_t Size;
  std::vector<cl_int> WorkDim;
  std::vector<cl_int3> GlobalSize;
  std::vector<cl_int3> GlobalID;
  std::vector<cl_int3> LocalSize;
  std::vector<cl_int3> EnqueuedLocalSize;
  std::vector<cl_int3> LocalID;
  std::vector<cl_int3> NumGroups;
  std::vector<cl_int3> GroupID;
  std::vector<cl_int3> GlobalOffset;
  std::vector<cl_int> LocalLinearID;
  std::vector<cl_int> SGLocalID;
};

static void verify(const KernelDataOutput &Res, size_t Dim,
                   const size_t *GlobalSizes, const size_t *LocalSizes,
                   int SGConstructionMode) {
  for (size_t I = 0; I < Res.Size; ++I) {
    EXPECT_EQ(Res.WorkDim[I], Dim)
        << "get_work_dim() doesn't match for item " << I;
    for (size_t J = 0; J < Dim; ++J) {
      EXPECT_EQ(Res.GlobalSize[I].s[J], GlobalSizes[J])
          << "get_global_size(" << J << ") doesn't match for item " << I;
      size_t LeadingDimSize = 1;
      for (size_t K = 0; K < J; ++K)
        LeadingDimSize *= GlobalSizes[K];
      size_t GID = I / LeadingDimSize % GlobalSizes[J];
      EXPECT_EQ(Res.GlobalID[I].s[J], GID)
          << "get_global_id(" << J << ") doesn't match for item " << I;
      EXPECT_EQ(Res.EnqueuedLocalSize[I].s[J], LocalSizes[J])
          << "get_enqueued_local_size(" << J << ") doesn't match for item "
          << I;
      size_t GroupID = GID / LocalSizes[J];
      EXPECT_EQ(Res.GroupID[I].s[J], GroupID)
          << "get_group_id(" << J << ") doesn't match for item " << I;
      size_t NumGroups = GlobalSizes[J] / LocalSizes[J];
      size_t RemainderSize = GlobalSizes[J] % LocalSizes[J];
      if (RemainderSize != 0)
        NumGroups++;
      else
        RemainderSize = LocalSizes[J];
      EXPECT_EQ(Res.NumGroups[I].s[J], NumGroups)
          << "get_num_groups(" << J << ") doesn't match for item " << I;
      bool IsRemainder = GroupID == NumGroups - 1;
      size_t CurrentLocalSize = IsRemainder ? RemainderSize : LocalSizes[J];
      EXPECT_EQ(Res.LocalSize[I].s[J], CurrentLocalSize)
          << "get_local_size(" << J << ") doesn't match for item " << I;
      EXPECT_EQ(Res.LocalID[I].s[J], GID % CurrentLocalSize)
          << "get_local_id(" << J << ") doesn't match for item " << I;
      EXPECT_EQ(Res.GlobalOffset[I].s[J], 0)
          << "get_global_offset(" << J << ") doesn't match for item " << I;
    }
    size_t LocalLinearID = 0;
    for (size_t J = 0; J < Dim; ++J) {
      size_t LeadingLocalSize = 1;
      for (size_t K = 0; K < J; ++K)
        LeadingLocalSize *= Res.LocalSize[I].s[K];
      LocalLinearID += Res.LocalID[I].s[J] * LeadingLocalSize;
    }
    EXPECT_EQ(Res.LocalLinearID[I], LocalLinearID)
        << "get_local_linear_id() doesn't match for item " << I;

    // Verify subgroup construction result
    if (SGConstructionMode == -1) {
      EXPECT_EQ(Res.SGLocalID[I], Res.LocalLinearID[I] % SG_SIZE)
          << "get_sub_group_local_id() doesn't match for item " << I;
    } else {
      EXPECT_EQ(Res.SGLocalID[I],
                Res.LocalID[I].s[SGConstructionMode] % SG_SIZE)
          << "get_sub_group_local_id() doesn't match for item " << I;
    }
  }
}

class SubGroupConstructionModeTest : public ::testing::TestWithParam<int> {
protected:
  cl_platform_id Platform;
  cl_device_id Device;
  cl_context Context;
  cl_command_queue Queue;

  const std::string Source = "__attribute__((intel_reqd_sub_group_size(" +
                             std::to_string(SG_SIZE) +
                             ")))"
                             R"(
    __kernel void basic(__global int *workdim,
                       __global int3 *gsize,
                       __global int3 *gid,
                       __global int3 *lsize,
                       __global int3 *enq_lsize,
                       __global int3 *lid,
                       __global int3 *groupnum,
                       __global int3 *groupid,
                       __global int3 *goffset,
                       __global int *llid,
                       __global int *sglid) {
      int glid = get_global_linear_id();
      workdim[glid] = get_work_dim();
      for (int i = 0; i < 3; ++i) {
        gsize[glid][i] = get_global_size(i);
        gid[glid][i] = get_global_id(i);
        lsize[glid][i] = get_local_size(i);
        enq_lsize[glid][i] = get_enqueued_local_size(i);
        lid[glid][i] = get_local_id(i);
        groupnum[glid][i] = get_num_groups(i);
        groupid[glid][i] = get_group_id(i);
        goffset[glid][i] = get_global_offset(i);
      }
      llid[glid] = get_local_linear_id();
      sglid[glid] = get_sub_group_local_id();
    }
  )";

  virtual void SetUp() override {
    // Set SG construction mode
    ASSERT_TRUE(SETENV("CL_CONFIG_CPU_SUB_GROUP_CONSTRUCTION",
                       std::to_string(GetParam()).c_str()))
        << "Failed to set SG construction mode.";
    cl_int Err = clGetPlatformIDs(1, &Platform, nullptr);
    ASSERT_OCL_SUCCESS(Err, "clGetPlatformIDs");

    Err = clGetDeviceIDs(Platform, gDeviceType, 1, &Device, nullptr);
    ASSERT_OCL_SUCCESS(Err, "clGetDeviceIDs");

    Context = clCreateContext(nullptr, 1, &Device, nullptr, nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateContext");

    Queue = clCreateCommandQueueWithProperties(Context, Device, nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateCommandQueueWithProperties");
  }

  void buildAndRun(KernelDataOutput &Dst, unsigned WorkDim,
                   const size_t *GlobalSizes, const size_t *LocalSizes) {
    cl_int Err;
    const char *S = Source.c_str();
    cl_program Program =
        clCreateProgramWithSource(Context, 1, &S, nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithSource");

    Err =
        clBuildProgram(Program, 1, &Device, "-cl-std=CL2.0", nullptr, nullptr);
    ASSERT_OCL_SUCCESS(Err, "clBuildProgram");

    cl_kernel Kernel = clCreateKernel(Program, "basic", &Err);
    EXPECT_OCL_SUCCESS(Err, "clCreateKernel");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 0, Dst.WorkDim.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(0)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 1, Dst.GlobalSize.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(1)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 2, Dst.GlobalID.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(2)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 3, Dst.LocalSize.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(3)");

    Err =
        clSetKernelArgMemPointerINTEL(Kernel, 4, Dst.EnqueuedLocalSize.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(4)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 5, Dst.LocalID.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(5)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 6, Dst.NumGroups.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(6)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 7, Dst.GroupID.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(7)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 8, Dst.GlobalOffset.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(8)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 9, Dst.LocalLinearID.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(9)");

    Err = clSetKernelArgMemPointerINTEL(Kernel, 10, Dst.SGLocalID.data());
    EXPECT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL(10)");

    Err = clEnqueueNDRangeKernel(Queue, Kernel, WorkDim, nullptr, GlobalSizes,
                                 LocalSizes, 0, nullptr, nullptr);
    EXPECT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

    Err = clFinish(Queue);
    EXPECT_OCL_SUCCESS(Err, "clFinish");

    Err = clReleaseKernel(Kernel);
    EXPECT_OCL_SUCCESS(Err, "clReleaseKernel");

    Err = clReleaseProgram(Program);
    ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  }

  virtual void TearDown() override {
    cl_int Err;
    if (Queue) {
      Err = clReleaseCommandQueue(Queue);
      ASSERT_OCL_SUCCESS(Err, "clReleaseCommandQueue");
    }
    if (Context) {
      Err = clReleaseContext(Context);
      ASSERT_OCL_SUCCESS(Err, "clReleaseContext");
    }
    ASSERT_TRUE(UNSETENV("CL_CONFIG_CPU_SUB_GROUP_CONSTRUCTION"))
        << "Failed to unset SG construction mode.";
  }
};

INSTANTIATE_TEST_SUITE_P(FrameworkTestType, SubGroupConstructionModeTest,
                         ::testing::Values(-1, 0, 1, 2));

TEST_P(SubGroupConstructionModeTest, UniformWG) {
  // Setup work sizes
  constexpr unsigned WorkDim = 3;
  size_t GlobalSizes[WorkDim] = {1, 4, 12};
  size_t LocalSizes[WorkDim] = {1, 2, 4};
  size_t LinearizedGlbSize = 1;
  for (unsigned I = 0; I < WorkDim; ++I)
    LinearizedGlbSize *= GlobalSizes[I];

  KernelDataOutput Result(LinearizedGlbSize);
  buildAndRun(Result, WorkDim, GlobalSizes, LocalSizes);

  verify(Result, WorkDim, GlobalSizes, LocalSizes, GetParam());
}
