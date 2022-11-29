; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i16 %arg1, <2 x i16> %arg2, <2 x i16> %arg3, <3 x i16> %arg4, <16 x i16> %arg5, <4 x i16> %arg6, <8 x i16> %arg7 ) {
entry:
  ret void
}

;; new func
;;short arg1 - alignment: 2
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i16*
; CHECK-NEXT: %explicit_0 = load i16, i16* [[ARG0_TYPECAST]], align 2

;; short2 arg2 - alignment: 4 - in UniformArgs 0+2 = 2 is aligned to 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 4
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x i16>*
; CHECK-NEXT: %explicit_1 = load <2 x i16>, <2 x i16>* [[ARG1_TYPECAST]], align 4

;; short2 arg3 - alignment: 4 - in UniformArgs 4+4=8 is aligned to 8
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 8
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <2 x i16>*
; CHECK-NEXT: %explicit_2 = load <2 x i16>, <2 x i16>* [[ARG2_TYPECAST]], align 4

;; short3 arg4 - alignment: 8 - in UniformArgs 8+4=12 is aligned to 16
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <3 x i16>*
; CHECK-NEXT: %explicit_3 = load <3 x i16>, <3 x i16>* [[ARG3_TYPECAST]], align 8

;; short16 arg5 - alignment: 32 - in UniformArgs 16+8=24 is aligned to 32
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 32
; CHECK-NEXT: [[ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG4_BUFF_INDEX]] to <16 x i16>*
; CHECK-NEXT: %explicit_4 = load <16 x i16>, <16 x i16>* [[ARG4_TYPECAST]], align 32

;; short4 arg6 - alignment: 8 - in UniformArgs 32+32=64 is aligned to 64
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 64
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to <4 x i16>*
; CHECK-NEXT: %explicit_5 = load <4 x i16>, <4 x i16>* [[ARG5_TYPECAST]], align 8

;; short8 arg7 - alignment: 16 - in UniformArgs 64+8=72 is aligned to 80
; CHECK-NEXT: [[ARG6_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 80
; CHECK-NEXT: [[ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG6_BUFF_INDEX]] to <8 x i16>*
; CHECK-NEXT: %explicit_6 = load <8 x i16>, <8 x i16>* [[ARG6_TYPECAST]], align 16

;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{void (i16, <2 x i16>, <2 x i16>, <3 x i16>, <16 x i16>, <4 x i16>, <8 x i16>)* @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-54: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
