; Checks that no functions should be removed.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %x = call i32 @thisIsNotKernel()
  ret void
}

define i32 @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,1
  ret i32 %x
}

!sycl.kernels = !{!0}

!0 = !{ptr @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = call i32 @thisIsNotKernel()
; CHECK:        define internal i32 @thisIsNotKernel()
; CHECK:        %x = add i32 1, 1



; DEBUGIFY-NOT: WARNING
