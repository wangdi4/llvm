; RUN: opt -dpcpp-kernel-barrier-in-function %s -S -o - | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: define void @kernel
define void @kernel() #0 {
; CHECK: @__builtin_dpcpp_kernel_barrier_dummy
  tail call void @__builtin_dpcpp_kernel_barrier(i32 1)
  ret void
}

; CHECK-LABEL: define void @func
define void @func() {
; CHECK-NEXT: tail call void @kernel
; CHECK-NEXT: ret void
  tail call void @kernel()
  ret void
}

declare void @__builtin_dpcpp_kernel_barrier(i32 %0) #1

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { convergent }
