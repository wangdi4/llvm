; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

; Compiled from OpenCL kernel:
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int c2[4];
; channel int c4[4][8];
; channel int c8;
;
; __kernel void for_channel() { int val4 = read_channel_intel(c2[0]); }
;
; __kernel void for_pipe(write_only pipe int __attribute__((blocking)) c0) {
;   for (int i = 0; i < 10; i++) {
;     write_pipe(c0, &i);
;   }
; }
;
; __attribute__((noinline)) void channel_func() {
;   int val4 = read_channel_intel(c4[0][0]);
; }
;
; __kernel void for_global_id(__global int *data) {
;   int gid = get_global_id(0);
;   data[gid] = 42;
; }
;
; __attribute__((noinline)) __kernel void for_channel_ver2() { channel_func(); }
;
; __kernel void for_channel_ver3() {
;   int val4 = read_channel_intel(c8);
; }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@c2.pipe = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c2.pipe.bs = addrspace(1) global [1312 x i8] zeroinitializer, align 4
@c4.pipe = addrspace(1) global [4 x [8 x ptr addrspace(1)]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [10496 x i8] zeroinitializer, align 4
@c8.pipe = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@c8.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent norecurse nounwind
define dso_local void @for_channel() local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
; CHECK-LABEL: @for_channel(
; CHECK-SAME: !use_fpga_pipes ![[TRUE:[0-9]+]]
; CHECK: alloca ptr addrspace(1), i32 4, align 8
; CHECK: alloca ptr addrspace(1), i32 4, align 8

  %read.dst = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c2.pipe, align 16
  %1 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4) #5
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @channel_func() unnamed_addr #2 {
entry:
; CHECK-LABEL: @channel_func(
; CHECK: alloca ptr addrspace(1), i32 32, align 8
; CHECK: alloca ptr addrspace(1), i32 32, align 8

  %read.dst = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c4.pipe, align 16
  %1 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4) #5
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @for_global_id(ptr addrspace(1) noundef align 4 %data) local_unnamed_addr #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_name !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !arg_type_null_val !10 {
entry:
; CHECK-LABEL: @for_global_id(
; CHECK-SAME: !use_fpga_pipes ![[FALSE:[0-9]+]]
; CHECK-NOT: alloca ptr addrspace(1), i32

  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #6
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %data, i64 %idxprom
  store i32 42, ptr addrspace(1) %arrayidx, align 4, !tbaa !11
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #3

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @for_channel_ver2() local_unnamed_addr #2 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
; CHECK-LABEL: @for_channel_ver2(
; CHECK-SAME: !use_fpga_pipes ![[TRUE:[0-9]+]]
; CHECK-NOT: alloca ptr addrspace(1), i32

  tail call fastcc void @channel_func() #7
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @for_channel_ver3() local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
; CHECK-LABEL: @for_channel_ver3(
; CHECK-SAME: !use_fpga_pipes ![[TRUE:[0-9]+]]
; CHECK: alloca ptr addrspace(1), align 8
; CHECK: alloca ptr addrspace(1), align 8

  %read.dst = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c8.pipe, align 8
  %1 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4) #5
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @for_pipe(ptr addrspace(1) %c0) local_unnamed_addr #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !15 !kernel_arg_type !16 !kernel_arg_base_type !16 !kernel_arg_type_qual !17 !kernel_arg_name !18 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !arg_type_null_val !19 {
for.cond.cleanup:
; CHECK-LABEL: @for_pipe(
; CHECK-SAME: !use_fpga_pipes ![[TRUE:[0-9]+]]
; CHECK: alloca ptr addrspace(1), align 8
; CHECK: alloca ptr addrspace(1), align 8

  %i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #5
  %0 = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %1 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 1, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %2 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 2, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 3, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %4 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 4, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %5 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 5, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %6 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 6, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %7 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 7, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %8 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 8, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %9 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  store i32 9, ptr %i, align 4, !tbaa !11
  tail call void @_Z18work_group_barrierj(i32 1) #5
  %10 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %c0, ptr addrspace(4) %0, i32 4, i32 4)
  tail call void @_Z18work_group_barrierj(i32 1) #5
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #5
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4) nocapture noundef readonly, i32 noundef, i32 noundef) local_unnamed_addr #0

define void @__pipe_global_ctor() {
entry:
  store ptr addrspace(1) @c2.pipe.bs, ptr addrspace(1) @c2.pipe, align 16
  store ptr addrspace(1) getelementptr inbounds ([1312 x i8], ptr addrspace(1) @c2.pipe.bs, i64 0, i64 328), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @c2.pipe, i64 0, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([1312 x i8], ptr addrspace(1) @c2.pipe.bs, i64 0, i64 656), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @c2.pipe, i64 0, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([1312 x i8], ptr addrspace(1) @c2.pipe.bs, i64 0, i64 984), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @c2.pipe, i64 0, i64 3), align 8
  tail call void @__pipe_init_array_fpga(ptr addrspace(1) @c2.pipe, i32 4, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @c4.pipe.bs, ptr addrspace(1) @c4.pipe, align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 328), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 656), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 984), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1312), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1640), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1968), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2296), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2624), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 0), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2952), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3280), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3608), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3936), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4264), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4592), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4920), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5248), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 0), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5576), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5904), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6232), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6560), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6888), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7216), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7544), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7872), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 0), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8200), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8528), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8856), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9184), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9512), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9840), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 10168), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 7), align 8
  tail call void @__pipe_init_array_fpga(ptr addrspace(1) @c4.pipe, i32 32, i32 4, i32 0, i32 0)
  tail call void @__pipe_init_fpga(ptr addrspace(1) @c8.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @c8.pipe.bs, ptr addrspace(1) @c8.pipe, align 8
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare void @__pipe_init_array_fpga(ptr addrspace(1) nocapture noundef readonly, i32 noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #0

; Function Attrs: convergent norecurse nounwind
declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #0

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32) local_unnamed_addr

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) local_unnamed_addr #4

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent noinline norecurse nounwind }
attributes #3 = { convergent mustprogress nofree nounwind willreturn memory(none) }
attributes #4 = { convergent }
attributes #5 = { nounwind }
attributes #6 = { convergent nounwind willreturn memory(none) }
attributes #7 = { convergent nounwind }

; CHECK: ![[TRUE]] = !{i1 true}
; CHECK: ![[FALSE]] = !{i1 false}

!sycl.kernels = !{!1}

!0 = !{i32 4}
!1 = !{ptr @for_channel, ptr @for_pipe, ptr @for_global_id, ptr @for_channel_ver2, ptr @for_channel_ver3}
!2 = !{}
!3 = !{i32 1}
!4 = !{!"none"}
!5 = !{!"int*"}
!6 = !{!""}
!7 = !{!"data"}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{ptr addrspace(1) null}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!"write_only"}
!16 = !{!"int"}
!17 = !{!"pipe"}
!18 = !{!"c0"}
!19 = !{target("spirv.Pipe", 1) zeroinitializer}

; DEBUGIFY-NOT: WARNING
