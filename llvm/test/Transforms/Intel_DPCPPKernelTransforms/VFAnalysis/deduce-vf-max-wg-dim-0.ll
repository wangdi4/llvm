; RUN: opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; This test checks that vf is set to 1 if max_wg_dimensions is 0 and
; no_barrier_path is true.

; CHECK: Kernel --> VF:
; CHECK-DAG:  <test1> : 1
; CHECK-DAG:  <test2> : 4
; CHECK: Kernel --> SGEmuSize:
; CHECK-DAG:  <test1> : 0
; CHECK-DAG:  <test2> : 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test1(i32 addrspace(1)* %dst) !no_barrier_path !1 !max_wg_dimensions !2 {
entry:
  store i32 0, i32 addrspace(1)* %dst, align 4
  ret void
}

define void @test2(i32 addrspace(1)* %dst) !no_barrier_path !3 !max_wg_dimensions !2 {
entry:
  store i32 0, i32 addrspace(1)* %dst, align 4
  tail call void @_Z7barrierj(i32 noundef 1) #0
  ret void
}

; Function Attrs: convergent
declare void @_Z7barrierj(i32 noundef) #0

attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test1, void (i32 addrspace(1)*)* @test2}
!1 = !{i1 true}
!2 = !{i32 0}
!3 = !{i1 false}
