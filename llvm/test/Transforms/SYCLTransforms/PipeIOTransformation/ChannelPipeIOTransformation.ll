; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; __kernel void k1(read_only pipe int p1 __attribute__((io("test1"))),
;                  write_only pipe int p2 __attribute__((io("test2"))),
;                  int i1, global float * f1) {
;   int data;
;   int i = read_pipe(p1, &data);
;   if (i > 0)
;     write_pipe(p2, &data);
; }
;
; channel int ichIn __attribute__((io("test_ch_in")));
; channel int ichOut __attribute__((io("test_ch_out")));
;
; __kernel void in_kernel() {
;   long data;
;   data = read_channel_intel(ichIn);
;   if (data > 0)
;     write_channel_intel(ichOut, data);
; }
; ----------------------------------------------------
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-demangle-fpga-pipes -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer,sycl-kernel-channel-pipe-transformation -verify %s -S
; ----------------------------------------------------

; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-io-transform %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-io-transform %s -S | FileCheck %s

; CHECK-DAG: @llvm.global_dtors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_dtor, ptr null }]
; CHECK-DAG: @test1.str = private unnamed_addr constant [6 x i8] c"test1\00", align 1
; CHECK-DAG: @test2.str = private unnamed_addr constant [6 x i8] c"test2\00", align 1
; CHECK-DAG: @test_ch_in.str = private unnamed_addr constant [11 x i8] c"test_ch_in\00", align 1

