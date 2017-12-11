; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
;
; channel int c1;
; channel int c2;
;
; struct St {
;   float f;
;   long  l;
; };
;
; channel struct St c3;
;
; channel int c4[4][8];
;
; channel int c5;
;
; __kernel void foo(read_only pipe int p) {
;   int val1 = read_channel_intel(c1);
;   int val2 = read_channel_intel(c2);
;
;   struct St val3 = read_channel_intel(c3);
;
;   write_channel_intel(c4[1][2], 45);
;
;   int val4 = read_channel_intel(c4[2][1]);
;
;   write_channel_intel(c5, 46);
;
;   int val5 = -1;
;   read_pipe(p, &val5);
; }
;
; __kernel void bar(write_only pipe int p) {
;   write_channel_intel(c1, 42);
;   write_channel_intel(c2, 43);
;
;   struct St st = {0};
;   write_channel_intel(c3, st);
;
;   int val1 = read_channel_intel(c4[1][2]);
;
;   write_channel_intel(c4[2][1], 44);
;
;   int val2 = read_channel_intel(c5);
;
;   int val3 = 47;
;   write_pipe(p, &val3);
; }
; ----------------------------------------------------
; Compilation command:
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -emit-llvm -cl-std=CL2.0
;   oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -spir-materializer -channel-pipe-transformation
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -builtin-import -always-inline -pipe-support -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

source_filename = "create_flush_all.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_t.0 = type opaque
%opencl.pipe_t = type opaque
%struct.St = type { float, i64 }
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@c1 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@c2 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@c3 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8
@c4 = common local_unnamed_addr addrspace(1) global [4 x [8 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
@c5 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@pipe.c1 = common addrspace(1) global %opencl.pipe_t.0 addrspace(1)* null, align 4
@pipe.c2 = common addrspace(1) global %opencl.pipe_t.0 addrspace(1)* null, align 4
@pipe.c3 = common addrspace(1) global %opencl.pipe_t.0 addrspace(1)* null, align 4
@pipe.c4 = common addrspace(1) global [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] zeroinitializer, align 4
@pipe.c5 = common addrspace(1) global %opencl.pipe_t.0 addrspace(1)* null, align 4
@pipe.c4.bs = common addrspace(1) global [45056 x i8] zeroinitializer, align 64
@pipe.c3.bs = common addrspace(1) global [4672 x i8] zeroinitializer, align 64
@pipe.c1.bs = common addrspace(1) global [1408 x i8] zeroinitializer, align 64
@pipe.c2.bs = common addrspace(1) global [1408 x i8] zeroinitializer, align 64
@pipe.c5.bs = common addrspace(1) global [1408 x i8] zeroinitializer, align 64
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__global_pipes_ctor, i8* null }]

; Function Attrs: nounwind
define void @foo(%opencl.pipe_t addrspace(1)* %p) local_unnamed_addr #0 !kernel_arg_addr_space !14 !kernel_arg_access_qual !15 !kernel_arg_type !16 !kernel_arg_base_type !16 !kernel_arg_type_qual !17 {
entry:
; CHECK:      %[[WRITE_ARR:[0-9]+]] = alloca %struct.__pipe_t addrspace(1)*, i32 37
; CHECK:      %[[READ_ARR:[0-9]+]] = alloca %struct.__pipe_t addrspace(1)*, i32 37
  %write.src = alloca i32
  %read.dst = alloca i32
  %val3 = alloca %struct.St, align 8
  %val5 = alloca i32, align 4
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c1, align 4, !tbaa !18
  %1 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c1
  %2 = bitcast %opencl.pipe_t.0 addrspace(1)* %1 to %struct.__pipe_t addrspace(1)*
  %3 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %4 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* %3)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE0:.*]])
; CHECK:      %[[RET0:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE0]]
; CHECK-NEXT: %[[ICMP0:[0-9]+]] = icmp ne i32 %[[RET0]], 0
; CHECK-NEXT: br i1 %[[ICMP0]], label %[[THEN0:[0-9]+]]
; CHECK:      <label>:[[THEN0]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  %call = load i32, i32* %read.dst
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c2, align 4, !tbaa !18
  %6 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c2
  %7 = bitcast %opencl.pipe_t.0 addrspace(1)* %6 to %struct.__pipe_t addrspace(1)*
  %8 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %9 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %7, i8 addrspace(4)* %8)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE1:.*]])
