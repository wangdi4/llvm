//==--- CompilationUtilsTest.cpp - test for CompilationUtils -*- C++ -*----==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===---------------------------------------------------------------------==//

#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "gtest_wrapper.h"

using namespace llvm::CompilationUtils;

class CompilationUtilsTest : public ::testing::Test {};

TEST_F(CompilationUtilsTest, PipeKindReadWrite) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);

  Kind = getPipeKind("__write_pipe_2");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);

  Kind = getPipeKind("__write_pipe_4");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWriteReserve, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);
}

TEST_F(CompilationUtilsTest, PipeKindReadWriteBlocking) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2_bl");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.Blocking);

  Kind = getPipeKind("__write_pipe_2_bl");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.Blocking);

  Kind = getPipeKind("__write_pipe_4_bl");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWriteReserve, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.Blocking);
}

TEST_F(CompilationUtilsTest, PipeKindReadWriteSimd) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2_bl_fpga_v16f32");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_EQ("v16f32", Kind.SimdSuffix);
  ASSERT_TRUE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);

  Kind = getPipeKind("__write_pipe_2_bl_fpga_v8i32");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_EQ("v8i32", Kind.SimdSuffix);
  ASSERT_TRUE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);

  Kind = getPipeKind("__write_pipe_4_bl");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWriteReserve, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.Blocking);
}

TEST_F(CompilationUtilsTest, PipeKindCommit) {
  PipeKind Kind;

  Kind = getPipeKind("__commit_read_pipe");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::Commit, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);

  Kind = getPipeKind("__commit_write_pipe");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::Commit, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);
}

TEST_F(CompilationUtilsTest, PipeKindReserve) {
  PipeKind Kind;

  Kind = getPipeKind("__reserve_read_pipe");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::Reserve, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);

  Kind = getPipeKind("__reserve_write_pipe");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::Reserve, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);
}

TEST_F(CompilationUtilsTest, PipeKindScope) {
  PipeKind Kind;

  Kind = getPipeKind("__sub_group_commit_read_pipe");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::Commit, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::SubGroup, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);

  Kind = getPipeKind("__work_group_commit_read_pipe");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::Commit, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkGroup, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);
}

TEST_F(CompilationUtilsTest, PipeKindInternal) {
  PipeKind Kind;

  Kind = getPipeKind("__global_pipe_ctor");
  ASSERT_FALSE(Kind);

  Kind = getPipeKind("__pipe_init");
  ASSERT_FALSE(Kind);

  Kind = getPipeKind("__pipe_init_array");
  ASSERT_FALSE(Kind);
}

TEST_F(CompilationUtilsTest, PipeKindReadWriteIO) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2_io_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.IO);
  ASSERT_FALSE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);

  Kind = getPipeKind("__write_pipe_2_io_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.IO);
  ASSERT_FALSE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);
}

TEST_F(CompilationUtilsTest, PipeKindReadWriteBlockingIO) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2_bl_io_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.IO);
  ASSERT_TRUE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);

  Kind = getPipeKind("__write_pipe_2_bl_io_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.IO);
  ASSERT_TRUE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);
}

TEST_F(CompilationUtilsTest, PipeKindReadWriteFPGA) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);

  Kind = getPipeKind("__write_pipe_2_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_FALSE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);
}

TEST_F(CompilationUtilsTest, PipeKindReadWriteBlockingFPGA) {
  PipeKind Kind;

  Kind = getPipeKind("__read_pipe_2_bl_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Read, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);

  Kind = getPipeKind("__write_pipe_2_bl_fpga");
  ASSERT_TRUE(Kind);
  ASSERT_EQ(PipeKind::OpKind::ReadWrite, Kind.Op);
  ASSERT_EQ(PipeKind::AccessKind::Write, Kind.Access);
  ASSERT_EQ(PipeKind::ScopeKind::WorkItem, Kind.Scope);
  ASSERT_TRUE(Kind.Blocking);
  ASSERT_TRUE(Kind.FPGA);
}
