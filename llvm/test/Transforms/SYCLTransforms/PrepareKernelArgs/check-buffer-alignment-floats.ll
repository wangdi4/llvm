; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(float %arg1, <2 x float> %arg2, <2 x float> %arg3, <3 x float> %arg4, <16 x float> %arg5, <4 x float> %arg6, <8 x float> %arg7 ) {
entry:
  ret void
}

;; new func
;;float arg1 - alignment: 4
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = load float, ptr [[ARG0_BUFF_INDEX]], align 4

;; float2 arg2 - alignment: 8 - in UniformArgs 0+4 = 4 is aligned to 8
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT: %explicit_1 = load <2 x float>, ptr [[ARG1_BUFF_INDEX]], align 8

;; float2 arg3 - alignment: 8 - in UniformArgs 8+8=16 is aligned to 16
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK-NEXT: %explicit_2 = load <2 x float>, ptr [[ARG2_BUFF_INDEX]], align 8

;; float3 arg4 - alignment: 16 - in UniformArgs 16+8=24 is aligned to 32
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 32
; CHECK-NEXT: %explicit_3 = load <3 x float>, ptr [[ARG3_BUFF_INDEX]], align 16

;; float16 arg5 - alignment: 64 - in UniformArgs 32+16=48 is aligned to 64
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 64
; CHECK-NEXT: %explicit_4 = load <16 x float>, ptr [[ARG4_BUFF_INDEX]], align 64

;; float4 arg6 - alignment: 16 - in UniformArgs 64+64=128 is aligned to 128
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 128
; CHECK-NEXT: %explicit_5 = load <4 x float>, ptr [[ARG5_BUFF_INDEX]], align 16

;; float8 arg7 - alignment: 32 - in UniformArgs 128+16=144 is aligned to 160
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 160
; CHECK-NEXT: %explicit_6 = load <8 x float>, ptr [[ARG6_BUFF_INDEX]], align 32

;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
