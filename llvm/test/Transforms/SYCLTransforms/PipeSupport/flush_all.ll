; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

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
;   opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-demangle-fpga-pipes -sycl-kernel-equalizer -sycl-kernel-channel-pipe-transformation %s -S
; ----------------------------------------------------

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.St = type { float, i64 }
%struct.channelpipetransformation.merge = type { float, i64 }

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@c1.pipe = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@c1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@c2.pipe = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@c2.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@c3.pipe = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !1, !packet_align !2
@c3.pipe.bs = addrspace(1) global [352 x i8] zeroinitializer, align 8
@c4.pipe = addrspace(1) global [4 x [8 x ptr addrspace(1)]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [10496 x i8] zeroinitializer, align 4
@c5.pipe = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@c5.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(ptr addrspace(1) %p) local_unnamed_addr #1 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !arg_type_null_val !12 {
entry:
; CHECK:      [[WRITE_ARR:%[0-9]+]] = alloca ptr addrspace(1), i32 37
; CHECK:      [[READ_ARR:%[0-9]+]] = alloca ptr addrspace(1), i32 37

  %write.src = alloca i32, align 4
  %val3 = alloca %struct.St, align 8
  %val5 = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c1.pipe, align 8
  %1 = addrspacecast ptr %write.src to ptr addrspace(4)
  %call3 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE0:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c1.pipe
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR]], {{.*}}, ptr addrspace(1) [[PIPE0:.*]])
; CHECK:      %[[RET0:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) [[PIPE0]]
; CHECK-NEXT: [[ICMP0:%[0-9]+]] = icmp ne i32 %[[RET0]], 0
; CHECK-NEXT: br i1 [[ICMP0]], label %[[THEN0:[0-9]+]]
; CHECK:      [[THEN0]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  %2 = load ptr addrspace(1), ptr addrspace(1) @c2.pipe, align 8
  %call12 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %2, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE1:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c2.pipe
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR]], {{.*}}, ptr addrspace(1) [[PIPE1]])
; CHECK:      %[[RET1:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) [[PIPE1]]
; CHECK-NEXT: [[ICMP1:[%0-9]+]] = icmp ne i32 %[[RET1]], 0
; CHECK-NEXT: br i1 [[ICMP1]], label %[[THEN1:[0-9]+]]
; CHECK:      [[THEN1]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %val3) #2
  %3 = load ptr addrspace(1), ptr addrspace(1) @c3.pipe, align 8
  %4 = addrspacecast ptr %val3 to ptr addrspace(4)
  %5 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %3, ptr addrspace(4) %4, i32 16, i32 8) #2
; CHECK:      [[PIPE2:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c3.pipe
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR]], {{.*}}, ptr addrspace(1) [[PIPE2]])
; CHECK:      %[[RET2:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) [[PIPE2]]
; CHECK-NEXT: [[ICMP2:%[0-9]+]] = icmp ne i32 %[[RET2]], 0
; CHECK-NEXT: br i1 [[ICMP2]], label %[[THEN2:[0-9]+]]
; CHECK:      [[THEN2]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  %6 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 2), align 16
  store i32 45, ptr %write.src, align 4
  %7 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %6, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE3:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 2
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR]], {{.*}}, ptr addrspace(1) [[PIPE3]])
; CHECK:      %[[RET3:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) [[PIPE3]]
; CHECK-NEXT: [[ICMP3:%[0-9]+]] = icmp ne i32 %[[RET3]], 0
; CHECK-NEXT: br i1 [[ICMP3]], label %[[THEN3:[0-9]+]]
; CHECK:      [[THEN3]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  %8 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 1), align 8
  %call21 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %8, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE4:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 1
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR]], {{.*}}, ptr addrspace(1) [[PIPE4]])
; CHECK:      %[[RET4:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) [[PIPE4]]
; CHECK-NEXT: [[ICMP4:%[0-9]+]] = icmp ne i32 %[[RET4]], 0
; CHECK-NEXT: br i1 [[ICMP4]], label %[[THEN4:[0-9]+]]
; CHECK:      [[THEN4]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  %9 = load ptr addrspace(1), ptr addrspace(1) @c5.pipe, align 8
  store i32 46, ptr %write.src, align 4
  %10 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %9, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE5:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c5.pipe
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR]], {{.*}}, ptr addrspace(1) [[PIPE5]])
; CHECK:      %[[RET5:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) [[PIPE5]]
; CHECK-NEXT: [[ICMP5:%[0-9]+]] = icmp ne i32 %[[RET5]], 0
; CHECK-NEXT: br i1 [[ICMP5]], label %[[THEN5:[0-9]+]]
; CHECK:      [[THEN5]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %val5) #2
  %11 = addrspacecast ptr %val5 to ptr addrspace(4)
  %12 = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %p, ptr addrspace(4) %11, i32 4, i32 4)
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR]], {{.*}}, ptr addrspace(1) %p)
; CHECK:      %[[RET6:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %p
; CHECK-NEXT: [[ICMP6:%[0-9]+]] = icmp ne i32 %[[RET6]], 0
; CHECK-NEXT: br i1 [[ICMP6]], label %[[THEN6:[0-9]+]]
; CHECK:      [[THEN6]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]

  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %val5) #2
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %val3) #2
  ret void
