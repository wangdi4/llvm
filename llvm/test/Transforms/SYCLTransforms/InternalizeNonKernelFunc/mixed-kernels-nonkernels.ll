; Checks that only non-kernel functions should be removed.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %x = add i32 1,1
  ret void
}

define void @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,2
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = add i32 1, 1
; CHECK-NOT:    define void @thisIsNotKernel()

; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY-NOT: WARNING
