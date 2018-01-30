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
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -emit-llvm -cl-std=CL2.0
;   oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -spir-materializer -channel-pipe-transformation
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-support -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@c4 = common local_unnamed_addr addrspace(1) global [4 x [8 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@c4.pipe = addrspace(1) global [4 x [8 x %opencl.pipe_t addrspace(1)*]] zeroinitializer, align 16
@c4.pipe.bs = addrspace(1) global [45056 x i8] zeroinitializer, align 4

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

; Function Attrs: convergent nounwind
define void @for_channel() local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 {
entry:
  %read.dst = alloca i32
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 0, i64 0)
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 0, i64 0), align 4, !tbaa !8
  %2 = bitcast %opencl.pipe_t addrspace(1)* %0 to %struct.__pipe_t addrspace(1)*
  %3 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* %3)
  %4 = load i32, i32* %read.dst
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
define void @for_pipe(%opencl.pipe_t addrspace(1)* %c0) local_unnamed_addr #2 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 {
entry:
  %i = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #6
  store i32 0, i32* %i, align 4, !tbaa !16
  %1 = addrspacecast i8* %0 to i8 addrspace(4)*
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #6
  ret void

for.body:                                         ; preds = %for.body, %entry
  %2 = bitcast %opencl.pipe_t addrspace(1)* %c0 to %struct.__pipe_t addrspace(1)*
  %3 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* nonnull %1) #6
  %4 = load i32, i32* %i, align 4, !tbaa !16
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4, !tbaa !16
  %cmp = icmp slt i32 %4, 9
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

declare i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32) local_unnamed_addr

; Function Attrs: convergent noinline nounwind
define void @channel_func() local_unnamed_addr #3 {
entry:
  %read.dst = alloca i32
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 0, i64 0)
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 0, i64 0), align 4, !tbaa !8
  %2 = bitcast %opencl.pipe_t addrspace(1)* %0 to %struct.__pipe_t addrspace(1)*
  %3 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* %3)
  %4 = load i32, i32* %read.dst
  ret void
}

; Function Attrs: convergent nounwind
define void @for_global_id(i32 addrspace(1)* nocapture %data) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_host_accessible !15 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #7
  %0 = shl i64 %call, 32
  %idxprom = ashr exact i64 %0, 32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 %idxprom
  store i32 42, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #4

; Function Attrs: convergent noinline nounwind
define void @for_channel_ver2() local_unnamed_addr #3 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 {
entry:
  tail call void @channel_func() #8
  ret void
}

; Function Attrs: convergent nounwind
define void @for_channel_ver3() local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 {
entry:
  tail call void @for_channel_ver2() #8
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #5

define void @__pipe_global_ctor() {
entry:
  store %opencl.pipe_t addrspace(1)* bitcast ([45056 x i8] addrspace(1)* @c4.pipe.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 0)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1408) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2816) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4224) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 3)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5632) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 4)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7040) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 5)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8448) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 6)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9856) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 7)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 11264) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 0)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 12672) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 14080) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 15488) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 3)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 16896) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 4)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 18304) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 5)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 19712) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 6)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 21120) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 7)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 22528) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 0)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 23936) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 25344) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 26752) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 3)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 28160) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 4)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 29568) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 5)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 30976) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 6)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 32384) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 7)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 33792) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 0)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 35200) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 36608) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 38016) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 3)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 39424) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 4)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 40832) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 5)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 42240) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 6)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 43648) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t addrspace(1)*]], [4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 7)
  call void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)* bitcast ([4 x [8 x %opencl.pipe_t addrspace(1)*]] addrspace(1)* @c4.pipe to %struct.__pipe_t addrspace(1)* addrspace(1)*), i32 32, i32 4, i32 1)
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32) #5

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #5

declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readnone }
attributes #6 = { nounwind }
attributes #7 = { convergent nounwind readnone }
attributes #8 = { convergent }

!opencl.channels = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = !{[4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 134c4d109831a8af6f2992e8338b04f34a4de803) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fcec820f4b80de4ed49d51d31292f09593b932e6)"}
!7 = !{void ()* @for_channel, void (%opencl.pipe_t addrspace(1)*)* @for_pipe, void (i32 addrspace(1)*)* @for_global_id, void ()* @for_channel_ver2, void ()* @for_channel_ver3}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{i32 1}
!12 = !{!"write_only"}
!13 = !{!"int"}
!14 = !{!"pipe"}
!15 = !{i1 false}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !9, i64 0}
!18 = !{!"none"}
!19 = !{!"int*"}
!20 = !{!""}