; CHECK:      call void @__flush_pipe_read_array(ptr [[READ_ARR]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR]]
; CHECK-NEXT: ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @bar(ptr addrspace(1) %p) local_unnamed_addr #1 !kernel_arg_addr_space !4 !kernel_arg_access_qual !13 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !arg_type_null_val !14 {
entry:
; CHECK:      [[WRITE_ARR1:%[0-9]+]] = alloca ptr addrspace(1), i32 37
; CHECK:      [[READ_ARR1:%[0-9]+]] = alloca ptr addrspace(1), i32 37

  %write.src = alloca %struct.channelpipetransformation.merge, align 8
  %read.dst = alloca i32, align 4
  %val3 = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c1.pipe, align 8
  store i32 42, ptr %read.dst, align 4
  %1 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %2 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE7:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c1.pipe
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR1]], {{.*}}, ptr addrspace(1) [[PIPE7]])
; CHECK:      %[[RET7:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) [[PIPE7]]
; CHECK-NEXT: [[ICMP7:%[0-9]+]] = icmp ne i32 %[[RET7]], 0
; CHECK-NEXT: br i1 [[ICMP7]], label %[[THEN7:[0-9]+]]
; CHECK:      [[THEN7]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  %3 = load ptr addrspace(1), ptr addrspace(1) @c2.pipe, align 8
  store i32 43, ptr %read.dst, align 4
  %4 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %3, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE8:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c2.pipe
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR1]], {{.*}}, ptr addrspace(1) [[PIPE8]])
; CHECK:      %[[RET8:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) [[PIPE8]]
; CHECK-NEXT: [[ICMP8:%[0-9]+]] = icmp ne i32 %[[RET8]], 0
; CHECK-NEXT: br i1 [[ICMP8]], label %[[THEN8:[0-9]+]]
; CHECK:      [[THEN8]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  %5 = load ptr addrspace(1), ptr addrspace(1) @c3.pipe, align 8
  %6 = getelementptr inbounds %struct.channelpipetransformation.merge, ptr %write.src, i64 0, i32 0
  store float 0.000000e+00, ptr %6, align 8
  %7 = getelementptr inbounds %struct.channelpipetransformation.merge, ptr %write.src, i64 0, i32 1
  store i64 0, ptr %7, align 8
  %8 = addrspacecast ptr %write.src to ptr addrspace(4)
  %9 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %5, ptr addrspace(4) %8, i32 16, i32 8) #2
; CHECK:      [[PIPE9:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c3.pipe
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR1]], {{.*}}, ptr addrspace(1) [[PIPE9]])
; CHECK:      %[[RET9:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) [[PIPE9]]
; CHECK-NEXT: [[ICMP9:%[0-9]+]] = icmp ne i32 %[[RET9]], 0
; CHECK-NEXT: br i1 [[ICMP9]], label %[[THEN9:[0-9]+]]
; CHECK:      [[THEN9]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  %10 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 2), align 16
  %call3 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %10, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE10:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 2
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR1]], {{.*}}, ptr addrspace(1) [[PIPE10]])
; CHECK:      %[[RET10:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) [[PIPE10]]
; CHECK-NEXT: [[ICMP10:%[0-9]+]] = icmp ne i32 %[[RET10]], 0
; CHECK-NEXT: br i1 [[ICMP10]], label %[[THEN10:[0-9]+]]
; CHECK:      [[THEN10]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  %11 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 1), align 8
  store i32 44, ptr %read.dst, align 4
  %12 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %11, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE11:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 1
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR1]], {{.*}}, ptr addrspace(1) [[PIPE11]])
; CHECK:      %[[RET11:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) [[PIPE11]]
; CHECK-NEXT: [[ICMP11:%[0-9]+]] = icmp ne i32 %[[RET11]], 0
; CHECK-NEXT: br i1 [[ICMP11]], label %[[THEN11:[0-9]+]]
; CHECK:      [[THEN11]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  %13 = load ptr addrspace(1), ptr addrspace(1) @c5.pipe, align 8
  %call12 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %13, ptr addrspace(4) %1, i32 4, i32 4) #2
; CHECK:      [[PIPE12:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @c5.pipe
; CHECK:      call void @__store_read_pipe_use(ptr [[READ_ARR1]], {{.*}}, ptr addrspace(1) [[PIPE12]])
; CHECK:      %[[RET12:.*]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) [[PIPE12]]
; CHECK-NEXT: [[ICMP12:%[0-9]+]] = icmp ne i32 %[[RET12]], 0
; CHECK-NEXT: br i1 [[ICMP12]], label %[[THEN12:[0-9]+]]
; CHECK:      [[THEN12]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %val3) #2
  store i32 47, ptr %val3, align 4, !tbaa !15
  %14 = addrspacecast ptr %val3 to ptr addrspace(4)
  %15 = call i32 @__write_pipe_2_fpga(ptr addrspace(1) %p, ptr addrspace(4) %14, i32 4, i32 4)
; CHECK:      call void @__store_write_pipe_use(ptr [[WRITE_ARR1]], {{.*}}, ptr addrspace(1) %p)
; CHECK:      %[[RET13:.*]] = call i32 @__write_pipe_2_fpga(ptr addrspace(1) %p
; CHECK-NEXT: [[ICMP13:%[0-9]+]] = icmp ne i32 %[[RET13]], 0
; CHECK-NEXT: br i1 [[ICMP13]], label %[[THEN13:[0-9]+]]
; CHECK:      [[THEN13]]:
; CHECK-NEXT: call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]

  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %val3) #2
  ret void
; CHECK:      call void @__flush_pipe_read_array(ptr [[READ_ARR1]]
; CHECK-NEXT: call void @__flush_pipe_write_array(ptr [[WRITE_ARR1]]
; CHECK-NEXT: ret void
}

; Function Attrs: convergent norecurse nounwind
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture noundef writeonly, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
declare i32 @__write_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture noundef readonly, i32 noundef, i32 noundef) local_unnamed_addr #1

define void @__pipe_global_ctor() {
entry:
  tail call void @__pipe_init_fpga(ptr addrspace(1) @c1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @c1.pipe.bs, ptr addrspace(1) @c1.pipe, align 8
  tail call void @__pipe_init_fpga(ptr addrspace(1) @c2.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @c2.pipe.bs, ptr addrspace(1) @c2.pipe, align 8
  tail call void @__pipe_init_fpga(ptr addrspace(1) @c3.pipe.bs, i32 16, i32 0, i32 0)
  store ptr addrspace(1) @c3.pipe.bs, ptr addrspace(1) @c3.pipe, align 8
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
  tail call void @__pipe_init_fpga(ptr addrspace(1) @c5.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @c5.pipe.bs, ptr addrspace(1) @c5.pipe, align 8
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
declare void @__pipe_init_array_fpga(ptr addrspace(1) nocapture noundef readonly, i32 noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32) local_unnamed_addr

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32) local_unnamed_addr

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { nounwind }

!sycl.kernels = !{!3}

!0 = !{i32 4}
!1 = !{i32 16}
!2 = !{i32 8}
!3 = !{ptr @foo, ptr @bar}
!4 = !{i32 1}
!5 = !{!"read_only"}
!6 = !{!"int"}
!7 = !{!"pipe"}
!8 = !{!"p"}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{!""}
!12 = !{target("spirv.Pipe", 0) zeroinitializer}
!13 = !{!"write_only"}
!14 = !{target("spirv.Pipe", 1) zeroinitializer}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
