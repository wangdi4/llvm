; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(ptr addrspace(1) %out) noreturn !no_barrier_path !1 {
entry:
  %id = call i64 @_Z13get_global_idj(i32 0)
  %cmp = icmp slt i64 %id, 10
  br i1 %cmp, label %body, label %exit

body:
  br label %exit

exit:
  ret void
}

declare i64 @_Z13get_global_idj(i32)

; To make sure "WG.boundaries.test" don't have noreturn attribute.
; CHECK: define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %out) {

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
