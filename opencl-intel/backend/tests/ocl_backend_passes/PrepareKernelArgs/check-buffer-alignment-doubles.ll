; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -S < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(double %arg1, <2 x double> %arg2, <2 x double> %arg3, <3 x double> %arg4, <16 x double> %arg5, <4 x double> %arg6, <8 x double> %arg7 ) nounwind {
entry:
  ret void
}

;; new func
;;double arg1 - alignment: 8
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to double*
; CHECK-NEXT: %explicit_0 = load double* [[ARG0_TYPECAST]], align 8

;; double2 arg2 - alignment: 16 - in pUniformArgs 0+8 = 8 is aligned to 16
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 16
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x double>*
; CHECK-NEXT: %explicit_1 = load <2 x double>* [[ARG1_TYPECAST]], align 16

;; double2 arg3 - alignment: 16 - in pUniformArgs 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 32
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <2 x double>*
; CHECK-NEXT: %explicit_2 = load <2 x double>* [[ARG2_TYPECAST]], align 16

;; double3 arg4 - alignment: 32 - in pUniformArgs 32+16=48 is aligned to 64
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 64
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <3 x double>*
; CHECK-NEXT: %explicit_3 = load <3 x double>* [[ARG3_TYPECAST]], align 32

;; double16 arg5 - alignment: 128 - in pUniformArgs 64+32=96 is aligned to 128
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 128
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to <16 x double>*
; CHECK-NEXT: %explicit_4 = load <16 x double>* [[ARG4_TYPECAST]], align 128

;; double4 arg6 - alignment: 32 - in pUniformArgs 128+128=256 is aligned to 256
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 256
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to <4 x double>*
; CHECK-NEXT: %explicit_5 = load <4 x double>* [[ARG5_TYPECAST]], align 32

;; double8 arg7 - alignment: 64 - in pUniformArgs 256+32=288 is aligned to 320
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 320
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to <8 x double>*
; CHECK-NEXT: %explicit_6 = load <8 x double>* [[ARG6_TYPECAST]], align 64

;;implicit args
;; call original func
; CHECK: call void @__t1_separated_args(double %explicit_0, <2 x double> %explicit_1, <2 x double> %explicit_2, <3 x double> %explicit_3, <16 x double> %explicit_4, <4 x double> %explicit_5, <8 x double> %explicit_6, [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (double, <2 x double>, <2 x double>, <3 x double>, <16 x double>, <4 x double>, <8 x double>)* @t1, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"double", !"double2", !"double2", !"double3", !"double16", !"double4", !"double8"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !"", !""}
!5 = !{!"kernel_arg_name", !"arg1", !"arg2", !"arg3", !"arg4", !"arg5", !"arg6", !"arg7"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
