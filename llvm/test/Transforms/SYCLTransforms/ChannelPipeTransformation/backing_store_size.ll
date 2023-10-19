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
; opt -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer %s -S
; ----------------------------------------------------

; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -sycl-channel-depth-emulation-mode=2 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -sycl-channel-depth-emulation-mode=2 %s -S | FileCheck %s --check-prefixes=CHECK,IGNOREDEPTH
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -sycl-channel-depth-emulation-mode=1 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -sycl-channel-depth-emulation-mode=1 %s -S | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -sycl-channel-depth-emulation-mode=0 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -sycl-channel-depth-emulation-mode=0 %s -S | FileCheck %s --check-prefixes=CHECK,STRICT

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@bar = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !2
@star = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
@star_arr = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK-DAG: @[[PIPE_BAR:bar.*]] = addrspace(1) global ptr addrspace(1)
; CHECK-DAG: @[[PIPE_FAR:far.*]] = addrspace(1) global ptr addrspace(1)
; CHECK-DAG: @[[PIPE_STAR:star.*]] = addrspace(1) global ptr addrspace(1)
; CHECK-DAG: @[[PIPE_BAR_ARR:bar_arr.*]] = addrspace(1) global [5 x ptr addrspace(1)]
; CHECK-DAG: @[[PIPE_FAR_ARR:far_arr.*]] = addrspace(1) global [5 x [4 x ptr addrspace(1)]]
; CHECK-DAG: @[[PIPE_STAR_ARR:star_arr.*]] = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]]
;
; size of pipe backing store is calculated as:
;   sizeof(__pipe_t) + packet_size * __get_pipe_max_packets(depth, mode)
; where __pipe_t is defined as:
;   %struct.__pipe_t = type { i32, i32, [56 x i8], i32, [60 x i8], i32, [60 x i8], %struct.__pipe_internal_buf, [52 x i8], %struct.__pipe_internal_buf, [52 x i8] }
;   %struct.__pipe_internal_buf = type { i32, i32, i32 }
;
; so, sizeof(__pipe_t) == 448 bytes
;
; __get_pipe_max_packets(depth, mode) by default (ignore-depth mode):
;   max(depth, MAX_VL_SUPPORTED_BY_PIPES) + 1 + PIPE_WRITE_BUF_PREFERRED_LIMIT - 1 = max(depth, 16) + 256
;
; for all channels in this test packet_size is 4 bytes
;
; bar: 448 + 4 * (max(0, 16) + 256) = 448 + 4 * 272 = 448 + 1088 = 1536
; far: 448 + 4 * (max(24, 16) + 256) = 448 + 4 * 280 = 448 + 1120 = 1568
; star: 448 + 4 * (max(0, 16) + 256) = 448 + 4 * 272 = 448 + 1088 = 1536
;
; IGNOREDEPTH-DAG: @[[PIPE_BAR]].bs = addrspace(1) global [1536 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_FAR]].bs = addrspace(1) global [1568 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_STAR]].bs = addrspace(1) global [1536 x i8]
;
; for arrays bs size = bs size of one pipe * num of pipes
;
; bar_arr: 1536 * 5 = 7680
; far_arr: 1568 * 5 * 4 = 31360
; star_arr: 1536 * 5 * 4 * 3 = 92160
;
; IGNOREDEPTH-DAG: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [7680 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [31360 x i8]
; IGNOREDEPTH-DAG: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [92160 x i8]
;
;
; for default mode:  __get_pipe_max_packets(depth, mode):
;   if depth == 0:
;     the same as for ignore-depth mode
;   else:
;     depth + 1
;
; bar: 448 + 4 * (max(0, 16) + 256) = 448 + 4 * 272 = 448 + 1088 = 1536
; far: 448 + 4 * (24 + 1) = 448 + 4 * 25 = 448 + 100 = 548
; star: 448 + 4 * (max(0, 16) + 256) = 448 + 4 * 272 = 448 + 1088 = 1536
;
; DEFAULT-DAG: @[[PIPE_BAR]].bs = addrspace(1) global [1536 x i8]
; DEFAULT-DAG: @[[PIPE_FAR]].bs = addrspace(1) global [548 x i8]
; DEFAULT-DAG: @[[PIPE_STAR]].bs = addrspace(1) global [1536 x i8]
;
; for arrays bs size = bs size of one pipe * num of pipes
;
; bar_arr: 1536 * 5 = 7680
; far_arr: 548 * 5 * 4 = 10960
; star_arr: 1536 * 5 * 4 * 3 = 92160
;
; DEFAULT-DAG: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [7680 x i8]
; DEFAULT-DAG: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [10960 x i8]
; DEFAULT-DAG: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [92160 x i8]
;
;
; for strict mode:  __get_pipe_max_packets(depth, mode):
;   if depth == 0:
;     depth = 1
;
;   max_packets = depth + 1
;
; bar: 448 + 4 * 2 = 448 + 4 * 2 = 448 + 8 = 456
; far: 448 + 4 * (24 + 1) = 448 + 4 * 25 = 448 + 100 = 548
; star: 448 + 4 * 2 = 448 + 4 * 2 = 448 + 8 = 456
;
; STRICT-DAG: @[[PIPE_BAR]].bs = addrspace(1) global [456 x i8]
; STRICT-DAG: @[[PIPE_FAR]].bs = addrspace(1) global [548 x i8]
; STRICT-DAG: @[[PIPE_STAR]].bs = addrspace(1) global [456 x i8]
;
; for arrays bs size = bs size of one pipe * num of pipes
;
; bar_arr: 456 * 5 = 2280
; far_arr: 548 * 5 * 4 = 10960
; star_arr: 456 * 5 * 4 * 3 = 27360
;
; STRICT-DAG: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [2280 x i8]
; STRICT-DAG: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [10960 x i8]
; STRICT-DAG: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [27360 x i8]

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !arg_type_null_val !4 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }

!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}
!sycl.kernels = !{!6}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 24}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!6 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING: Missing line
