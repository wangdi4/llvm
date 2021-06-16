; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-io-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-io-transformation %s -S | FileCheck %s
;
; This test checks that io channel is replaced with builtin correctly when
; user-defined functions are not inlined.
; IR is dumped from VOLCANO_LLVM_OPTIONS=-print-before=pipe-io-transformation
; from following cl source:
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

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_ro_t = type opaque

@ch_in0 = internal addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !1
@ch_in = internal addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@ch_in1 = internal addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !2
@ch_out0 = internal addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !3
@ch_out1 = internal addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !4
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@ch_in0.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0, !io !1
@ch_in0.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch_in.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@ch_in.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch_in1.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0, !io !2
@ch_in1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch_out0.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0, !io !3
@ch_out0.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch_out1.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0, !io !4
@ch_out1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; CHECK: @ch_in0.str = private unnamed_addr constant [7 x i8] c"ch_in0\00"
; CHECK: @ch_in1.str = private unnamed_addr constant [7 x i8] c"ch_in1\00"
; CHECK: @ch_out0.str = private unnamed_addr constant [8 x i8] c"ch_out0\00"
; CHECK: @ch_out1.str = private unnamed_addr constant [8 x i8] c"ch_out1\00"

; Function Attrs: convergent norecurse nounwind
define void @readFileViaChannel(i32 addrspace(1)* %dst) #0 !kernel_arg_addr_space !10 !kernel_arg_access_qual !11 !kernel_arg_type !12 !kernel_arg_base_type !12 !kernel_arg_type_qual !13 !kernel_arg_name !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !13 !kernel_arg_buffer_location !13 {
; CHECK-LABEL: @readFileViaChannel
; CHECK: call i32 @__io_pipe_0_0_readChannelHelper
; CHECK: call i32 @readChannelHelper
; CHECK: call i32 @__io_pipe_0_0_foo
; CHECK: call i32 @__io_pipe_1_1_foo
entry:
  %0 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in0.pipe, align 8
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch_in0, align 4, !tbaa !17
  %2 = call i32 @readChannelHelper(%opencl.pipe_rw_t addrspace(1)* %0)
  %3 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in.pipe, align 8
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch_in, align 4, !tbaa !17
  %5 = call i32 @readChannelHelper(%opencl.pipe_rw_t addrspace(1)* %3)
  %add = add nsw i32 %2, %5
  %6 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in0.pipe, align 8
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch_in0, align 4, !tbaa !17
  %8 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in.pipe, align 8
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch_in, align 4, !tbaa !17
  %10 = call i32 @foo(%opencl.pipe_rw_t addrspace(1)* %6, %opencl.pipe_rw_t addrspace(1)* %8)
  %add3 = add nsw i32 %add, %10
  %11 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in.pipe, align 8
  %12 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch_in, align 4, !tbaa !17
  %13 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in1.pipe, align 8
  %14 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch_in1, align 4, !tbaa !17
  %15 = call i32 @foo(%opencl.pipe_rw_t addrspace(1)* %11, %opencl.pipe_rw_t addrspace(1)* %13)
  %add5 = add nsw i32 %add3, %15
  store i32 %add5, i32 addrspace(1)* %dst, align 4, !tbaa !20
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(i8 addrspace(1)* getelementptr inbounds ([328 x i8], [328 x i8] addrspace(1)* @ch_in0.pipe.bs, i32 0, i32 0), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch_in0.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in0.pipe, align 8
  call void @__pipe_init_fpga(i8 addrspace(1)* getelementptr inbounds ([328 x i8], [328 x i8] addrspace(1)* @ch_in.pipe.bs, i32 0, i32 0), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch_in.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in.pipe, align 8
  call void @__pipe_init_fpga(i8 addrspace(1)* getelementptr inbounds ([328 x i8], [328 x i8] addrspace(1)* @ch_in1.pipe.bs, i32 0, i32 0), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch_in1.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_in1.pipe, align 8
  call void @__pipe_init_fpga(i8 addrspace(1)* getelementptr inbounds ([328 x i8], [328 x i8] addrspace(1)* @ch_out0.pipe.bs, i32 0, i32 0), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch_out0.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_out0.pipe, align 8
  call void @__pipe_init_fpga(i8 addrspace(1)* getelementptr inbounds ([328 x i8], [328 x i8] addrspace(1)* @ch_out1.pipe.bs, i32 0, i32 0), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch_out1.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch_out1.pipe, align 8
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare void @__pipe_init_fpga(i8 addrspace(1)*, i32, i32, i32) #1

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @foo(%opencl.pipe_rw_t addrspace(1)* %0, %opencl.pipe_rw_t addrspace(1)* %1) #2 {
; CHECK-LABEL: @foo
; CHECK: call i32 @bar
entry:
  %2 = call i32 @bar(%opencl.pipe_rw_t addrspace(1)* %0, %opencl.pipe_rw_t addrspace(1)* %1)
  ret i32 %2
}

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @readChannelHelper(%opencl.pipe_rw_t addrspace(1)* %0) #2 {
; CHECK-LABEL: @readChannelHelper
; CHECK: call i32 @__read_pipe_2_bl_fpga
entry:
  %read.dst = alloca i32, align 4
  %1 = bitcast %opencl.pipe_rw_t addrspace(1)* %0 to %opencl.pipe_ro_t addrspace(1)*
  %2 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %1, i8 addrspace(4)* %2, i32 4, i32 4)
  %3 = load i32, i32* %read.dst, align 4
  ret i32 %3
}

; Function Attrs: convergent norecurse nounwind
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @bar(%opencl.pipe_rw_t addrspace(1)* %0, %opencl.pipe_rw_t addrspace(1)* %1) #2 {
; CHECK-LABEL: @bar
; CHECK: call i32 @__read_pipe_2_bl_fpga
entry:
  %read.dst = alloca i32, align 4
  %2 = bitcast %opencl.pipe_rw_t addrspace(1)* %0 to %opencl.pipe_ro_t addrspace(1)*
  %3 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %2, i8 addrspace(4)* %3, i32 4, i32 4)
  %4 = load i32, i32* %read.dst, align 4
  ret i32 %4
}

; CHECK: internal i32 @__io_pipe_0_0_foo
; CHECK: internal i32 @__io_pipe_0_0_readChannelHelper
; CHECK: internal i32 @__io_pipe_1_1_foo
; CHECK: internal i32 @__io_pipe_0_0_bar
; CHECK: internal i32 @__io_pipe_1_1_bar

; CHECK: declare i32 @__read_pipe_2_bl_io_fpga

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!7}
!llvm.ident = !{!8}
!sycl.kernels = !{!9}

!0 = !{i32 4}
!1 = !{!"ch_in0"}
!2 = !{!"ch_in1"}
!3 = !{!"ch_out0"}
!4 = !{!"ch_out1"}
!5 = !{i32 2, i32 0}
!6 = !{}
!7 = !{!"-cl-std=CL2.0"}
!8 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!9 = !{void (i32 addrspace(1)*)* @readFileViaChannel}
!10 = !{i32 1}
!11 = !{!"none"}
!12 = !{!"int*"}
!13 = !{!""}
!14 = !{!"dst"}
!15 = !{i1 false}
!16 = !{i32 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !18, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
