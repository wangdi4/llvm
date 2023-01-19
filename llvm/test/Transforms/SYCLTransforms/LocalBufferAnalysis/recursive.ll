; RUN: opt -passes='print<dpcpp-kernel-local-buffer-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check local buffer size and offset of local variables in not-inlined case
; with recursive function calls.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: LocalBufferInfo
; CHECK:   Local variables used in kernel
; CHECK:     test
; CHECK-DAG:   test.i
; CHECK-DAG:   test.k
; CHECK-DAG:   test.j
; CHECK:   Kernel local buffer size
; CHECK:     test : 408
; CHECK:   Offset of local variable in containing kernel's local buffer
; CHECK-DAG:   test.j : 404
; CHECK-DAG:   test.k : 400
; CHECK-DAG:   test.i : 0

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.k = internal addrspace(3) global i32 undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4

define internal fastcc zeroext i1 @xxx() !recursive_call !1 {
entry:
  %0 = load i32, i32 addrspace(3)* @test.k, align 4, !tbaa !2
  %1 = tail call fastcc zeroext i1 @foo()
  ret i1 false
}

define internal fastcc zeroext i1 @foo() !recursive_call !1 {
entry:
  %0 = load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4, !tbaa !2
  %1 = tail call fastcc zeroext i1 @xxx()
  ret i1 false
}

define internal fastcc zeroext i1 @bar() !recursive_call !1 {
entry:
  %0 = load i32, i32 addrspace(3)* @test.j, align 4, !tbaa !2
  %1 = tail call fastcc zeroext i1 @foo()
  ret i1 false
}

define dso_local void @test(i32 addrspace(1)* noalias noundef align 4 %results) !recursive_call !1 {
entry:
  store i32 0, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4
  store i32 0, i32 addrspace(3)* @test.j, align 4, !tbaa !2
  store i32 0, i32 addrspace(3)* @test.k, align 4, !tbaa !2
  %0 = tail call fastcc zeroext i1 @bar()
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test}
!1 = !{i1 true}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
