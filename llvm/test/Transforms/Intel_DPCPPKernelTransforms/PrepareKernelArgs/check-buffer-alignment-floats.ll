; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(float %arg1, <2 x float> %arg2, <2 x float> %arg3, <3 x float> %arg4, <16 x float> %arg5, <4 x float> %arg6, <8 x float> %arg7 ) {
entry:
  ret void
}

;; new func
;;float arg1 - alignment: 4
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to float*
; CHECK-NEXT: %explicit_0 = load float, float* [[ARG0_TYPECAST]], align 4

;; float2 arg2 - alignment: 8 - in UniformArgs 0+4 = 4 is aligned to 8
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 8
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x float>*
; CHECK-NEXT: %explicit_1 = load <2 x float>, <2 x float>* [[ARG1_TYPECAST]], align 8

;; float2 arg3 - alignment: 8 - in UniformArgs 8+8=16 is aligned to 16
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <2 x float>*
; CHECK-NEXT: %explicit_2 = load <2 x float>, <2 x float>* [[ARG2_TYPECAST]], align 8

;; float3 arg4 - alignment: 16 - in UniformArgs 16+8=24 is aligned to 32
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 32
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <3 x float>*
; CHECK-NEXT: %explicit_3 = load <3 x float>, <3 x float>* [[ARG3_TYPECAST]], align 16

;; float16 arg5 - alignment: 64 - in UniformArgs 32+16=48 is aligned to 64
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 64
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to <16 x float>*
; CHECK-NEXT: %explicit_4 = load <16 x float>, <16 x float>* [[ARG4_TYPECAST]], align 64

;; float4 arg6 - alignment: 16 - in UniformArgs 64+64=128 is aligned to 128
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 128
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to <4 x float>*
; CHECK-NEXT: %explicit_5 = load <4 x float>, <4 x float>* [[ARG5_TYPECAST]], align 16

;; float8 arg7 - alignment: 32 - in UniformArgs 128+16=144 is aligned to 160
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 160
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to <8 x float>*
; CHECK-NEXT: %explicit_6 = load <8 x float>, <8 x float>* [[ARG6_TYPECAST]], align 32

;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{void (float, <2 x float>, <2 x float>, <3 x float>, <16 x float>, <4 x float>, <8 x float>)* @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-54: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
