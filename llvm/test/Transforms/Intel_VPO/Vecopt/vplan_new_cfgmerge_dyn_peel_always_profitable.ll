; This test verifies that, when computing a profitable peel TC for dynamic
; peeling, the vectorizer correctly identifies scenarios where the computed
; threshold would be less then UF*VF indicating that peeling is profitable
; for all trip counts.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec' \
; RUN:       -vplan-enable-vectorized-peel=true -vplan-enable-peeling=true \
; RUN:       -vplan-force-vf=8 -vplan-force-uf=4 -mattr=+avx512f \
; RUN:       -debug-only=LoopVectorizationPlanner_peel_tc 2>&1 | FileCheck %s

; CHECK:  (VF = 8, UF = 4) min profitable peel tc = 31
; CHECK:  Min profitable peel tc is less than VF*UF, skipping

define void @foo(ptr %A, i64 %N) {
entry:
  br label %DIR.OMP.SIMD

DIR.OMP.SIMD:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                             ; preds = %for.body, %DIR.OMP.SIMD
  %iv = phi i64 [ 0, %DIR.OMP.SIMD ], [ %iv.next, %for.body ]
  %arrayidx = getelementptr float, ptr %A, i64 %iv
  %1 = load float, ptr %arrayidx, align 4
  %2 = load float, ptr %arrayidx, align 4
  %3 = load float, ptr %arrayidx, align 4
  %4 = load float, ptr %arrayidx, align 4
  %5 = load float, ptr %arrayidx, align 4
  %6 = load float, ptr %arrayidx, align 4
  %7 = load float, ptr %arrayidx, align 4
  %8 = load float, ptr %arrayidx, align 4
  %9 = load float, ptr %arrayidx, align 4
  %10 = load float, ptr %arrayidx, align 4
  %11 = load float, ptr %arrayidx, align 4
  %12 = load float, ptr %arrayidx, align 4
  %13 = load float, ptr %arrayidx, align 4
  %14 = load float, ptr %arrayidx, align 4
  %iv.next = add i64 %iv, 1
  %exitcond.not = icmp uge i64 %iv, %N
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD, label %for.body

DIR.OMP.END.SIMD:                               ; preds = %for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
