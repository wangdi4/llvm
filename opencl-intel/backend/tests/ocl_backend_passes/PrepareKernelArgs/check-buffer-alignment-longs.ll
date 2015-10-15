; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i64 %arg1, <2 x i64> %arg2, <2 x i64> %arg3, <3 x i64> %arg4, <16 x i64> %arg5, <4 x i64> %arg6, <8 x i64> %arg7 ) nounwind {
entry:
  ret void
}

;; new func
;;long arg1 - alignment: 8
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i64*
; CHECK-NEXT: %explicit_0 = load i64* [[ARG0_TYPECAST]], align 8

;; long2 arg2 - alignment: 16 - in pUniformArgs 0+8 = 8 is aligned to 16
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 16
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x i64>*
; CHECK-NEXT: %explicit_1 = load <2 x i64>* [[ARG1_TYPECAST]], align 16

;; long2 arg3 - alignment: 16 - in pUniformArgs 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 32
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <2 x i64>*
; CHECK-NEXT: %explicit_2 = load <2 x i64>* [[ARG2_TYPECAST]], align 16

;; long3 arg4 - alignment: 32 - in pUniformArgs 32+16=48 is aligned to 64
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 64
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <3 x i64>*
; CHECK-NEXT: %explicit_3 = load <3 x i64>* [[ARG3_TYPECAST]], align 32

;; long16 arg5 - alignment: 128 - in pUniformArgs 64+32=96 is aligned to 128
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 128
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to <16 x i64>*
; CHECK-NEXT: %explicit_4 = load <16 x i64>* [[ARG4_TYPECAST]], align 128

;; long4 arg6 - alignment: 32 - in pUniformArgs 128+128=256 is aligned to 256
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 256
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to <4 x i64>*
; CHECK-NEXT: %explicit_5 = load <4 x i64>* [[ARG5_TYPECAST]], align 32

;; long8 arg7 - alignment: 64 - in pUniformArgs 256+32=288 is aligned to 320
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 320
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to <8 x i64>*
; CHECK-NEXT: %explicit_6 = load <8 x i64>* [[ARG6_TYPECAST]], align 64

;;implicit args
;; call original func
; CHECK: call void @__t1_separated_args(i64 %explicit_0, <2 x i64> %explicit_1, <2 x i64> %explicit_2, <3 x i64> %explicit_3, <16 x i64> %explicit_4, <4 x i64> %explicit_5, <8 x i64> %explicit_6, [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i64, <2 x i64>, <2 x i64>, <3 x i64>, <16 x i64>, <4 x i64>, <8 x i64>)* @t1, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"long", !"long2", !"long2", !"long3", !"long16", !"long4", !"long8"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !"", !""}
!5 = !{!"kernel_arg_name", !"arg1", !"arg2", !"arg3", !"arg4", !"arg5", !"arg6", !"arg7"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
