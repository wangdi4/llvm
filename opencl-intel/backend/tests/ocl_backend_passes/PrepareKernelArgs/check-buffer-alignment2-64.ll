; RUN: opt -mtriple=x86_64 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK64
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{i8, i8, i8}>

; CHECK: @t1
define void @t1(%struct.struct1* %arg1, <2 x i8> %arg2, i32 %arg3, <8 x i16> %arg4, i8 %arg5, i32* %arg6 ) nounwind {
entry:
  ret void
}

;; new func - Win64
;;struct1 my_struct1 arg1 - struct is byVal- size:3
; CHECK64: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 0
; CHECK64-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1*

;; char2 arg2 - alignment: 2 - in pBuffer 0+3 = 3 is aligned to 4
; CHECK64-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 4
; CHECK64-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x i8>*
; CHECK64-NEXT: [[ARG1:%[a-zA-Z0-9]+]] = load <2 x i8>* [[ARG1_TYPECAST]], align 2

;; int arg3 - alignment: 4 - in pBuffer 4+2=6 is aligned to 8
; CHECK64-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 8
; CHECK64-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to i32*
; CHECK64-NEXT: [[ARG2:%[a-zA-Z0-9]+]] = load i32* [[ARG2_TYPECAST]], align 4

;; short8 arg4 - alignment: 16 - in pBuffer 8+4=12 is aligned to 16
; CHECK64-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 16
; CHECK64-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <8 x i16>*
; CHECK64-NEXT: [[ARG3:%[a-zA-Z0-9]+]] = load <8 x i16>* [[ARG3_TYPECAST]], align 16

;; char arg5 - alignment: 1 - in pBuffer 16+16=32 is aligned to 32
; CHECK64-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 32
; CHECK64-NEXT: [[ARG4:%[a-zA-Z0-9]+]] = load i8* [[ARG4_BUFF_INDEX]], align 1

;; int* arg6 - alignment: 8 - in pBuffer 32+1=33 is aligned to 40
; CHECK64-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 40
; CHECK64-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to i32**
; CHECK64-NEXT: [[ARG5:%[a-zA-Z0-9]+]] = load i32** [[ARG5_TYPECAST]], align 8
;;implicit args
;; call original func
; CHECK64: call void @__t1_separated_args(%struct.struct1* [[ARG0_TYPECAST]], <2 x i8> [[ARG1]], i32 [[ARG2]], <8 x i16> [[ARG3]], i8 [[ARG4]], i32* [[ARG5]], [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK64-NEXT: ret void


!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void (%struct.struct1*, <2 x i8>, i32, <8 x i16>, i8, i32*)* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"My_struct1", metadata !"char2", metadata !"int", metadata !"short8", metadata !"char", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arg1", metadata !"arg2", metadata !"arg3", metadata !"arg4", metadata !"arg5", metadata !"arg6"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
