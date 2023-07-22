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
; opt -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer %s -S
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=2 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=2 %s -S | FileCheck %s --check-prefixes=CHECK,IGNOREDEPTH
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=1 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=1 %s -S | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=0 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=0 %s -S | FileCheck %s --check-prefixes=CHECK,STRICT

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0

; DEFAULT: @ch.pipe = addrspace(1) global ptr addrspace(1) null, {{.*}} !depth_is_ignored ![[MD:[0-9]+]]
; CHECK-NOT: @ch.pipe {{.*}] %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]
; IGNOREDEPTH-NOT: @ch.pipe {{.*}] %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]
; STRICT-NOT: @ch.pipe {{.*}] %opencl.pipe_rw_t {{.*}} !depth_is_ignored ![[MD:[0-9]+]]

; CHECK: define {{.*}}@__pipe_global_ctor
; IGNOREDEPTH: call void @__pipe_init_fpga({{.*}} @ch.pipe.bs, i32 4, i32 0, i32 2)
; DEFAULT: call void @__pipe_init_fpga({{.*}} @ch.pipe.bs, i32 4, i32 0, i32 1)
; STRICT: call void @__pipe_init_fpga({{.*}} @ch.pipe.bs, i32 4, i32 0, i32 0)

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

; DEFAULT: ![[MD]] = !{i1 true}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING: Missing line
