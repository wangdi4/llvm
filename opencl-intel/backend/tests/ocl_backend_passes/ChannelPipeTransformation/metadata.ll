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
; __kernel void foo() {
; }
; ----------------------------------------------------
; RUN: opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque

@ich = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@lch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8
@sch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 16

; CHECK-DAG: @pipe.ich.bs = {{.*}} global [32 x i8] {{.*}} align 4
; CHECK-DAG: @pipe.lch.bs = {{.*}} global [60 x i8] {{.*}} align 8
; CHECK-DAG: @pipe.sch.bs = {{.*}} global [44 x i8] {{.*}} align 16

; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.ich.bs{{.*}} i32 4, i32 1)
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.lch.bs{{.*}} i32 8, i32 3)
; CHECK-DAG: call {{.*}} @__pipe_init{{.*}} @pipe.sch.bs{{.*}} i32 16, i32 1)

; Function Attrs: norecurse nounwind readnone
define spir_kernel void @foo() #0 {
entry:
  ret void
}

attributes #0 = { norecurse nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !9, !13}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!17}
!opencl.spir.version = !{!17}
!opencl.used.extensions = !{!18}
!opencl.used.optional.core.features = !{!18}
!opencl.compiler.options = !{!18}
!llvm.ident = !{!19}

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
!17 = !{i32 2, i32 0}
!18 = !{}
!19 = !{!"clang version 3.8.1 "}