; CHECK:      %[[RET1:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE1]]
; CHECK-NEXT: %[[ICMP1:[0-9]+]] = icmp ne i32 %[[RET1]], 0
; CHECK-NEXT: br i1 %[[ICMP1]], label %[[THEN1:[0-9]+]]
; CHECK:      <label>:[[THEN1]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  %call1 = load i32, i32* %read.dst
  %10 = bitcast %struct.St* %val3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %10) #5
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c3, align 8, !tbaa !18
  %12 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c3
  %13 = bitcast %opencl.pipe_t.0 addrspace(1)* %12 to %struct.__pipe_t addrspace(1)*
  %14 = addrspacecast %struct.St* %val3 to i8 addrspace(4)*
  %15 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %13, i8 addrspace(4)* %14)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE2:.*]])
; CHECK:      %[[RET2:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE2]]
; CHECK-NEXT: %[[ICMP2:[0-9]+]] = icmp ne i32 %[[RET2]], 0
; CHECK-NEXT: br i1 %[[ICMP2]], label %[[THEN2:[0-9]+]]
; CHECK:      <label>:[[THEN2]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  %16 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 1, i64 2), align 4, !tbaa !18
  store i32 45, i32* %write.src
  %17 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i64 0, i64 1, i64 2)
  %18 = bitcast %opencl.pipe_t.0 addrspace(1)* %17 to %struct.__pipe_t addrspace(1)*
  %19 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %20 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %18, i8 addrspace(4)* %19)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE3:.*]])
; CHECK:      %[[RET3:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE3]]
; CHECK-NEXT: %[[ICMP3:[0-9]+]] = icmp ne i32 %[[RET3]], 0
; CHECK-NEXT: br i1 %[[ICMP3]], label %[[THEN3:[0-9]+]]
; CHECK:      <label>:[[THEN3]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  %21 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 2, i64 1), align 4, !tbaa !18
  %22 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i64 0, i64 2, i64 1)
  %23 = bitcast %opencl.pipe_t.0 addrspace(1)* %22 to %struct.__pipe_t addrspace(1)*
  %24 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %25 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %23, i8 addrspace(4)* %24)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE4:.*]])
; CHECK:      %[[RET4:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE4]]
; CHECK-NEXT: %[[ICMP4:[0-9]+]] = icmp ne i32 %[[RET4]], 0
; CHECK-NEXT: br i1 %[[ICMP4]], label %[[THEN4:[0-9]+]]
; CHECK:      <label>:[[THEN4]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  %call2 = load i32, i32* %read.dst
  %26 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c5, align 4, !tbaa !18
  store i32 46, i32* %write.src
  %27 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c5
  %28 = bitcast %opencl.pipe_t.0 addrspace(1)* %27 to %struct.__pipe_t addrspace(1)*
  %29 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %30 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %28, i8 addrspace(4)* %29)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE5:.*]])
; CHECK:      %[[RET5:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE5]]
; CHECK-NEXT: %[[ICMP5:[0-9]+]] = icmp ne i32 %[[RET5]], 0
; CHECK-NEXT: br i1 %[[ICMP5]], label %[[THEN5:[0-9]+]]
; CHECK:      <label>:[[THEN5]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  %31 = bitcast i32* %val5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %31) #5
  store i32 -1, i32* %val5, align 4, !tbaa !21
  %32 = addrspacecast i8* %31 to i8 addrspace(4)*
  %33 = bitcast %opencl.pipe_t addrspace(1)* %p to %struct.__pipe_t addrspace(1)*
  %34 = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %33, i8 addrspace(4)* nonnull %32) #5
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE6:.*]])
; CHECK:      %[[RET6:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE6]]
; CHECK-NEXT: %[[ICMP6:[0-9]+]] = icmp ne i32 %[[RET6]], 0
; CHECK-NEXT: br i1 %[[ICMP6]], label %[[THEN6:[0-9]+]]
; CHECK:      <label>:[[THEN6]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]

  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %31) #5
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %10) #5
  ret void
; CHECK:      call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR]]
; CHECK-NEXT: ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
define void @bar(%opencl.pipe_t addrspace(1)* %p) local_unnamed_addr #0 !kernel_arg_addr_space !14 !kernel_arg_access_qual !23 !kernel_arg_type !16 !kernel_arg_base_type !16 !kernel_arg_type_qual !17 {
entry:
; CHECK:      %[[WRITE_ARR1:[0-9]+]] = alloca %struct.__pipe_t addrspace(1)*, i32 37
; CHECK:      %[[READ_ARR1:[0-9]+]] = alloca %struct.__pipe_t addrspace(1)*, i32 37
  %write.src = alloca i32
  %read.dst = alloca i32
  %st = alloca %struct.St, align 8
  %val3 = alloca i32, align 4
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c1, align 4, !tbaa !18
  store i32 42, i32* %write.src
  %1 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c1
  %2 = bitcast %opencl.pipe_t.0 addrspace(1)* %1 to %struct.__pipe_t addrspace(1)*
  %3 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %4 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* %3)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE7:.*]])
