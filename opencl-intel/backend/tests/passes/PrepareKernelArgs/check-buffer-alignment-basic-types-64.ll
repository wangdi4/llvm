; RUN: %oclopt -mtriple=x86_64 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -mtriple=x86_64 -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll -check-prefix=CHECK64
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i16 %arg1, i8* %arg2, i8 %arg3, i32 %arg4, float %arg5, i64 %arg6, double %arg7 ) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 {
entry:
  ret void
}

;; new func - Win64
;;short arg1 - alignment: 2
; CHECK64: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 0
; CHECK64-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i16*
; CHECK64-NEXT: %explicit_0 = load i16, i16* [[ARG0_TYPECAST]], align 2

;;char* arg2 - alignment: 8 - in pUniformArgs 0+2 = 2 is aligned to 8
; CHECK64-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 8
; CHECK64-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to i8**
; CHECK64-NEXT: %explicit_1 = load i8*, i8** [[ARG1_TYPECAST]], align 8

;;char arg3 - alignment: 1 - in pUniformArgs 8+8=16 is aligned to 16
; CHECK64-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 16
; CHECK64-NEXT: %explicit_2 = load i8, i8* [[ARG2_BUFF_INDEX]], align 1

;;int arg4 - alignment: 4 - in pUniformArgs 16+1=17 is aligned to 20
; CHECK64-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 20
; CHECK64-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to i32*
; CHECK64-NEXT: %explicit_3 = load i32, i32* [[ARG3_TYPECAST]], align 4

;;float arg5 - alignment: 4 - in pUniformArgs 20+4=24 is aligned to 24
; CHECK64-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 24
; CHECK64-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to float*
; CHECK64-NEXT: %explicit_4 = load float, float* [[ARG4_TYPECAST]], align 4

;;long arg6 - alignment: 8 - in pUniformArgs 24+4=28 is aligned to 32
; CHECK64-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 32
; CHECK64-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to i64*
; CHECK64-NEXT: %explicit_5 = load i64, i64* [[ARG5_TYPECAST]], align 8

;;double arg7 - alignment: 8 - in pUniformArgs 32+8=40 is aligned to 40
; CHECK64-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %pUniformArgs, i32 40
; CHECK64-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to double*
; CHECK64-NEXT: %explicit_6 = load double, double* [[ARG6_TYPECAST]], align 8
;;implicit args
;; call original func
; CHECK64: call void @__t1_separated_args(i16 %explicit_0, i8* %explicit_1, i8 %explicit_2, i32 %explicit_3, float %explicit_4, i64 %explicit_5, double %explicit_6, [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK64-NEXT: ret void

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i16, i8*, i8, i32, float, i64, double)* @t1}
!1 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"short", !"char*", !"char", !"int", !"float", !"long", !"double"}
!4 = !{!"", !"", !"", !"", !"", !"", !""}
!5 = !{!"arg1", !"arg2", !"arg3", !"arg4", !"arg5", !"arg6", !"arg7"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-54: WARNING: Instruction with empty DebugLoc in function t1 {{.*}}
; DEBUGIFY-NOT: WARNING
