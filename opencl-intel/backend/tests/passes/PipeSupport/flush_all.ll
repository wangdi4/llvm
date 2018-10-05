; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
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
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -emit-llvm -cl-std=CL1.2
;   oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -builtin-import -always-inline -pipe-support -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_ro_t = type opaque
%struct.St = type { float, i64 }
%opencl.pipe_wo_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@c1 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@c2 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@c3 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8, !packet_size !1, !packet_align !2
@c4 = common addrspace(1) global [4 x [8 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@c5 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@c1.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@c1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@c2.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@c2.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@c3.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !1, !packet_align !2
@c3.pipe.bs = addrspace(1) global [352 x i8] zeroinitializer, align 8
@c4.pipe = addrspace(1) global [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [10496 x i8] zeroinitializer, align 4
@c5.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@c5.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define void @foo(%opencl.pipe_ro_t addrspace(1)* %p) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 {
entry:
; CHECK:      %[[WRITE_ARR:[0-9]+]] = alloca i8 addrspace(1)*, i32 37
; CHECK:      %[[READ_ARR:[0-9]+]] = alloca i8 addrspace(1)*, i32 37
  %write.src = alloca i32
  %p.addr = alloca %opencl.pipe_ro_t addrspace(1)*, align 8
  %val1 = alloca i32, align 4
  %val2 = alloca i32, align 4
  %val3 = alloca %struct.St, align 8
  %val4 = alloca i32, align 4
  %val5 = alloca i32, align 4
  store %opencl.pipe_ro_t addrspace(1)* %p, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !15
  %0 = bitcast i32* %val1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c1.pipe
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c1, align 4, !tbaa !15
  %3 = bitcast %opencl.pipe_rw_t addrspace(1)* %1 to %opencl.pipe_ro_t addrspace(1)*
  %4 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %call3 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %3, i8 addrspace(4)* %4, i32 4, i32 4)
; CHECK:      %[[PIPE0RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c1.pipe
; CHECK:      %[[PIPE0RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE0RW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE0RO:.*]])
; CHECK:      %[[RET0:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE0RO]]
; CHECK-NEXT: %[[ICMP0:[0-9]+]] = icmp ne i32 %[[RET0]], 0
; CHECK-NEXT: br i1 %[[ICMP0]], label %[[THEN0:[0-9]+]]
; CHECK:      <label>:[[THEN0]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %5 = load i32, i32* %write.src
  store i32 %5, i32* %val1, align 4, !tbaa !18
  %6 = bitcast i32* %val2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #3
  %7 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c2.pipe
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c2, align 4, !tbaa !15
  %9 = bitcast %opencl.pipe_rw_t addrspace(1)* %7 to %opencl.pipe_ro_t addrspace(1)*
  %10 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %call12 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %9, i8 addrspace(4)* %10, i32 4, i32 4)
; CHECK:      %[[PIPE1RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c2.pipe
; CHECK:      %[[PIPE1RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE1RW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE1RO:.*]])
; CHECK:      %[[RET1:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE1RO]]
; CHECK-NEXT: %[[ICMP1:[0-9]+]] = icmp ne i32 %[[RET1]], 0
; CHECK-NEXT: br i1 %[[ICMP1]], label %[[THEN1:[0-9]+]]
; CHECK:      <label>:[[THEN1]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %11 = load i32, i32* %write.src
  store i32 %11, i32* %val2, align 4, !tbaa !18
  %12 = bitcast %struct.St* %val3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %12) #3
  %13 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c3.pipe
  %14 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c3, align 8, !tbaa !15
  %15 = bitcast %opencl.pipe_rw_t addrspace(1)* %13 to %opencl.pipe_ro_t addrspace(1)*
  %16 = addrspacecast %struct.St* %val3 to i8 addrspace(4)*
  %17 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %15, i8 addrspace(4)* %16, i32 16, i32 8)
; CHECK:      %[[PIPE2RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c3.pipe
; CHECK:      %[[PIPE2RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE2RW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE2RO:.*]])
; CHECK:      %[[RET2:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE2RO]]
; CHECK-NEXT: %[[ICMP2:[0-9]+]] = icmp ne i32 %[[RET2]], 0
; CHECK-NEXT: br i1 %[[ICMP2]], label %[[THEN2:[0-9]+]]
; CHECK:      <label>:[[THEN2]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %18 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 1, i64 2)
  %19 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 1, i64 2), align 4, !tbaa !15
  store i32 45, i32* %write.src
  %20 = bitcast %opencl.pipe_rw_t addrspace(1)* %18 to %opencl.pipe_wo_t addrspace(1)*
  %21 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %22 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %20, i8 addrspace(4)* %21, i32 4, i32 4)
; CHECK:      %[[PIPE3RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} getelementptr {{.*}} @c4.pipe, i64 0, i64 1, i64 2
; CHECK:      %[[PIPE3WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE3RW]] to %opencl.pipe_wo_t
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE3WO:.*]])
; CHECK:      %[[RET3:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE3WO]]
; CHECK-NEXT: %[[ICMP3:[0-9]+]] = icmp ne i32 %[[RET3]], 0
; CHECK-NEXT: br i1 %[[ICMP3]], label %[[THEN3:[0-9]+]]
; CHECK:      <label>:[[THEN3]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %23 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %23) #3
  %24 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 2, i64 1)
  %25 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 2, i64 1), align 4, !tbaa !15
  %26 = bitcast %opencl.pipe_rw_t addrspace(1)* %24 to %opencl.pipe_ro_t addrspace(1)*
  %27 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %call21 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %26, i8 addrspace(4)* %27, i32 4, i32 4)
