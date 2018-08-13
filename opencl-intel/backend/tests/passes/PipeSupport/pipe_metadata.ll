; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int c4[4][8];
;
; __kernel void for_channel() {
;     int val4 = read_channel_intel(c4[0][0]);
; }
;
; __kernel void for_pipe(write_only pipe int __attribute__((blocking)) c0) {
;     for (int i = 0; i < 10; i++) {
;         write_pipe (c0, &i);
;     }
; }
;
; __attribute__((noinline))
; void channel_func() {
;     int val4 = read_channel_intel(c4[0][0]);
; }
;
; __kernel void for_global_id(__global int *data) {
;     int gid = get_global_id(0);
;     data[gid] = 42;
; }
;
; __attribute__((noinline))
; __kernel void for_channel_ver2()
; {
;     channel_func();
; }
;
; __kernel void for_channel_ver3() {
;     for_channel_ver2();
; }
; ----------------------------------------------------
; Compilation command:
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -emit-llvm -cl-std=CL1.2
;   oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-support -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

; CHECK: define void @for_channel
; CHECK-SAME: !use_fpga_pipes ![[TRUE:[0-9]+]]
;
; CHECK: define void @for_pipe
; CHECK-SAME: !use_fpga_pipes ![[TRUE]]
;
; CHECK: define void @for_global_id
; CHECK-SAME: !use_fpga_pipes ![[FALSE:[0-9]+]]
;
; CHECK: define void @for_channel_ver2
; CHECK-SAME: !use_fpga_pipes ![[TRUE]]
;
; CHECK: define void @for_channel_ver3
; CHECK-SAME: !use_fpga_pipes ![[TRUE]]
;
; CHECK: ![[TRUE]] = !{i1 true}
; CHECK: ![[FALSE]] = !{i1 false}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_ro_t = type opaque
%opencl.pipe_wo_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@c4 = common addrspace(1) global [4 x [8 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@c4.pipe = addrspace(1) global [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [10496 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define void @for_channel() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %read.dst = alloca i32
  %val4 = alloca i32, align 4
  %0 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #6
  %1 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 0, i64 0)
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 0, i64 0), align 4, !tbaa !6
  %3 = bitcast %opencl.pipe_rw_t addrspace(1)* %1 to %opencl.pipe_ro_t addrspace(1)*
  %4 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %3, i8 addrspace(4)* %4, i32 4, i32 4)
  %5 = load i32, i32* %read.dst
  store i32 %5, i32* %val4, align 4, !tbaa !9
  %6 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #6
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define void @for_pipe(%opencl.pipe_wo_t addrspace(1)* %c0) #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !17 !kernel_arg_buffer_location !17 {
entry:
  %c0.addr = alloca %opencl.pipe_wo_t addrspace(1)*, align 8
  %i = alloca i32, align 4
  store %opencl.pipe_wo_t addrspace(1)* %c0, %opencl.pipe_wo_t addrspace(1)** %c0.addr, align 8, !tbaa !6
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #6
  store i32 0, i32* %i, align 4, !tbaa !9
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !9
  %cmp = icmp slt i32 %1, 10
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #6
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %c0.addr, align 8, !tbaa !6
  %4 = bitcast i32* %i to i8*
  %5 = addrspacecast i8* %4 to i8 addrspace(4)*
  %6 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %3, i8 addrspace(4)* %5, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, i32* %i, align 4, !tbaa !9
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32* %i, align 4, !tbaa !9
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

declare i32 @__write_pipe_2_bl_AS0(%opencl.pipe_wo_t addrspace(1)*, i8*, i32, i32)

; Function Attrs: convergent noinline nounwind
define void @channel_func() #2 {
entry:
  %read.dst = alloca i32
  %val4 = alloca i32, align 4
  %0 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #6
  %1 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 0, i64 0)
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 0, i64 0), align 4, !tbaa !6
  %3 = bitcast %opencl.pipe_rw_t addrspace(1)* %1 to %opencl.pipe_ro_t addrspace(1)*
  %4 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %3, i8 addrspace(4)* %4, i32 4, i32 4)
  %5 = load i32, i32* %read.dst
  store i32 %5, i32* %val4, align 4, !tbaa !9
  %6 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #6
  ret void
}

; Function Attrs: convergent nounwind
define void @for_global_id(i32 addrspace(1)* %data) #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !17 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !17 !kernel_arg_buffer_location !17 {
entry:
  %data.addr = alloca i32 addrspace(1)*, align 8
  %gid = alloca i32, align 4
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8, !tbaa !20
  %0 = bitcast i32* %gid to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #6
  %call = call i64 @_Z13get_global_idj(i32 0) #7
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %gid, align 4, !tbaa !9
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !tbaa !20
  %2 = load i32, i32* %gid, align 4, !tbaa !9
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 %idxprom
  store i32 42, i32 addrspace(1)* %arrayidx, align 4, !tbaa !9
  %3 = bitcast i32* %gid to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #6
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) #3

; Function Attrs: convergent noinline nounwind
define void @for_channel_ver2() #4 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  call void @channel_func() #8
  ret void
}

; Function Attrs: convergent nounwind
define void @for_channel_ver3() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  call void @for_channel_ver2() #9
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #5

define void @__pipe_global_ctor() {
entry:
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([10496 x i8] addrspace(1)* @c4.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 328) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 656) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 984) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1312) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1640) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1968) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2296) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 7)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2624) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2952) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 3280) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 3608) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 3936) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4264) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4592) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4920) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 7)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5248) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5576) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5904) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 6232) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 6560) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 6888) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7216) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7544) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 7)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7872) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8200) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8528) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8856) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9184) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9512) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9840) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 10168) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 7)
  call void @__pipe_init_array_fpga(%struct.__pipe_t addrspace(1)* addrspace(1)* bitcast ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe to %struct.__pipe_t addrspace(1)* addrspace(1)*), i32 32, i32 4, i32 0, i32 0)
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_fpga(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32, i32) #5

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #5

declare i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readnone }
attributes #6 = { nounwind }
attributes #7 = { convergent nounwind readnone }
attributes #8 = { convergent }
attributes #9 = { convergent "uniform-work-group-size"="true" }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 "}
!5 = !{void ()* @for_channel, void (%opencl.pipe_wo_t addrspace(1)*)* @for_pipe, void (i32 addrspace(1)*)* @for_global_id, void ()* @for_channel_ver2, void ()* @for_channel_ver3}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}
!11 = !{i32 1}
!12 = !{!"write_only"}
!13 = !{!"int"}
!14 = !{!"pipe"}
!15 = !{i1 false}
!16 = !{i32 0}
!17 = !{!""}
!18 = !{!"none"}
!19 = !{!"int*"}
!20 = !{!21, !21, i64 0}
!21 = !{!"any pointer", !7, i64 0}
