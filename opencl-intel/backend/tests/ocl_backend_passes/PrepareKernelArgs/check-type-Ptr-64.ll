; RUN: opt -mtriple=x86_64 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK64
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i32* %arg1) nounwind {
entry:
  ret void
}

;; new func - Win64
;;int* arg1 - expected alignment: 8
; CHECK64: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 0
; CHECK64-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i32**
; CHECK64-NEXT: [[ARG0:%[a-zA-Z0-9]+]] = load i32** [[ARG0_TYPECAST]], align 8
;;implicit args
;; call original func
; CHECK64: call void @__t1_separated_args(i32* [[ARG0]], [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK64-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void (i32*)* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space",i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arg1"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}