; CHECK:      %[[PIPE4RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} getelementptr {{.*}} @c4.pipe, i64 0, i64 2, i64 1
; CHECK:      %[[PIPE4RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE4RW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE4RO:.*]])
; CHECK:      %[[RET4:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE4RO]]
; CHECK-NEXT: %[[ICMP4:[0-9]+]] = icmp ne i32 %[[RET4]], 0
; CHECK-NEXT: br i1 %[[ICMP4]], label %[[THEN4:[0-9]+]]
; CHECK:      <label>:[[THEN4]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %28 = load i32, i32* %write.src
  store i32 %28, i32* %val4, align 4, !tbaa !18
  %29 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c5.pipe
  %30 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c5, align 4, !tbaa !15
  store i32 46, i32* %write.src
  %31 = bitcast %opencl.pipe_rw_t addrspace(1)* %29 to %opencl.pipe_wo_t addrspace(1)*
  %32 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %33 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %31, i8 addrspace(4)* %32, i32 4, i32 4)
; CHECK:      %[[PIPE5RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c5.pipe
; CHECK:      %[[PIPE5WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE5RW]] to %opencl.pipe_wo_t
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE5WO:.*]])
; CHECK:      %[[RET5:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE5WO]]
; CHECK-NEXT: %[[ICMP5:[0-9]+]] = icmp ne i32 %[[RET5]], 0
; CHECK-NEXT: br i1 %[[ICMP5]], label %[[THEN5:[0-9]+]]
; CHECK:      <label>:[[THEN5]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %34 = bitcast i32* %val5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %34) #3
  store i32 -1, i32* %val5, align 4, !tbaa !18
  %35 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !15
  %36 = bitcast i32* %val5 to i8*
  %37 = addrspacecast i8* %36 to i8 addrspace(4)*
  %38 = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %35, i8 addrspace(4)* %37, i32 4, i32 4)
; CHECK:      %[[PIPE6RO:[0-9]+]] = load %opencl.pipe_ro_t {{.*}} %p.addr
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE6RO:.*]])
; CHECK:      %[[RET6:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE6RO]]
; CHECK-NEXT: %[[ICMP6:[0-9]+]] = icmp ne i32 %[[RET6]], 0
; CHECK-NEXT: br i1 %[[ICMP6]], label %[[THEN6:[0-9]+]]
; CHECK:      <label>:[[THEN6]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]

  %39 = bitcast i32* %val5 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %39) #3
  %40 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %40) #3
  %41 = bitcast %struct.St* %val3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %41) #3
  %42 = bitcast i32* %val2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %42) #3
  %43 = bitcast i32* %val1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %43) #3
  ret void