; CHECK:      %[[RET7:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE7]]
; CHECK-NEXT: %[[ICMP7:[0-9]+]] = icmp ne i32 %[[RET7]], 0
; CHECK-NEXT: br i1 %[[ICMP7]], label %[[THEN7:[0-9]+]]
; CHECK:      <label>:[[THEN7]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c2, align 4, !tbaa !18
  store i32 43, i32* %write.src
  %6 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c2
  %7 = bitcast %opencl.pipe_t.0 addrspace(1)* %6 to %struct.__pipe_t addrspace(1)*
  %8 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %9 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %7, i8 addrspace(4)* %8)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE8:.*]])
; CHECK:      %[[RET8:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE8]]
; CHECK-NEXT: %[[ICMP8:[0-9]+]] = icmp ne i32 %[[RET8]], 0
; CHECK-NEXT: br i1 %[[ICMP8]], label %[[THEN8:[0-9]+]]
; CHECK:      <label>:[[THEN8]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  %10 = bitcast %struct.St* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %10) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull %10, i8 0, i64 16, i32 8, i1 false)
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c3, align 8, !tbaa !18
  %12 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c3
  %13 = bitcast %opencl.pipe_t.0 addrspace(1)* %12 to %struct.__pipe_t addrspace(1)*
  %14 = addrspacecast %struct.St* %st to i8 addrspace(4)*
  %15 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %13, i8 addrspace(4)* %14)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE9:.*]])
; CHECK:      %[[RET9:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE9]]
; CHECK-NEXT: %[[ICMP9:[0-9]+]] = icmp ne i32 %[[RET9]], 0
; CHECK-NEXT: br i1 %[[ICMP9]], label %[[THEN9:[0-9]+]]
; CHECK:      <label>:[[THEN9]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  %16 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 1, i64 2), align 4, !tbaa !18
  %17 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i64 0, i64 1, i64 2)
  %18 = bitcast %opencl.pipe_t.0 addrspace(1)* %17 to %struct.__pipe_t addrspace(1)*
  %19 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %20 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %18, i8 addrspace(4)* %19)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE10:.*]])
; CHECK:      %[[RET10:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE10]]
; CHECK-NEXT: %[[ICMP10:[0-9]+]] = icmp ne i32 %[[RET10]], 0
; CHECK-NEXT: br i1 %[[ICMP10]], label %[[THEN10:[0-9]+]]
; CHECK:      <label>:[[THEN10]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  %call = load i32, i32* %read.dst
  %21 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 2, i64 1), align 4, !tbaa !18
  store i32 44, i32* %write.src
  %22 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i64 0, i64 2, i64 1)
  %23 = bitcast %opencl.pipe_t.0 addrspace(1)* %22 to %struct.__pipe_t addrspace(1)*
  %24 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %25 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %23, i8 addrspace(4)* %24)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE11:.*]])
; CHECK:      %[[RET11:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE11]]
; CHECK-NEXT: %[[ICMP11:[0-9]+]] = icmp ne i32 %[[RET11]], 0
; CHECK-NEXT: br i1 %[[ICMP11]], label %[[THEN11:[0-9]+]]
; CHECK:      <label>:[[THEN11]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  %26 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c5, align 4, !tbaa !18
  %27 = load %opencl.pipe_t.0 addrspace(1)*, %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c5
  %28 = bitcast %opencl.pipe_t.0 addrspace(1)* %27 to %struct.__pipe_t addrspace(1)*
  %29 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %30 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %28, i8 addrspace(4)* %29)
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE12:.*]])
; CHECK:      %[[RET12:[0-9]+]] = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE12]]
; CHECK-NEXT: %[[ICMP12:[0-9]+]] = icmp ne i32 %[[RET12]], 0
; CHECK-NEXT: br i1 %[[ICMP12]], label %[[THEN12:[0-9]+]]
; CHECK:      <label>:[[THEN12]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  %call1 = load i32, i32* %read.dst
  %31 = bitcast i32* %val3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %31) #5
  store i32 47, i32* %val3, align 4, !tbaa !21
  %32 = addrspacecast i8* %31 to i8 addrspace(4)*
  %33 = bitcast %opencl.pipe_t addrspace(1)* %p to %struct.__pipe_t addrspace(1)*
  %34 = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %33, i8 addrspace(4)* nonnull %32) #5
