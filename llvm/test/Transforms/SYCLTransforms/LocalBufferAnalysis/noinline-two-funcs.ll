; RUN: opt -passes='print<sycl-kernel-local-buffer-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check local buffer size and offset of local variables in the case a kernel
; calls two noinline functions.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: LocalBufferInfo
; CHECK:   Local variables used in kernel
; CHECK:     test
; CHECK-DAG:   test.i
; CHECK-DAG:   test.k
; CHECK-DAG:   test.j
; CHECK:   Kernel local buffer size
; CHECK:     test : 16
; CHECK:   Offset of local variable in containing kernel's local buffer
; CHECK-DAG:   test.j : 8
; CHECK-DAG:   test.k : 4
; CHECK-DAG:   test.i : 0

@test.i = internal addrspace(3) global i8 undef, align 1
@test.k = internal addrspace(3) global i32 undef, align 4
@test.j = internal addrspace(3) global i64 undef, align 8

define internal fastcc zeroext i1 @bar() {
entry:
  %0 = load i8, ptr addrspace(3) @test.i, align 1, !tbaa !9
  %1 = load i32, ptr addrspace(3) @test.k, align 4
  ret i1 false
}

define internal fastcc zeroext i1 @foo() {
entry:
  %0 = load i8, ptr addrspace(3) @test.i, align 1, !tbaa !9
  %1 = load i64, ptr addrspace(3) @test.j, align 8
  ret i1 false
}

define dso_local void @test() {
entry:
  %0 = call fastcc zeroext i1 @bar()
  %1 = call fastcc zeroext i1 @foo()
  ret void
}

!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.stat.type = !{!3}
!sycl.stat.exec_time = !{!4}
!sycl.stat.run_time_version = !{!5}
!sycl.stat.workload_name = !{!6}
!sycl.stat.module_name = !{!7}
!sycl.kernels = !{!8}

!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0", !"-cl-kernel-arg-info"}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!""}
!4 = !{!"2022-11-17 11:37:40"}
!5 = !{!"2022.15.11.0"}
!6 = !{!"_build_cl"}
!7 = !{!"_build_cl1"}
!8 = !{ptr @test}
!9 = !{!10, !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