; CHECK:      call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR]]
; CHECK-NEXT: ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @__read_pipe_2_AS0(%opencl.pipe_ro_t addrspace(1)*, i8*, i32, i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define void @bar(%opencl.pipe_wo_t addrspace(1)* %p) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !20 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 {
entry:
; CHECK:      %[[WRITE_ARR1:[0-9]+]] = alloca i8 addrspace(1)*, i32 37
; CHECK:      %[[READ_ARR1:[0-9]+]] = alloca i8 addrspace(1)*, i32 37
  %read.dst = alloca i32
  %p.addr = alloca %opencl.pipe_wo_t addrspace(1)*, align 8
  %st = alloca %struct.St, align 8
  %val1 = alloca i32, align 4
  %val2 = alloca i32, align 4
  %val3 = alloca i32, align 4
  store %opencl.pipe_wo_t addrspace(1)* %p, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !15
  %0 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c1.pipe
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c1, align 4, !tbaa !15
  store i32 42, i32* %read.dst
  %2 = bitcast %opencl.pipe_rw_t addrspace(1)* %0 to %opencl.pipe_wo_t addrspace(1)*
  %3 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %4 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %2, i8 addrspace(4)* %3, i32 4, i32 4)
; CHECK:      %[[PIPE7RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c1.pipe
; CHECK:      %[[PIPE7WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE7RW]] to %opencl.pipe_wo_t
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE7WO:.*]])
; CHECK:      %[[RET7:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE7WO]]
; CHECK-NEXT: %[[ICMP7:[0-9]+]] = icmp ne i32 %[[RET7]], 0
; CHECK-NEXT: br i1 %[[ICMP7]], label %[[THEN7:[0-9]+]]
; CHECK:      <label>:[[THEN7]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %5 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c2.pipe
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c2, align 4, !tbaa !15
  store i32 43, i32* %read.dst
  %7 = bitcast %opencl.pipe_rw_t addrspace(1)* %5 to %opencl.pipe_wo_t addrspace(1)*
  %8 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %9 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %7, i8 addrspace(4)* %8, i32 4, i32 4)
; CHECK:      %[[PIPE8RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c2.pipe
; CHECK:      %[[PIPE8WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE8RW]] to %opencl.pipe_wo_t
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE8WO:.*]])
; CHECK:      %[[RET8:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE8WO]]
; CHECK-NEXT: %[[ICMP8:[0-9]+]] = icmp ne i32 %[[RET8]], 0
; CHECK-NEXT: br i1 %[[ICMP8]], label %[[THEN8:[0-9]+]]
; CHECK:      <label>:[[THEN8]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %10 = bitcast %struct.St* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %10) #3
  %11 = bitcast %struct.St* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %11, i8 0, i64 16, i1 false)
  %12 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c3.pipe
  %13 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c3, align 8, !tbaa !15
  %14 = bitcast %opencl.pipe_rw_t addrspace(1)* %12 to %opencl.pipe_wo_t addrspace(1)*
  %15 = addrspacecast %struct.St* %st to i8 addrspace(4)*
  %16 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %14, i8 addrspace(4)* %15, i32 16, i32 8)
; CHECK:      %[[PIPE9RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c3.pipe
; CHECK:      %[[PIPE9WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE9RW]] to %opencl.pipe_wo_t
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE9WO:.*]])
; CHECK:      %[[RET9:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE9WO]]
; CHECK-NEXT: %[[ICMP9:[0-9]+]] = icmp ne i32 %[[RET9]], 0
; CHECK-NEXT: br i1 %[[ICMP9]], label %[[THEN9:[0-9]+]]
; CHECK:      <label>:[[THEN9]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %17 = bitcast i32* %val1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %17) #3
  %18 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 1, i64 2)
  %19 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 1, i64 2), align 4, !tbaa !15
  %20 = bitcast %opencl.pipe_rw_t addrspace(1)* %18 to %opencl.pipe_ro_t addrspace(1)*
  %21 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call2 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %20, i8 addrspace(4)* %21, i32 4, i32 4)
; CHECK:      %[[PIPE10RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} getelementptr {{.*}} @c4.pipe, i64 0, i64 1, i64 2
; CHECK:      %[[PIPE10RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE10RW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR1]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE10RO:.*]])
; CHECK:      %[[RET10:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE10RO]]
; CHECK-NEXT: %[[ICMP10:[0-9]+]] = icmp ne i32 %[[RET10]], 0
; CHECK-NEXT: br i1 %[[ICMP10]], label %[[THEN10:[0-9]+]]
; CHECK:      <label>:[[THEN10]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %22 = load i32, i32* %read.dst
  store i32 %22, i32* %val1, align 4, !tbaa !18
  %23 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 2, i64 1)
  %24 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 2, i64 1), align 4, !tbaa !15
  store i32 44, i32* %read.dst
  %25 = bitcast %opencl.pipe_rw_t addrspace(1)* %23 to %opencl.pipe_wo_t addrspace(1)*
  %26 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %27 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %25, i8 addrspace(4)* %26, i32 4, i32 4)
