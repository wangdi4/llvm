; Checks that all functions here are not kernels and they should be all removed.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,1
  ret void
}

define void @alsoNotKernel(i32 %input) nounwind {
entry:
  %x = add i32 1, %input
  ret void
}

define i32 @notKernel() nounwind {
entry:
  %res = add i32 1, 1
  ret i32 %res
}

define float @notKernel2(float %input) nounwind optnone noinline {
entry:
  %result = fmul float %input, 2.0
  ret float %result
}

; CHECK-NOT:    define void @thisIsNotKernel
; CHECK-NOT:    define void @alsoNotKernel
; CHECK-NOT:    define i32 @notKernel
; CHECK-NOT:    define float @notKernel2

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing line 5
; DEBUGIFY: WARNING: Missing line 6
; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY: WARNING: Missing line 8
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY: WARNING: Missing variable 3
; DEBUGIFY: WARNING: Missing variable 4
; DEBUGIFY-NOT: WARNING
