; RUN: opt -mtriple=x86_64 -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -mtriple=x86_64 -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i32* %arg1) {
entry:
  ret void
}

;; new func - Win64
;;int* arg1 - expected alignment: 8
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to i32**
; CHECK-NEXT: %explicit_0 = load i32*, i32** [[ARG0_TYPECAST]], align 8
;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{void (i32*)* @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-36: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
