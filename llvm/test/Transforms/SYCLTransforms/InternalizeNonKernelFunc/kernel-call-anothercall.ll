; Checks that only function nonKernel3 should be removed.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %x = call i32 @nonKernel1()
  ret void
}

define i32 @nonKernel1() nounwind {
entry:
  %y = call i32 @nonKernel2()
  ret i32 %y
}

define i32 @nonKernel2() nounwind {
entry:
  %z = add i32 1, 1
  ret i32 %z
}

define i32 @nonKernel3() nounwind {
entry:
  %w = call i32 @nonKernel1()
  ret i32 %w
}

!sycl.kernels = !{!0}

!0 = !{ptr @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = call i32 @nonKernel1()
; CHECK:        define internal i32 @nonKernel1()
; CHECK:        %y = call i32 @nonKernel2()
; CHECK:        define internal i32 @nonKernel2()
; CHECK:        %z = add i32 1, 1
; CHECK-NOT:    define{{.*}}i32 @nonKernel3()

; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY: WARNING: Missing line 8
; DEBUGIFY: WARNING: Missing variable 4
; DEBUGIFY-NOT: WARNING