; CHECK:      call void @__store_pipe_use(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %struct.__pipe_t addrspace(1)* %[[PIPE12:.*]])
; CHECK:      %[[RET12:[0-9]+]] = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %[[PIPE12]]
; CHECK-NEXT: %[[ICMP12:[0-9]+]] = icmp ne i32 %[[RET12]], 0
; CHECK-NEXT: br i1 %[[ICMP12]], label %[[THEN12:[0-9]+]]
; CHECK:      <label>:[[THEN12]]
; CHECK-NEXT: call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]

  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %31) #5
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %10) #5
  ret void
; CHECK:      call void @__flush_pipe_read_array(%struct.__pipe_t addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(%struct.__pipe_t addrspace(1)** %[[WRITE_ARR1]]
; CHECK-NEXT: ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #1

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #3

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #3

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32) #3

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32) #3

define void @__global_pipes_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.c2.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.c2.bs to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c2
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.c5.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.c5.bs to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c5
  store %opencl.pipe_t.0 addrspace(1)* bitcast ([45056 x i8] addrspace(1)* @pipe.c4.bs to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 0)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 1408) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 2816) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 2)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 4224) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 3)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 5632) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 4)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 7040) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 5)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 8448) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 6)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 9856) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 0, i32 7)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 11264) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 0)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 12672) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 14080) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 2)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 15488) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 3)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 16896) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 4)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 18304) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 5)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 19712) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 6)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 21120) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 1, i32 7)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 22528) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 0)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 23936) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 25344) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 2)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 26752) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 3)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 28160) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 4)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 29568) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 5)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 30976) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 6)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 32384) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 2, i32 7)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 33792) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 0)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 35200) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 36608) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 2)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 38016) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 3)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 39424) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 4)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 40832) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 5)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 42240) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 6)
  store %opencl.pipe_t.0 addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([45056 x i8], [45056 x i8] addrspace(1)* @pipe.c4.bs, i64 0, i64 43648) to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]], [4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4, i32 0, i32 3, i32 7)
  call void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)* bitcast ([4 x [8 x %opencl.pipe_t.0 addrspace(1)*]] addrspace(1)* @pipe.c4 to %struct.__pipe_t addrspace(1)* addrspace(1)*), i32 32, i32 4, i32 1)
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([4672 x i8] addrspace(1)* @pipe.c3.bs to %struct.__pipe_t addrspace(1)*), i32 16, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast ([4672 x i8] addrspace(1)* @pipe.c3.bs to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c3
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.c1.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 1)
  store %opencl.pipe_t.0 addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.c1.bs to %opencl.pipe_t.0 addrspace(1)*), %opencl.pipe_t.0 addrspace(1)* addrspace(1)* @pipe.c1
  ret void
}

; Function Attrs: alwaysinline nounwind
declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*) #4

; Function Attrs: alwaysinline nounwind
declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*) #4

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { alwaysinline nounwind }
attributes #5 = { nounwind }

!opencl.channels = !{!0, !3, !4, !7, !8}
!llvm.module.flags = !{!9}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!10}
!opencl.spir.version = !{!10}
!opencl.used.extensions = !{!11}
!opencl.used.optional.core.features = !{!11}
!opencl.compiler.options = !{!11}
!llvm.ident = !{!12}
!opencl.kernels = !{!13}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @c1, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @c2, !1, !2}
!4 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @c3, !5, !6}
!5 = !{!"packet_size", i32 16}
!6 = !{!"packet_align", i32 8}
!7 = !{[4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, !1, !2}
!8 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @c5, !1, !2}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{i32 2, i32 0}
!11 = !{}
!12 = !{!"clang version 5.0.0 (cfe/trunk)"}
!13 = !{void (%opencl.pipe_t addrspace(1)*)* @foo, void (%opencl.pipe_t addrspace(1)*)* @bar}
!14 = !{i32 1}
!15 = !{!"read_only"}
!16 = !{!"int"}
!17 = !{!"pipe"}
!18 = !{!19, !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !19, i64 0}
!23 = !{!"write_only"}
