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
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ich = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@lch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8, !packet_size !1, !packet_align !1, !depth !2
@sch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 16, !packet_size !3, !packet_align !3, !depth !4
@ich_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@lch_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 8, !packet_size !1, !packet_align !1, !depth !2
@sch_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 16, !packet_size !3, !packet_align !3, !depth !4

; CHECK-DAG: @ich.pipe.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 4
; CHECK-DAG: @lch.pipe.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 8
; CHECK-DAG: @sch.pipe.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 16
; CHECK-DAG: @sch_arr.pipe.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 16
; CHECK-DAG: @ich_arr.pipe.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 4
; CHECK-DAG: @lch_arr.pipe.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 8

; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @ich.pipe.bs {{.*}} i32 4, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @lch.pipe.bs {{.*}} i32 8, i32 3
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @sch.pipe.bs {{.*}} i32 16, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}}({{.*}} @ich_arr.pipe {{.*}} i32 5, i32 4, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}}({{.*}} @sch_arr.pipe {{.*}} i32 60, i32 16, i32 0
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}}({{.*}} @lch_arr.pipe {{.*}} i32 20, i32 8, i32 3

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !16 !kernel_arg_access_qual !16 !kernel_arg_type !16 !kernel_arg_base_type !16 !kernel_arg_type_qual !16 {
entry:
  ret void
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!14}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!15}
!opencl.spir.version = !{!15}
!opencl.used.extensions = !{!16}
!opencl.used.optional.core.features = !{!16}
!opencl.compiler.options = !{!16}
!llvm.ident = !{!17}

!0 = !{i32 4}
!1 = !{i32 8}
!2 = !{i32 3}
!3 = !{i32 16}
!4 = !{i32 0}
!14 = !{i32 1, !"wchar_size", i32 4}
!15 = !{i32 2, i32 0}
!16 = !{}
!17 = !{!"clang version 5.0.0 "}
