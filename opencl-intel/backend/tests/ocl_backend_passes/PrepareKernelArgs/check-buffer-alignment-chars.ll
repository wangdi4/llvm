; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i8 %arg1, <2 x i8> %arg2, <2 x i8> %arg3, <3 x i8> %arg4, <16 x i8> %arg5, <4 x i8> %arg6, <8 x i8> %arg7 ) nounwind {
entry:
  ret void
}

;; new func
;;char arg1 - alignment: 1  (no bitcast from i8* to i8*)
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 0
; CHECK-NEXT: [[ARG0:%[a-zA-Z0-9]+]] = load i8* [[ARG0_BUFF_INDEX]], align 1

;; char2 arg2 - alignment: 2 - in pBuffer 0+1 = 1 is aligned to 2
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 2
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x i8>*
; CHECK-NEXT: [[ARG1:%[a-zA-Z0-9]+]] = load <2 x i8>* [[ARG1_TYPECAST]], align 2

;; char2 arg3 - alignment: 2 - in pBuffer 2+2 = 4 is aligned to 4
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 4
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <2 x i8>*
; CHECK-NEXT: [[ARG2:%[a-zA-Z0-9]+]] = load <2 x i8>* [[ARG2_TYPECAST]], align 2

;; char3 arg4 - alignment: 4 - in pBuffer 4+2=6 is aligned to 8
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 8
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <3 x i8>*
; CHECK-NEXT: [[ARG3:%[a-zA-Z0-9]+]] = load <3 x i8>* [[ARG3_TYPECAST]], align 4

;; char16 arg5 - alignment: 16 - in pBuffer 8+4=12 is aligned to 16
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 16
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to <16 x i8>*
; CHECK-NEXT: [[ARG4:%[a-zA-Z0-9]+]] = load <16 x i8>* [[ARG4_TYPECAST]], align 16

;; char4 arg6 - alignment: 4 - in pBuffer 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 32
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to <4 x i8>*
; CHECK-NEXT: [[ARG5:%[a-zA-Z0-9]+]] = load <4 x i8>* [[ARG5_TYPECAST]], align 4

;; char8 arg7 - alignment: 8 - in pBuffer 32+4=36 is aligned to 40
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 40
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to <8 x i8>*
; CHECK-NEXT: [[ARG6:%[a-zA-Z0-9]+]] = load <8 x i8>* [[ARG6_TYPECAST]], align 8

;;implicit args
;; call original func
; CHECK: call void @__t1_separated_args(i8 [[ARG0]], <2 x i8> [[ARG1]], <2 x i8> [[ARG2]], <3 x i8> [[ARG3]], <16 x i8> [[ARG4]], <4 x i8> [[ARG5]], <8 x i8> [[ARG6]], [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void (i8, <2 x i8>, <2 x i8>, <3 x i8>, <16 x i8>, <4 x i8>, <8 x i8>)* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"char", metadata !"char2", metadata !"char2", metadata !"char3", metadata !"char16", metadata !"char4", metadata !"char8"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arg1", metadata !"arg2", metadata !"arg3", metadata !"arg4", metadata !"arg5", metadata !"arg6", metadata !"arg7"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
