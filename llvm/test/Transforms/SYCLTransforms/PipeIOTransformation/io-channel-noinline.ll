; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-io-transform %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-io-transform %s -S | FileCheck %s
;
; This test checks that io channel is replaced with builtin correctly when
; user-defined functions are not inlined.
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-demangle-fpga-pipes -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer,sycl-kernel-channel-pipe-transformation -verify %s -S
;
; #pragma OPENCL EXTENSION cl_intel_channels:enable
; channel int ch_in;
; channel int ch_in0 __attribute__((io("ch_in0")));
; channel int ch_in1 __attribute__((io("ch_in1")));
; channel int ch_out0 __attribute__((io("ch_out0")));
; channel int ch_out1 __attribute__((io("ch_out1")));
;
; __attribute__((noinline))
; int bar(channel int ch0, channel int ch1) {
;   return read_channel_intel(ch0);
; }
;
; __attribute__((noinline))
; int foo(channel int ch0, channel int ch1) {
;   return bar(ch0, ch1);
; }
;
; __attribute__((noinline))
; int readChannelHelper(channel int io_channel) {
;   return read_channel_intel(io_channel);
; }
;
; kernel void readFileViaChannel(global int *dst) {
;   *dst = readChannelHelper(ch_in0) + readChannelHelper(ch_in) +
;          foo(ch_in0, ch_in) + foo(ch_in, ch_in1);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch_in0 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !io !1
@ch_in = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@ch_in1 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !io !2
@ch_out0 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !io !3
@ch_out1 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !io !4
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch_in0.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !io !1
@ch_in0.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ch_in.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch_in.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ch_in1.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !io !2
@ch_in1.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ch_out0.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !io !3
@ch_out0.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ch_out1.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !io !4
@ch_out1.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

; CHECK: @ch_in0.str = private unnamed_addr constant [7 x i8] c"ch_in0\00"
; CHECK: @ch_in1.str = private unnamed_addr constant [7 x i8] c"ch_in1\00"
; CHECK: @ch_out0.str = private unnamed_addr constant [8 x i8] c"ch_out0\00"
; CHECK: @ch_out1.str = private unnamed_addr constant [8 x i8] c"ch_out1\00"

; CHECK-LABEL: @readFileViaChannel
; CHECK: call i32 @__io_pipe_0_0_readChannelHelper
; CHECK: call i32 @readChannelHelper
; CHECK: call i32 @__io_pipe_0_0_foo
; CHECK: call i32 @__io_pipe_1_1_foo

; Function Attrs: convergent norecurse nounwind
define dso_local void @readFileViaChannel(ptr addrspace(1) noundef align 4 %dst) #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !12 !kernel_arg_buffer_location !12 !arg_type_null_val !15 {
entry:
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8, !tbaa !16
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch_in0.pipe, align 8, !tbaa !20
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch_in0, align 4, !tbaa !20
  %2 = call i32 @readChannelHelper(ptr addrspace(1) %0) #3
  %3 = load ptr addrspace(1), ptr addrspace(1) @ch_in.pipe, align 8, !tbaa !20
  %4 = load ptr addrspace(1), ptr addrspace(1) @ch_in, align 4, !tbaa !20
  %5 = call i32 @readChannelHelper(ptr addrspace(1) %3) #3
  %add = add nsw i32 %2, %5
  %6 = load ptr addrspace(1), ptr addrspace(1) @ch_in0.pipe, align 8, !tbaa !20
  %7 = load ptr addrspace(1), ptr addrspace(1) @ch_in0, align 4, !tbaa !20
  %8 = load ptr addrspace(1), ptr addrspace(1) @ch_in.pipe, align 8, !tbaa !20
  %9 = load ptr addrspace(1), ptr addrspace(1) @ch_in, align 4, !tbaa !20
  %10 = call i32 @foo(ptr addrspace(1) %6, ptr addrspace(1) %8) #3
  %add3 = add nsw i32 %add, %10
  %11 = load ptr addrspace(1), ptr addrspace(1) @ch_in.pipe, align 8, !tbaa !20
  %12 = load ptr addrspace(1), ptr addrspace(1) @ch_in, align 4, !tbaa !20
  %13 = load ptr addrspace(1), ptr addrspace(1) @ch_in1.pipe, align 8, !tbaa !20
  %14 = load ptr addrspace(1), ptr addrspace(1) @ch_in1, align 4, !tbaa !20
  %15 = call i32 @foo(ptr addrspace(1) %11, ptr addrspace(1) %13) #3
  %add5 = add nsw i32 %add3, %15
  %16 = load ptr addrspace(1), ptr %dst.addr, align 8, !tbaa !16
  store i32 %add5, ptr addrspace(1) %16, align 4, !tbaa !21
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch_in0.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch_in0.pipe.bs, ptr addrspace(1) @ch_in0.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch_in.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch_in.pipe.bs, ptr addrspace(1) @ch_in.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch_in1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch_in1.pipe.bs, ptr addrspace(1) @ch_in1.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch_out0.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch_out0.pipe.bs, ptr addrspace(1) @ch_out0.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch_out1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch_out1.pipe.bs, ptr addrspace(1) @ch_out1.pipe, align 8
  ret void
}

