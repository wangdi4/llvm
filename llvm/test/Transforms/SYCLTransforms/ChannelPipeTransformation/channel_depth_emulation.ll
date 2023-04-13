; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; __kernel void foo() {
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=2 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=2 %s -S | FileCheck %s --check-prefixes=CHECK,IGNOREDEPTH
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=1 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=1 %s -S | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=0 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=0 %s -S | FileCheck %s --check-prefixes=CHECK,STRICT

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
; DEFAULT: @ch.pipe {{.*}} %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]
; CHECK-NOT: @ch.pipe {{.*}] %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]
; IGNOREDEPTH-NOT: @ch.pipe {{.*}] %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]
; STRICT-NOT: @ch.pipe {{.*}] %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]

; CHECK: define {{.*}}@__pipe_global_ctor
; IGNOREDEPTH: call void @__pipe_init_fpga({{.*}} @ch.pipe.bs {{.*}}, i32 4, i32 0, i32 2)
; DEFAULT: call void @__pipe_init_fpga({{.*}} @ch.pipe.bs {{.*}}, i32 4, i32 0, i32 1)
; STRICT: call void @__pipe_init_fpga({{.*}} @ch.pipe.bs {{.*}}, i32 4, i32 0, i32 0)

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

; DEFAULT: ![[MD]] = !{i1 true}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 6.0.0 "}

; DEBUGIFY-NOT: WARNING: Missing line
