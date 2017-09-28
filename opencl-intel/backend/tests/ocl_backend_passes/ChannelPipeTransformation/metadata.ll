; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
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
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque

@ich = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@lch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8
@sch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 16
@ich_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@lch_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 8
@sch_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 16

; CHECK-DAG: @pipe.ich.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 4
; CHECK-DAG: @pipe.lch.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 8
; CHECK-DAG: @pipe.sch.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 16
; CHECK-DAG: @pipe.sch_arr.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 16
; CHECK-DAG: @pipe.ich_arr.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 4
; CHECK-DAG: @pipe.lch_arr.bs = {{.*}} global [{{[0-9]+}} x i8] {{.*}} align 8

; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.ich.bs{{.*}} i32 4, i32 1)
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.lch.bs{{.*}} i32 8, i32 3)
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.sch.bs{{.*}} i32 16, i32 1)
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}}({{.*}} @pipe.ich_arr{{.*}} i32 5, i32 4, i32 1)
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}}({{.*}} @pipe.sch_arr{{.*}} i32 60, i32 16, i32 1)
; CHECK-DAG: call {{.*}} @__pipe_init_array{{.*}}({{.*}} @pipe.lch_arr{{.*}} i32 20, i32 8, i32 3)

; Function Attrs: norecurse nounwind readnone
define spir_kernel void @foo() #0 {
entry:
  ret void
}

attributes #0 = { norecurse nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !9, !13, !17, !18, !19}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!20}
!opencl.spir.version = !{!20}
!opencl.used.extensions = !{!21}
!opencl.used.optional.core.features = !{!21}
!opencl.compiler.options = !{!21}
!llvm.ident = !{!22}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ich, !7, !8}
!7 = !{!"packet_size", i32 4}
!8 = !{!"packet_align", i32 4}
!9 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @lch, !10, !11, !12}
!10 = !{!"packet_size", i32 8}
!11 = !{!"packet_align", i32 8}
!12 = !{!"depth", i32 3}
!13 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @sch, !14, !15, !16}
!14 = !{!"packet_size", i32 16}
!15 = !{!"packet_align", i32 16}
!16 = !{!"depth", i32 0}
!17 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ich_arr, !7, !8}
!18 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @lch_arr, !10, !11, !12}
!19 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @sch_arr, !14, !15, !16}
!20 = !{i32 2, i32 0}
!21 = !{}
!22 = !{!"clang version 3.8.1 "}
