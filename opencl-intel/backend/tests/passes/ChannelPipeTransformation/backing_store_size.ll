; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel int bar __attribute__((depth(0)));
; channel float far __attribute__((depth(24)));
;
; struct Foo {
;   int i;
; };
; channel struct Foo star;
;
; channel int bar_arr[5] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(24)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation --channel-depth-emulation-mode=2 -verify %s -S | FileCheck %s --check-prefixes=CHECK,IGNOREDEPTH
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation --channel-depth-emulation-mode=1 -verify %s -S | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation --channel-depth-emulation-mode=0 -verify %s -S | FileCheck %s --check-prefixes=CHECK,STRICT
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !depth !2
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK-DAG: @[[PIPE_BAR:bar.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*
; CHECK-DAG: @[[PIPE_FAR:far.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*
; CHECK-DAG: @[[PIPE_STAR:star.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*
; CHECK-DAG: @[[PIPE_BAR_ARR:bar_arr.*]] = addrspace(1) global [5 x %opencl.pipe_rw_t addrspace(1)*]
; CHECK-DAG: @[[PIPE_FAR_ARR:far_arr.*]] = addrspace(1) global [5 x [4 x %opencl.pipe_rw_t addrspace(1)*]]
; CHECK-DAG: @[[PIPE_STAR_ARR:star_arr.*]] = addrspace(1) global [5 x [4 x [3 x %opencl.pipe_rw_t addrspace(1)*]]]
;
; size of pipe backing store is calculated as:
;   sizeof(__pipe_t) + packet_size * __get_pipe_max_packets(depth, mode)
; where __pipe_t is defined as:
;   %struct.__pipe_t = type { i32, i32, [56 x i8], i32, [60 x i8], i32, [60 x i8], %struct.__pipe_internal_buf, [52 x i8], %struct.__pipe_internal_buf, [52 x i8] }
;   %struct.__pipe_internal_buf = type { i32, i32, i32 }
;
; so, sizeof(__pipe_t) == 320 bytes
;
; __get_pipe_max_packets(depth, mode) by default (ignore-depth mode):
;   max(depth, MAX_VL_SUPPORTED_BY_PIPES) + 1 + PIPE_WRITE_BUF_PREFERRED_LIMIT - 1 = max(depth, 16) + 256
;
; for all channels in this test packet_size is 4 bytes
;
; bar: 320 + 4 * (max(0, 16) + 256) = 320 + 4 * 272 = 320 + 1088 = 1408
; far: 320 + 4 * (max(24, 16) + 256) = 320 + 4 * 280 = 320 + 1120 = 1440
; star: 320 + 4 * (max(0, 16) + 256) = 320 + 4 * 272 = 320 + 1088 = 1408
;
; IGNOREDEPTH-DAG: @[[PIPE_BAR]].bs = addrspace(1) global [1408 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_FAR]].bs = addrspace(1) global [1440 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_STAR]].bs = addrspace(1) global [1408 x i8]
;
; for arrays bs size = bs size of one pipe * num of pipes
;
; bar_arr: 1408 * 5 = 7040
; far_arr: 1440 * 5 * 4 = 28800
; star_arr: 1408 * 5 * 4 * 3 = 84480
;
; IGNOREDEPTH-DAG: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [7040 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [28800 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [84480 x i8]
;
;
; for default mode:  __get_pipe_max_packets(depth, mode):
;   if depth == 0:
;     the same as for ignore-depth mode
;   else:
;     depth + 1
;
; bar: 320 + 4 * (max(0, 16) + 256) = 320 + 4 * 272 = 320 + 1088 = 1408
; far: 320 + 4 * (24 + 1) = 320 + 4 * 25 = 320 + 100 = 420
; star: 320 + 4 * (max(0, 16) + 256) = 320 + 4 * 272 = 320 + 1088 = 1408
;
; DEFAULT-DAG: @[[PIPE_BAR]].bs = addrspace(1) global [1408 x i8]
; DEFAULT-DAG: @[[PIPE_FAR]].bs = addrspace(1) global [420 x i8]
; DEFAULT-DAG: @[[PIPE_STAR]].bs = addrspace(1) global [1408 x i8]
;
; for arrays bs size = bs size of one pipe * num of pipes
;
; bar_arr: 1408 * 5 = 7040
; far_arr: 420 * 5 * 4 = 8400
; star_arr: 1408 * 5 * 4 * 3 = 84480
;
; DEFAULT-DAG: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [7040 x i8]
; DEFAULT-DAG: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [8400 x i8]
; DEFAULT-DAG: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [84480 x i8]
;
;
; for strict mode:  __get_pipe_max_packets(depth, mode):
;   if depth == 0:
;     depth = 1
;
;   max_packets = depth + 1
;
; bar: 320 + 4 * 2 = 320 + 4 * 2 = 320 + 8 = 328
; far: 320 + 4 * (24 + 1) = 320 + 4 * 25 = 320 + 100 = 420
; star: 320 + 4 * 2 = 320 + 4 * 2 = 320 + 8 = 328
;
; STRICT-DAG: @[[PIPE_BAR]].bs = addrspace(1) global [328 x i8]
; STRICT-DAG: @[[PIPE_FAR]].bs = addrspace(1) global [420 x i8]
; STRICT-DAG: @[[PIPE_STAR]].bs = addrspace(1) global [328 x i8]
;
; for arrays bs size = bs size of one pipe * num of pipes
;
; bar_arr: 328 * 5 = 1640
; far_arr: 420 * 5 * 4 = 8400
; star_arr: 328 * 5 * 4 * 3 = 19680
;
; STRICT-DAG: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [1640 x i8]
; STRICT-DAG: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [8400 x i8]
; STRICT-DAG: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [19680 x i8]

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 {
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 24}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 1, i32 2}
!5 = !{}
!6 = !{!"clang version 6.0.0 "}
