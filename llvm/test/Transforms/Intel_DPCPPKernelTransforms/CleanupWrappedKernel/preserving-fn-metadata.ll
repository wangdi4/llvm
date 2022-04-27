; RUN: opt -dpcpp-kernel-cleanup-wrapped %s -S  -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-cleanup-wrapped %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-cleanup-wrapped %s -S  -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-cleanup-wrapped %s -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-msvc-elf"

define void @__test_if_separated_args() #0 !kernel_wrapper !{void()* @test_if} {
  ret void
}

; CHECK: declare {{.*}} void @__test_if_separated_args() #0
; CHECK-NOT: define void @__test_if_separated_args()

define void @test_if() #1 {
  ret void
}
;; TODO: replace "kernel_wrapper" with "kernel-wrapper"
attributes #0 = { alwaysinline "kernel_wrapper"="test_if" }
attributes #1 = { noinline }

!sycl.kernels = !{!0}
!0 = !{void ()* @__test_if_separated_args}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
