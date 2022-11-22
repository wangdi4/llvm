; RUN: opt -passes='print<dpcpp-kernel-local-buffer-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check local buffer size and offset of local variables in not-inlined case
; with two kernels.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: LocalBufferInfo
; CHECK:   Local variables used in kernel
; CHECK:     test1
; CHECK-DAG:   test1.i
; CHECK-DAG:   test1.j
; CHECK:     test2
; CHECK-DAG:   test2.s
; CHECK-DAG:   test2.t
; CHECK:   Kernel local buffer size
; CHECK-DAG: test1 : 404
; CHECK-DAG: test2 : 88
; CHECK:   Offset of local variable in containing kernel's local buffer
; CHECK-DAG:   test2.s : 0
; CHECK-DAG:   test1.j : 400
; CHECK-DAG:   test1.i : 0
; CHECK-DAG:   test2.t : 80

@test1.i = internal addrspace(3) global [100 x i32] undef, align 4
@test1.j = internal addrspace(3) global i32 undef, align 4
@test2.s = internal addrspace(3) global [10 x double] undef, align 8
@test2.t = internal addrspace(3) global double undef, align 8

define internal fastcc zeroext i1 @foo() {
entry:
  %0 = load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test1.i, i64 0, i64 0), align 4, !tbaa !1
  %1 = load i32, i32 addrspace(3)* @test1.j, align 4
  ret i1 false
}

define internal fastcc zeroext i1 @bar() {
entry:
  %0 = load double, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test2.s, i64 0, i64 0), align 8, !tbaa !5
  %1 = load double, double addrspace(3)* @test2.t, align 8
  ret i1 false
}

define dso_local void @test1(i32 addrspace(1)* noalias noundef align 4 %results) {
entry:
  %call1 = tail call fastcc zeroext i1 @foo()
  ret void
}

define dso_local void @test2(i32 addrspace(1)* noalias noundef align 4 %results) {
entry:
  %call1 = tail call fastcc zeroext i1 @bar()
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test1, void (i32 addrspace(1)*)* @test2}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !3, i64 0}
