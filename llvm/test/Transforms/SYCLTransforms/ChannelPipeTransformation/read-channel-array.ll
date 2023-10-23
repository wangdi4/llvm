; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

; Compiled from OpenCL kernel and dumped before sycl-kernel-channel-pipe-transformation pass:
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel int c4[4][8][16][2];
; kernel void test() {
;   int val0 = read_channel_intel(c4[0][0][0][0]);
;   int val1 = read_channel_intel(c4[0][0][0][1]);
;   int val2 = read_channel_intel(c4[3][2][1][0]);
;   int val3 = read_channel_intel(c4[3][2][0][0]);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @c4 = addrspace(1) global [4 x [8 x [16 x [2 x ptr addrspace(1)]]]] zeroinitializer, align 8, !packet_size
; CHECK: @c4.bs = addrspace(1) global [466944 x i8] zeroinitializer, align 4
; CHECK: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]

@c4 = addrspace(1) global [4 x [8 x [16 x [2 x ptr addrspace(1)]]]] zeroinitializer, align 8, !packet_size !0, !packet_align !0

define dso_local void @test() #0 {
entry:
; CHECK: load ptr addrspace(1), ptr addrspace(1) @c4, align 8
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1)
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 0, i64 0, i64 0, i64 1), align 8
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1)
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 3, i64 2, i64 1), align 8
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1)
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 3, i64 2), align 8
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1)

  %0 = load ptr addrspace(1), ptr addrspace(1) @c4, align 8
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #1
  %1 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 0, i64 0, i64 0, i64 1), align 8
  %call1 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %1) #1
  %2 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 3, i64 2, i64 1), align 8
  %call2 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %2) #1
  %3 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 3, i64 2), align 8
  %call3 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %3) #1
  ret void
}

declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }

!sycl.kernels = !{!1}

!0 = !{i32 4}
!1 = !{ptr @test}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %read.dst = alloca i32, align 4
; DEBUGIFY-COUNT-20: Instruction with empty DebugLoc in function __pipe_global_ctor
; DEBUGIFY-NOT: WARNING
