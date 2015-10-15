; RUN: opt -mtriple=i686 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i16 %arg1, i8* %arg2, i8 %arg3, i32 %arg4, float %arg5, i64 %arg6, double %arg7 ) nounwind {
entry:
  ret void
}

;; new func - Win32
;;short arg1 - alignment: 2
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i16*
; CHECK-NEXT: %explicit_0 = load i16* [[ARG0_TYPECAST]], align 2

;;char* arg2 - alignment: 4 - in pUniformArgs 0+2 = 2 is aligned to 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 4
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to i8**
; CHECK-NEXT: %explicit_1 = load i8** [[ARG1_TYPECAST]], align 4

;;char arg3 - alignment: 1 - in pUniformArgs 4+4=8 is aligned to 8
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 8
; CHECK-NEXT: %explicit_2 = load i8* [[ARG2_BUFF_INDEX]], align 1

;;int arg4 - alignment: 4 - in pUniformArgs 8+1=9 is aligned to 12
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 12
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to i32*
; CHECK-NEXT: %explicit_3 = load i32* [[ARG3_TYPECAST]], align 4

;;float arg5 - alignment: 4 - in pUniformArgs 12+4=16 is aligned to 16
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 16
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to float*
; CHECK-NEXT: %explicit_4 = load float* [[ARG4_TYPECAST]], align 4

;;long arg6 - alignment: 8 - in pUniformArgs 16+4=20 is aligned to 24
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 24
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to i64*
; CHECK-NEXT: %explicit_5 = load i64* [[ARG5_TYPECAST]], align 8

;;double arg7 - alignment: 8 - in pUniformArgs 24+8=32 is aligned to 32
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 32
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to double*
; CHECK-NEXT: %explicit_6 = load double* [[ARG6_TYPECAST]], align 8
;;implicit args
;; call original func
; CHECK: call void @__t1_separated_args(i16 %explicit_0, i8* %explicit_1, i8 %explicit_2, i32 %explicit_3, float %explicit_4, i64 %explicit_5, double %explicit_6, [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK-NEXT: ret void


!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i16, i8*, i8, i32, float, i64, double)* @t1, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"short", !"char*", !"char", !"int", !"float", !"long", !"double"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !"", !""}
!5 = !{!"kernel_arg_name", !"arg1", !"arg2", !"arg3", !"arg4", !"arg5", !"arg6", !"arg7"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
