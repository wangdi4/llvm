; RUN: opt -mtriple=i686 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{i8, i8, i8}>

; CHECK: @t1
define void @t1(%struct.struct1* %arg1, <2 x i8> %arg2, i32 %arg3, <8 x i16> %arg4, i8 %arg5, i32* %arg6 ) nounwind {
entry:
  ret void
}

;; new func - Win32
;;struct1 my_struct1 arg1 - struct is byVal- size:3
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1*

;; char2 arg2 - alignment: 2 - in pUniformArgs 0+3 = 3 is aligned to 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 4
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x i8>*
; CHECK-NEXT: %explicit_1 = load <2 x i8>* [[ARG1_TYPECAST]], align 2

;; int arg3 - alignment: 4 - in pUniformArgs 4+2=6 is aligned to 8
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 8
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to i32*
; CHECK-NEXT: %explicit_2 = load i32* [[ARG2_TYPECAST]], align 4

;; short8 arg4 - alignment: 16 - in pUniformArgs 8+4=12 is aligned to 16
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 16
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <8 x i16>*
; CHECK-NEXT: %explicit_3 = load <8 x i16>* [[ARG3_TYPECAST]], align 16

;; char arg5 - alignment: 1 - in pUniformArgs 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 32
; CHECK-NEXT: %explicit_4 = load i8* [[ARG4_BUFF_INDEX]], align 1

;; int* arg6 - alignment: 4 - in pUniformArgs 32+1=33 is aligned to 36
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 36
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to i32**
; CHECK-NEXT: %explicit_5 = load i32** [[ARG5_TYPECAST]], align 4
;;implicit args
;; call original func
; CHECK: call void @__t1_separated_args(%struct.struct1* %explicit_0, <2 x i8> %explicit_1, i32 %explicit_2, <8 x i16> %explicit_3, i8 %explicit_4, i32* %explicit_5, [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK-NEXT: ret void


!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (%struct.struct1*, <2 x i8>, i32, <8 x i16>, i8, i32*)* @t1, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"My_struct1", !"char2", !"int", !"short8", !"char", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !""}
!5 = !{!"kernel_arg_name", !"arg1", !"arg2", !"arg3", !"arg4", !"arg5", !"arg6"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
