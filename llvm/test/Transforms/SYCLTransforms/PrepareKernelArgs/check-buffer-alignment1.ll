; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i8 %arg1, <2 x i8> %arg2, double %arg3, <8 x float> %arg4, <16 x i64> %arg5 ) {
entry:
  ret void
}

;; new func
;;char arg1 - alignment: 1  (no bitcast from i8* to i8*)
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = load i8, ptr [[ARG0_BUFF_INDEX]], align 1

;; char2 arg2 - alignment: 2 - in UniformArgs 0+1 = 1 is aligned to 2
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 2
; CHECK-NEXT: %explicit_1 = load <2 x i8>, ptr [[ARG1_BUFF_INDEX]], align 2

;; double arg3 - alignment: 8 - in UniformArgs 2+2=4 is aligned to 8
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT: %explicit_2 = load double, ptr [[ARG2_BUFF_INDEX]], align 8

;; float8 arg4 - alignment: 32 - in UniformArgs 8+8=16 is aligned to 32
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 32
; CHECK-NEXT: %explicit_3 = load <8 x float>, ptr [[ARG3_BUFF_INDEX]], align 32

;; long16 arg5 - alignment: 128 - in UniformArgs 32+32=64 is aligned to 128
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 128
; CHECK-NEXT: %explicit_4 = load <16 x i64>, ptr [[ARG4_BUFF_INDEX]], align 128
;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-38: WARNING: Instruction with empty DebugLoc in function t1 {{.*}}
; DEBUGIFY-NOT: WARNING
