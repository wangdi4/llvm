; RUN: %oclopt -mtriple=x86_64 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK64
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i32* %arg1) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 {
entry:
  ret void
}

;; new func - Win64
;;int* arg1 - expected alignment: 8
; CHECK64: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 0
; CHECK64-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i32**
; CHECK64-NEXT: %explicit_0 = load i32*, i32** [[ARG0_TYPECAST]], align 8
;;implicit args
;; call original func
; CHECK64: call void @__t1_separated_args(i32* %explicit_0, [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK64-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i32*)* @t1}
!1 = !{i32 0}
!2 = !{!"none"}
!3 = !{!"int*"}
!4 = !{!""}
!5 = !{!"arg1"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
