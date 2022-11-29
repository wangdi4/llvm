; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S < %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(double %arg1, <2 x double> %arg2, <2 x double> %arg3, <3 x double> %arg4, <16 x double> %arg5, <4 x double> %arg6, <8 x double> %arg7 ) {
entry:
  ret void
}

;; new func
;;double arg1 - alignment: 8
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to double*
; CHECK-NEXT: %explicit_0 = load double, double* [[ARG0_TYPECAST]], align 8

;; double2 arg2 - alignment: 16 - in UniformArgs 0+8 = 8 is aligned to 16
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x double>*
; CHECK-NEXT: %explicit_1 = load <2 x double>, <2 x double>* [[ARG1_TYPECAST]], align 16

;; double2 arg3 - alignment: 16 - in UniformArgs 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 32
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <2 x double>*
; CHECK-NEXT: %explicit_2 = load <2 x double>, <2 x double>* [[ARG2_TYPECAST]], align 16

;; double3 arg4 - alignment: 32 - in UniformArgs 32+16=48 is aligned to 64
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 64
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <3 x double>*
; CHECK-NEXT: %explicit_3 = load <3 x double>, <3 x double>* [[ARG3_TYPECAST]], align 32

;; double16 arg5 - alignment: 128 - in UniformArgs 64+32=96 is aligned to 128
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 128
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to <16 x double>*
; CHECK-NEXT: %explicit_4 = load <16 x double>, <16 x double>* [[ARG4_TYPECAST]], align 128

;; double4 arg6 - alignment: 32 - in UniformArgs 128+128=256 is aligned to 256
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 256
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to <4 x double>*
; CHECK-NEXT: %explicit_5 = load <4 x double>, <4 x double>* [[ARG5_TYPECAST]], align 32

;; double8 arg7 - alignment: 64 - in UniformArgs 256+32=288 is aligned to 320
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 320
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to <8 x double>*
; CHECK-NEXT: %explicit_6 = load <8 x double>, <8 x double>* [[ARG6_TYPECAST]], align 64

;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{void (double, <2 x double>, <2 x double>, <3 x double>, <16 x double>, <4 x double>, <8 x double>)* @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-54: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
