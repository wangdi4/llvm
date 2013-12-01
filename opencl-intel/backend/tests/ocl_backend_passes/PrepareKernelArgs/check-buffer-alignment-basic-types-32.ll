; RUN: opt -mtriple=i686 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK32
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i16 %arg1, i8* %arg2, i8 %arg3, i32 %arg4, float %arg5, i64 %arg6, double %arg7 ) nounwind {
entry:
  ret void
}

;; new func - Win32
;;short arg1 - alignment: 2
; CHECK32: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 0
; CHECK32-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i16*
; CHECK32-NEXT: [[ARG0:%[a-zA-Z0-9]+]] = load i16* [[ARG0_TYPECAST]], align 2

;;char* arg2 - alignment: 4 - in pBuffer 0+2 = 2 is aligned to 4
; CHECK32-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 4
; CHECK32-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to i8**
; CHECK32-NEXT: [[ARG1:%[a-zA-Z0-9]+]] = load i8** [[ARG1_TYPECAST]], align 4

;;char arg3 - alignment: 1 - in pBuffer 4+4=8 is aligned to 8
; CHECK32-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 8
; CHECK32-NEXT: [[ARG2:%[a-zA-Z0-9]+]] = load i8* [[ARG2_BUFF_INDEX]], align 1

;;int arg4 - alignment: 4 - in pBuffer 8+1=9 is aligned to 12
; CHECK32-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 12
; CHECK32-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to i32*
; CHECK32-NEXT: [[ARG3:%[a-zA-Z0-9]+]] = load i32* [[ARG3_TYPECAST]], align 4

;;float arg5 - alignment: 4 - in pBuffer 12+4=16 is aligned to 16
; CHECK32-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 16
; CHECK32-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to float*
; CHECK32-NEXT: [[ARG4:%[a-zA-Z0-9]+]] = load float* [[ARG4_TYPECAST]], align 4

;;long arg6 - alignment: 8 - in pBuffer 16+4=20 is aligned to 24
; CHECK32-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 24
; CHECK32-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to i64*
; CHECK32-NEXT: [[ARG5:%[a-zA-Z0-9]+]] = load i64* [[ARG5_TYPECAST]], align 8

;;double arg7 - alignment: 8 - in pBuffer 24+8=32 is aligned to 32
; CHECK32-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 32
; CHECK32-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to double*
; CHECK32-NEXT: [[ARG6:%[a-zA-Z0-9]+]] = load double* [[ARG6_TYPECAST]], align 8
;;implicit args
;; call original func
; CHECK32: call void @__t1_separated_args(i16 [[ARG0]], i8* [[ARG1]], i8 [[ARG2]], i32 [[ARG3]], float [[ARG4]], i64 [[ARG5]], double [[ARG6]], [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK32-NEXT: ret void


!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void (i16, i8*, i8, i32, float, i64, double)* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"short", metadata !"char*", metadata !"char", metadata !"int", metadata !"float", metadata !"long", metadata !"double"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arg1", metadata !"arg2", metadata !"arg3", metadata !"arg4", metadata !"arg5", metadata !"arg6", metadata !"arg7"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
