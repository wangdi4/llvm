; RUN: opt -passes='print<sycl-kernel-local-buffer-analysis>' -S --disable-output %s 2>&1 | FileCheck %s

; Check local buffer size and offset in following case:
;     test (kernel)
;      |
;     foo
;      |
;     bar

; CHECK: LocalBufferInfo
; CHECK:   Local variables used in kernel
; CHECK:     test
; CHECK:       test.i
; CHECK:       test.k
; CHECK:       test.j
; CHECK:   Kernel local buffer size
; CHECK:     test : 408
; CHECK:   Offset of local variable in containing kernel's local buffer
; CHECK-DAG: test.j : 404
; CHECK-DAG: test.k : 400
; CHECK-DAG: test.i : 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.k = internal addrspace(3) global i32 undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4

define internal fastcc zeroext i1 @foo() {
entry:
  %0 = load i32, ptr addrspace(3) @test.i, align 4, !tbaa !1
  %1 = load i32, ptr addrspace(3) @test.k, align 4
  ret i1 false
}

define internal fastcc zeroext i1 @bar() {
entry:
  %0 = load i32, ptr addrspace(3) @test.j, align 4, !tbaa !1
  %call = tail call fastcc zeroext i1 @foo()
  ret i1 false
}

define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %results) !kernel_arg_base_type !5 !arg_type_null_val !6 {
entry:
  store i32 1234, ptr addrspace(3) @test.i, align 4, !tbaa !1
  store i32 1234, ptr addrspace(3) @test.j, align 4, !tbaa !1
  store i32 1234, ptr addrspace(3) @test.k, align 4, !tbaa !1
  %call1 = tail call fastcc zeroext i1 @bar()
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!"int*"}
!6 = !{ptr addrspace(1) null}