; CHECK:      %[[PIPE11RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} getelementptr {{.*}} @c4.pipe, i64 0, i64 2, i64 1
; CHECK:      %[[PIPE11WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE11RW]] to %opencl.pipe_wo_t
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE11WO:.*]])
; CHECK:      %[[RET11:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE11WO]]
; CHECK-NEXT: %[[ICMP11:[0-9]+]] = icmp ne i32 %[[RET11]], 0
; CHECK-NEXT: br i1 %[[ICMP11]], label %[[THEN11:[0-9]+]]
; CHECK:      <label>:[[THEN11]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %28 = bitcast i32* %val2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %28) #3
  %29 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c5.pipe
  %30 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @c5, align 4, !tbaa !15
  %31 = bitcast %opencl.pipe_rw_t addrspace(1)* %29 to %opencl.pipe_ro_t addrspace(1)*
  %32 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call11 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %31, i8 addrspace(4)* %32, i32 4, i32 4)
; CHECK:      %[[PIPE12RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @c5.pipe
; CHECK:      %[[PIPE12RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE12RW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use(i8 addrspace(1)** %[[READ_ARR1]], {{.*}}, %opencl.pipe_ro_t addrspace(1)* %[[PIPE12RO:.*]])
; CHECK:      %[[RET12:.*]] = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE12RO]]
; CHECK-NEXT: %[[ICMP12:[0-9]+]] = icmp ne i32 %[[RET12]], 0
; CHECK-NEXT: br i1 %[[ICMP12]], label %[[THEN12:[0-9]+]]
; CHECK:      <label>:[[THEN12]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %33 = load i32, i32* %read.dst
  store i32 %33, i32* %val2, align 4, !tbaa !18
  %34 = bitcast i32* %val3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %34) #3
  store i32 47, i32* %val3, align 4, !tbaa !18
  %35 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !15
  %36 = bitcast i32* %val3 to i8*
  %37 = addrspacecast i8* %36 to i8 addrspace(4)*
  %38 = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %35, i8 addrspace(4)* %37, i32 4, i32 4)
; CHECK:      %[[PIPE13WO:[0-9]+]] = load %opencl.pipe_wo_t {{.*}} %p.addr
; CHECK:      call void @__store_write_pipe_use(i8 addrspace(1)** %[[WRITE_ARR1]], {{.*}}, %opencl.pipe_wo_t addrspace(1)* %[[PIPE13WO:.*]])
; CHECK:      %[[RET13:.*]] = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE13WO]]
; CHECK-NEXT: %[[ICMP13:[0-9]+]] = icmp ne i32 %[[RET13]], 0
; CHECK-NEXT: br i1 %[[ICMP13]], label %[[THEN13:[0-9]+]]
; CHECK:      <label>:[[THEN13]]
; CHECK-NEXT: call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]

  %39 = bitcast i32* %val3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %39) #3
  %40 = bitcast i32* %val2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %40) #3
  %41 = bitcast i32* %val1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %41) #3
  %42 = bitcast %struct.St* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %42) #3
  ret void
; CHECK:      call void @__flush_pipe_read_array(i8 addrspace(1)** %[[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(i8 addrspace(1)** %[[WRITE_ARR1]]
; CHECK-NEXT: ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

declare i32 @__write_pipe_2_AS0(%opencl.pipe_wo_t addrspace(1)*, i8*, i32, i32)

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #2

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #2

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @c1.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @c1.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c1.pipe
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @c2.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @c2.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c2.pipe
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([352 x i8] addrspace(1)* @c3.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 16, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([352 x i8] addrspace(1)* @c3.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c3.pipe
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
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @c5.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @c5.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @c5.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_fpga(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32, i32) #2

declare i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

declare i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = !{i32 4}
!1 = !{i32 16}
!2 = !{i32 8}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 1, i32 2}
!5 = !{}
!6 = !{!"clang version 7.0.0 "}
!7 = !{void (%opencl.pipe_ro_t addrspace(1)*)* @foo, void (%opencl.pipe_wo_t addrspace(1)*)* @bar}
!8 = !{i32 1}
!9 = !{!"read_only"}
!10 = !{!"int"}
!11 = !{!"pipe"}
!12 = !{i1 false}
!13 = !{i32 0}
!14 = !{!""}
!15 = !{!16, !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !16, i64 0}
!20 = !{!"write_only"}
