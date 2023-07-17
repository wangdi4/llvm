; RUN: opt -passes='print<sycl-kernel-local-buffer-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check local buffer size and offset of local variables are the same for a
; kernel and its vectorized kernel.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: LocalBufferInfo
; CHECK:   Local variables used in kernel
; CHECK:     test
; CHECK-DAG:   test.l
; CHECK-DAG:   test.c
; CHECK-DAG:   test.i
; CHECK:     _ZGVeN16u_test
; CHECK-DAG:   test.l
; CHECK-DAG:   test.c
; CHECK-DAG:   test.i
; CHECK:   Kernel local buffer size
; CHECK:     test : 20
; CHECK:     _ZGVeN16u_test : 20
; CHECK:   Offset of local variable in containing kernel's local buffer
; CHECK-DAG:   test.c : 8
; CHECK-DAG:   test.l : 0
; CHECK-DAG:   test.i : 12

@test.l = internal unnamed_addr addrspace(3) global [1 x i64] undef, align 8
@test.c = internal unnamed_addr addrspace(3) global [1 x i8] undef, align 1
@test.i = internal unnamed_addr addrspace(3) global [2 x i32] undef, align 4

define dso_local void @test() !vectorized_kernel !1 {
entry:
  %0 = load i64, ptr addrspace(3) getelementptr inbounds ([1 x i64], ptr addrspace(3) @test.l, i64 0, i64 0), align 8, !tbaa !2
  %1 = load i8, ptr addrspace(3) getelementptr inbounds ([1 x i8], ptr addrspace(3) @test.c, i64 0, i64 0), align 1, !tbaa !6
  %2 = load i32, ptr addrspace(3) getelementptr inbounds ([2 x i32], ptr addrspace(3) @test.i, i64 0, i64 0), align 4, !tbaa !7
  ret void
}

define dso_local void @_ZGVeN16u_test() !scalar_kernel !0 {
entry:
  %0 = getelementptr inbounds [1 x i64], ptr addrspace(3) @test.l, i64 0, i64 0
  %1 = getelementptr inbounds [1 x i8], ptr addrspace(3) @test.c, i64 0, i64 0
  %2 = getelementptr inbounds [2 x i32], ptr addrspace(3) @test.i, i64 0, i64 0
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{ptr @_ZGVeN16u_test}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!4, !4, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !4, i64 0}
