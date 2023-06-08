; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local fastcc void @foo() {
entry:
  ret void
}

define dso_local void @bar() {
entry:
  call void @dummy_barrier.()
  tail call void @_Z18work_group_barrierj(i32 1) #0
  unreachable
}

declare dso_local void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()

define dso_local void @kernel() {
DIR.OMP.PARALLEL.LOOP.3:
  br i1 undef, label %loop.region.exit, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %DIR.OMP.PARALLEL.LOOP.3
  call void @dummy_barrier.()
  tail call void @_Z18work_group_barrierj(i32 1) #0
  tail call void @bar()
  call void @dummy_barrier.()
  unreachable

loop.region.exit:                                 ; preds = %DIR.OMP.PARALLEL.LOOP.3
  tail call void @_Z18work_group_barrierj(i32 1) #0
  ret void
}

attributes #0 = { convergent nounwind }

; CHECK-LABEL: Group-A Values
; CHECK-LABEL: Group-B.1 Values
; CHECK-LABEL: Group-B.2 Values
; CHECK-LABEL: Function Equivalence Classes:
; CHECK-DAG: [foo]: foo
; CHECK-DAG: [kernel]: kernel bar

; CHECK-NEXT: Buffer Total Size:
; CHECK-DAG: leader(foo) : (0)
; CHECK-DAG: leader(kernel) : (0)
; CHECK-NEXT: DONE

!sycl.kernels = !{!0}
!0 = !{ptr @kernel}
