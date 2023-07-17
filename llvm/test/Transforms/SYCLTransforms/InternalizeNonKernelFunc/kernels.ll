; Checks that all kernel functions should not be removed.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @thisIsKernel() nounwind {
entry:
  %x = add i32 1,1
  ret void
}

define void @alsoKernel(i32 %input) nounwind {
entry:
  %x = add i32 1, %input
  ret void
}

; CHECK: define void @thisIsKernel()
; CHECK: %x = add i32 1, 1
; CHECK: define void @alsoKernel(i32 %input)
; CHECK: %x = add i32 1, %input

!sycl.kernels = !{!0}
!0 = !{ptr @thisIsKernel, ptr @alsoKernel}

; DEBUGIFY-NOT: WARNING
