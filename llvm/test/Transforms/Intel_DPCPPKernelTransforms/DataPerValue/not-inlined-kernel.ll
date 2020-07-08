; RUN: opt -analyze -dpcpp-kernel-data-per-value-analysis %s -S -o - | FileCheck %s
; XFAIL: windows-msvc

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local fastcc void @foo() {
entry:
  ret void
}

define dso_local void @bar() {
entry:
  tail call void @__builtin_dpcpp_kernel_barrier() #0
  unreachable
}

declare dso_local void @__builtin_dpcpp_kernel_barrier()

define dso_local void @kernel() #1 {
DIR.OMP.PARALLEL.LOOP.3:
  br i1 undef, label %loop.region.exit, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %DIR.OMP.PARALLEL.LOOP.3
  tail call void @bar()
  unreachable

loop.region.exit:                                 ; preds = %DIR.OMP.PARALLEL.LOOP.3
  ret void
}

attributes #0 = { convergent nounwind }
attributes #1 = { "sycl_kernel" }

; CHECK-LABEL: Group-A Values
; CHECK-LABEL: Group-B.1 Values
; CHECK-LABEL: Group-B.2 Values
; CHECK-LABEL: Buffer Total Size:
; CHECK-NEXT: +foo : [0]
; CHECK-NEXT: +bar : [2]
; CHECK-NEXT: +kernel : [2]
; CHECK-NEXT: entry(0) : (0)
; CHECK-NEXT: entry(2) : (0)
; CHECK-LABEL: DONE
