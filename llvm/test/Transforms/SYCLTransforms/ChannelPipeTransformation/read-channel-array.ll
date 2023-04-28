; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

; Compiled from OpenCL kernel:
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel int c4[4][8][16][2];
; kernel void test() {
;   int val0 = read_channel_intel(c4[0][0][0][0]);
;   int val1 = read_channel_intel(c4[0][0][0][1]);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@c4 = addrspace(1) global [4 x [8 x [16 x [2 x ptr addrspace(1)]]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

define dso_local void @test() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
; CHECK: load ptr addrspace(1), ptr addrspace(1) @c4.pipe, align 8
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1)
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 0, i64 0, i64 1), align 8
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1)
  %0 = load ptr addrspace(1), ptr addrspace(1) @c4, align 4, !tbaa !3
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #2
  %1 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x [16 x [2 x ptr addrspace(1)]]]], ptr addrspace(1) @c4, i64 0, i64 0, i64 0, i64 0, i64 1), align 4, !tbaa !3
  %call1 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %1) #2
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind }

!sycl.kernels = !{!1}

!0 = !{i32 4}
!1 = !{ptr @test}
!2 = !{}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %read.dst = alloca i32, align 4
; DEBUGIFY-COUNT-1026: Instruction with empty DebugLoc in function __pipe_global_ctor
; DEBUGIFY-NOT: WARNING
