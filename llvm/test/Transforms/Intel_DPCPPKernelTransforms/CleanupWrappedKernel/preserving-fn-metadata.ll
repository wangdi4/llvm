; RUN: opt -dpcpp-kernel-cleanup-wrapped %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-cleanup-wrapped %s -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-msvc-elf"

define void @__test_if_separated_args() #0 {
  ret void
}

; CHECK: declare void @__test_if_separated_args() #0
; CHECK-NOT: define void @__test_if_separated_args()

define void @test_if() #1 {
  ret void
}
;; TODO: replace "kernel_wrapper" with "kernel-wrapper"
attributes #0 = { alwaysinline "kernel_wrapper"="test_if" "sycl-kernel"}
attributes #1 = { noinline }
