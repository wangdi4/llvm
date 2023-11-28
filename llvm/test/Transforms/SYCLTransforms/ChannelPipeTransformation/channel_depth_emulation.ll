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

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=2 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=2 %s -S | FileCheck %s --check-prefixes=CHECK,IGNOREDEPTH
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=1 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=1 %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=0 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -sycl-channel-depth-emulation-mode=0 %s -S | FileCheck %s --check-prefixes=CHECK,STRICT

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0

; DEFAULT: warning: The default channel depths in the emulation flow will be different from the hardware flow depth (0) to speed up emulation. The following channels are affected:
; DEFAULT-NEXT: - ch

; DEFAULT: @ch = addrspace(1) global ptr addrspace(1) null, {{.*}}
; CHECK-NOT: @ch {{.*}] %opencl.pipe_rw_t {{.*}}
; IGNOREDEPTH-NOT: @ch {{.*}] %opencl.pipe_rw_t {{.*}}
; STRICT-NOT: @ch {{.*}] %opencl.pipe_rw_t {{.*}}

; CHECK: define {{.*}}@__pipe_global_ctor
; IGNOREDEPTH: call void @__pipe_init_fpga({{.*}} @ch.bs, i32 4, i32 0, i32 2)
; DEFAULT: call void @__pipe_init_fpga({{.*}} @ch.bs, i32 4, i32 0, i32 1)
; STRICT: call void @__pipe_init_fpga({{.*}} @ch.bs, i32 4, i32 0, i32 0)

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

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING: Missing line