; CHECK: define dso_local void @in_kernel
; CHECK: %[[PIPE1RW:[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @ichIn.pipe
; CHECK: %[[TESTCHINSTR:[0-9]+]] = addrspacecast ptr @test_ch_in.str to ptr addrspace(4)
; CHECK: call i32 @__read_pipe_2_bl_io_fpga(ptr addrspace(1) %[[PIPE1RW]], ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %[[TESTCHINSTR]], i32 4, i32 4)
; CHECK: %[[PIPE2RW:[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @ichOut.pipe
; CHECK: %[[TESTCHOUTSTR:[0-9]+]] = addrspacecast ptr @test_ch_out.str to ptr addrspace(4)
; CHECK: call i32 @__write_pipe_2_bl_io_fpga(ptr addrspace(1) %[[PIPE2RW]], ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %[[TESTCHOUTSTR]], i32 4, i32 4)
;
; CHECK: define dso_local void @k1
; CHECK: %[[PIPERO:[0-9]+]] = load ptr addrspace(1), ptr %p1.addr
; CHECK: %[[TEST1STR:[0-9]+]] = addrspacecast ptr @test1.str to ptr addrspace(4)
; CHECK: call i32 @__read_pipe_2_io_fpga(ptr addrspace(1) %[[PIPERO]], ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %[[TEST1STR]], i32 4, i32 4)
; CHECK: %[[PIPEWO:[0-9]+]] = load ptr addrspace(1), ptr %p2.addr
; CHECK: %[[TEST2STR:[0-9]+]] = addrspacecast ptr @test2.str to ptr addrspace(4)
; CHECK: call i32 @__write_pipe_2_io_fpga(ptr addrspace(1) %[[PIPEWO]], ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %[[TEST2STR]], i32 4, i32 4)
;
; CHECK: define void @__pipe_global_dtor()
; CHECK: call void @__pipe_release_fpga(ptr addrspace(1) @ichIn.pipe)
; CHECK: call void @__pipe_release_fpga(ptr addrspace(1) @ichOut.pipe)
; CHECK: declare void @__pipe_release_fpga(ptr addrspace(1) nocapture readonly)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ichIn = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !io !1
@ichOut = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !io !2
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ichIn.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !io !1
@ichIn.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ichOut.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !io !2
@ichOut.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: convergent norecurse nounwind
define dso_local void @in_kernel() #1 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !arg_type_null_val !4 {
entry:
  %write.src = alloca i32, align 4
  %data = alloca i64, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %data) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @ichIn.pipe, align 8, !tbaa !7
  %1 = load ptr addrspace(1), ptr addrspace(1) @ichIn, align 4, !tbaa !7
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %3 = load i32, ptr %write.src, align 4
  %conv = sext i32 %3 to i64
  store i64 %conv, ptr %data, align 8, !tbaa !10
  %4 = load i64, ptr %data, align 8, !tbaa !10
  %cmp = icmp sgt i64 %4, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %5 = load ptr addrspace(1), ptr addrspace(1) @ichOut.pipe, align 8, !tbaa !7
  %6 = load ptr addrspace(1), ptr addrspace(1) @ichOut, align 4, !tbaa !7
  %7 = load i64, ptr %data, align 8, !tbaa !10
  %conv2 = trunc i64 %7 to i32
  store i32 %conv2, ptr %write.src, align 4
  %8 = addrspacecast ptr %write.src to ptr addrspace(4)
  %9 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %5, ptr addrspace(4) %8, i32 4, i32 4)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @llvm.lifetime.end.p0(i64 8, ptr %data) #3
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @k1(ptr addrspace(1) %p1, ptr addrspace(1) %p2, i32 noundef %i1, ptr addrspace(1) noundef align 4 %f1) #1 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_base_type !14 !kernel_arg_type_qual !15 !kernel_arg_host_accessible !16 !kernel_arg_pipe_depth !17 !kernel_arg_pipe_io !18 !kernel_arg_buffer_location !19 !arg_type_null_val !20 {
entry:
  %p1.addr = alloca ptr addrspace(1), align 8
  %p2.addr = alloca ptr addrspace(1), align 8
  %i1.addr = alloca i32, align 4
  %f1.addr = alloca ptr addrspace(1), align 8
  %data = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr addrspace(1) %p1, ptr %p1.addr, align 8, !tbaa !7
  store ptr addrspace(1) %p2, ptr %p2.addr, align 8, !tbaa !7
  store i32 %i1, ptr %i1.addr, align 4, !tbaa !21
  store ptr addrspace(1) %f1, ptr %f1.addr, align 8, !tbaa !23
  call void @llvm.lifetime.start.p0(i64 4, ptr %data) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %0 = load ptr addrspace(1), ptr %p1.addr, align 8, !tbaa !7
  %1 = addrspacecast ptr %data to ptr addrspace(4)
  %2 = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4)
  store i32 %2, ptr %i, align 4, !tbaa !21
  %3 = load i32, ptr %i, align 4, !tbaa !21
  %cmp = icmp sgt i32 %3, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %4 = load ptr addrspace(1), ptr %p2.addr, align 8, !tbaa !7
  %5 = addrspacecast ptr %data to ptr addrspace(4)
  %6 = call i32 @__write_pipe_2_fpga(ptr addrspace(1) %4, ptr addrspace(4) %5, i32 4, i32 4)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %data) #3
  ret void
}

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #2

; Function Attrs: nounwind memory(none)
declare i32 @__write_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture readonly, i32, i32) #2

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ichIn.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ichIn.pipe.bs, ptr addrspace(1) @ichIn.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ichOut.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ichOut.pipe.bs, ptr addrspace(1) @ichOut.pipe, align 8
  ret void
}

; Function Attrs: nounwind memory(none)
declare void @__pipe_init_fpga(ptr addrspace(1), i32, i32, i32) #2

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #2 = { nounwind memory(none) }
attributes #3 = { nounwind }

!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}
!sycl.kernels = !{!6}

!0 = !{i32 4}
!1 = !{!"test_ch_in"}
!2 = !{!"test_ch_out"}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!6 = !{ptr @in_kernel, ptr @k1}
!7 = !{!8, !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"long", !8, i64 0}
!12 = !{i32 1, i32 1, i32 0, i32 1}
!13 = !{!"read_only", !"write_only", !"none", !"none"}
!14 = !{!"int", !"int", !"int", !"float*"}
!15 = !{!"pipe", !"pipe", !"", !""}
!16 = !{i1 false, i1 false, i1 false, i1 false}
!17 = !{i32 0, i32 0, i32 0, i32 0}
!18 = !{!"test1", !"test2", !"", !""}
!19 = !{!"", !"", !"", !""}
!20 = !{target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 1) zeroinitializer, i32 0, ptr addrspace(1) null}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !8, i64 0}
!23 = !{!24, !24, i64 0}
!24 = !{!"any pointer", !8, i64 0}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-5: WARNING: Instruction with empty DebugLoc in function __pipe_global_dtor
; DEBUGIFY-NOT: WARNING
