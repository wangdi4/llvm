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
; channel long lch ;
; channel struct st sch;
;
; __kernel void foo() {
; }
; ----------------------------------------------------
; RUN: opt -runtimelib=../../Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque

@ich = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@lch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8
@sch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 16

; CHECK-DAG: @pipe.ich.bs = {{.*}} global [132 x i8] {{.*}} align 4
; CHECK-DAG: @pipe.lch.bs = {{.*}} global [136 x i8] {{.*}} align 8
; CHECK-DAG: @pipe.sch.bs = {{.*}} global [144 x i8] {{.*}} align 16

; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.ich.bs{{.*}} i32 4, i32 4)
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.lch.bs{{.*}} i32 8, i32 8)
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.sch.bs{{.*}} i32 16, i32 16)

; Function Attrs: norecurse nounwind readnone
define spir_kernel void @foo() #0 {
entry:
  ret void
}

attributes #0 = { norecurse nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !7, !8}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!9}
!opencl.spir.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!opencl.compiler.options = !{!10}
!llvm.ident = !{!11}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ich, i32 4, i32 4}
!7 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @lch, i32 8, i32 8}
!8 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @sch, i32 16, i32 16}
!9 = !{i32 2, i32 0}
!10 = !{}
!11 = !{!"clang version 3.8.1 "}
