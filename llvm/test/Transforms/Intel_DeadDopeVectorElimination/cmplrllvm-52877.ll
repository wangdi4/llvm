; RUN: opt -whole-program-assume -passes='deaddopevectorelimination' -S < %s 2>&1 | FileCheck %s

; CHECK: define dso_local void @MAIN__() {
; CHECK: store i64 1, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"iter8_$IN", i64 0, i32 6, i64 1, i32 2), align 8

; For CMPLRLLVM-52877, check that the compiler does not assert because there
; is a getelementptr with DimIndex != 0.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
@"iter8_$IN" = internal unnamed_addr global %"QNCA_a0$float*$rank2$" { ptr null, i64 0, i64 0, i64 0, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

define dso_local void @MAIN__() {
bb:
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"iter8_$IN", i64 0, i32 6, i64 1, i32 2), align 8
  ret void
}

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}
!ifx.types.dv = !{!2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}

