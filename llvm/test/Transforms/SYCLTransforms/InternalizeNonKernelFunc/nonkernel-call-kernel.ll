; Checks that all non-kernels should be removed.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define i32 @thisIsNotKernel() nounwind {
entry:
  call void @anotherNotKernel()
  %w = add i32 1, 1
  ret i32 %w
}

define void @anotherNotKernel() nounwind {
entry:
  call void @thisIsKernel()
  ret void
}

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %z = add i32 1,1
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @thisIsKernel}

; CHECK-NOT:    define{{.*}}void @thisIsNotKernel()
; CHECK-NOT:    define{{.*}}void @anotherNotKernel()
; CHECK:        define void @thisIsKernel()

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing line 5
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY-NOT: WARNING