; Function Attrs: nounwind memory(none)
declare void @__pipe_init_fpga(ptr addrspace(1), i32, i32, i32) #1

; CHECK-LABEL: @bar
; CHECK: call i32 @__read_pipe_2_bl_fpga
; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @bar(ptr addrspace(1) %ch0, ptr addrspace(1) %ch1) #2 !arg_type_null_val !23 {
entry:
  %read.dst = alloca i32, align 4
  %pipe.ch0.addr = alloca ptr addrspace(1), align 8
  %ch0.addr = alloca ptr addrspace(1), align 4
  %pipe.ch1.addr = alloca ptr addrspace(1), align 8
  %ch1.addr = alloca ptr addrspace(1), align 4
  store ptr addrspace(1) %ch0, ptr %pipe.ch0.addr, align 4, !tbaa !20
  store ptr addrspace(1) %ch1, ptr %pipe.ch1.addr, align 4, !tbaa !20
  %0 = load ptr addrspace(1), ptr %pipe.ch0.addr, align 8, !tbaa !20
  %1 = load ptr addrspace(1), ptr %ch0.addr, align 4, !tbaa !20
  %2 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %3 = load i32, ptr %read.dst, align 4
  ret i32 %3
}

; CHECK-LABEL: @foo
; CHECK: call i32 @bar
; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @foo(ptr addrspace(1) %ch0, ptr addrspace(1) %ch1) #2 !arg_type_null_val !23 {
entry:
  %pipe.ch0.addr = alloca ptr addrspace(1), align 8
  %ch0.addr = alloca ptr addrspace(1), align 4
  %pipe.ch1.addr = alloca ptr addrspace(1), align 8
  %ch1.addr = alloca ptr addrspace(1), align 4
  store ptr addrspace(1) %ch0, ptr %pipe.ch0.addr, align 4, !tbaa !20
  store ptr addrspace(1) %ch1, ptr %pipe.ch1.addr, align 4, !tbaa !20
  %0 = load ptr addrspace(1), ptr %pipe.ch0.addr, align 8, !tbaa !20
  %1 = load ptr addrspace(1), ptr %ch0.addr, align 4, !tbaa !20
  %2 = load ptr addrspace(1), ptr %pipe.ch1.addr, align 8, !tbaa !20
  %3 = load ptr addrspace(1), ptr %ch1.addr, align 4, !tbaa !20
  %4 = call i32 @bar(ptr addrspace(1) %0, ptr addrspace(1) %2) #3
  ret i32 %4
}

; CHECK-LABEL: @readChannelHelper
; CHECK: call i32 @__read_pipe_2_bl_fpga
; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @readChannelHelper(ptr addrspace(1) %io_channel) #2 !arg_type_null_val !24 {
entry:
  %read.dst = alloca i32, align 4
  %pipe.io_channel.addr = alloca ptr addrspace(1), align 8
  %io_channel.addr = alloca ptr addrspace(1), align 4
  store ptr addrspace(1) %io_channel, ptr %pipe.io_channel.addr, align 4, !tbaa !20
  %0 = load ptr addrspace(1), ptr %pipe.io_channel.addr, align 8, !tbaa !20
  %1 = load ptr addrspace(1), ptr %io_channel.addr, align 4, !tbaa !20
  %2 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %3 = load i32, ptr %read.dst, align 4
  ret i32 %3
}

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)


; CHECK: i32 @__io_pipe_0_0_foo
; CHECK: i32 @__io_pipe_0_0_readChannelHelper
; CHECK: i32 @__io_pipe_1_1_foo
; CHECK: i32 @__io_pipe_0_0_bar
; CHECK: i32 @__io_pipe_1_1_bar

; CHECK: declare i32 @__read_pipe_2_bl_io_fpga

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nounwind memory(none) }
attributes #2 = { convergent noinline norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind }

!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!sycl.kernels = !{!8}

!0 = !{i32 4}
!1 = !{!"ch_in0"}
!2 = !{!"ch_in1"}
!3 = !{!"ch_out0"}
!4 = !{!"ch_out1"}
!5 = !{i32 1, i32 2}
!6 = !{}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!8 = !{ptr @readFileViaChannel}
!9 = !{i32 1}
!10 = !{!"none"}
!11 = !{!"int*"}
!12 = !{!""}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{ptr addrspace(1) null}
!16 = !{!17, !17, i64 0}
!17 = !{!"any pointer", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!18, !18, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !18, i64 0}
!23 = !{target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}
!24 = !{target("spirv.Channel") zeroinitializer}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-9: WARNING: Instruction with empty DebugLoc in function __pipe_global_dtor
; DEBUGIFY-NOT: WARNING
