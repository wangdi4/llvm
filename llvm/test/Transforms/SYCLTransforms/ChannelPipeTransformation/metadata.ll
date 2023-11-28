; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; struct  __attribute__((packed, aligned(16))) st {
;   int i1;
;   int i2;
;   long l;
; };
;
; channel int ich;
; channel long lch __attribute__((depth(3)));
; channel struct st sch __attribute__((depth(0)));
;
; channel int ich_arr[5];
; channel long lch_arr[5][4] __attribute__((depth(3)));
; channel struct st sch_arr[5][4][3] __attribute__((depth(0)));
;
; __kernel void foo() {
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ich = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@lch = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !1, !packet_align !1, !depth !2
@sch = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !3, !packet_align !3, !depth !4
@ich_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0
@lch_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 8, !packet_size !1, !packet_align !1, !depth !2
@sch_arr = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 8, !packet_size !3, !packet_align !3, !depth !4

; CHECK-DAG: @ich.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 4
; CHECK-DAG: @lch.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 8
; CHECK-DAG: @sch.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 16
; CHECK-DAG: @sch_arr.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 16
; CHECK-DAG: @ich_arr.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 4
; CHECK-DAG: @lch_arr.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 8

; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @ich.bs, i32 4, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @lch.bs, i32 8, i32 3
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @sch.bs, i32 16, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}} @ich_arr{{.*}} i32 5, i32 4, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}} @sch_arr{{.*}} i32 60, i32 16, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}} @lch_arr{{.*}} i32 20, i32 8, i32 3

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !6 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !6 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !arg_type_null_val !6 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }

!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!sycl.kernels = !{!8}

!0 = !{i32 4}
!1 = !{i32 8}
!2 = !{i32 3}
!3 = !{i32 16}
!4 = !{i32 0}
!5 = !{i32 2, i32 0}
!6 = !{}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!8 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor
; DEBUGIFY-NOT: WARNING
