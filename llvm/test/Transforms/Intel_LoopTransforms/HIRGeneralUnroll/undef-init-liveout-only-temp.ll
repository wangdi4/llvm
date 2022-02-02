; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -hir-cg -sroa -print-after=sroa -S < %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>,hir-cg,sroa,print" -S < %s 2>&1 | FileCheck %s

; Verify that we initialize unconditional liveout only temps like %call to undef
; before the unrolled loop. This is done to avoid artificial phis being created
; in outer loops headers for private temps.

; HIR-
; + DO i1 = 0, %n1 + -1, 1   <DO_LOOP>
; |   %tmp.priv.0.lcssa = 1111;
; |
; |   + DO i2 = 0, %n2 + -1, 1   <DO_LOOP>
; |   |   %call = @baz();
; |   + END LOOP
; |      %tmp.priv.0.lcssa = %call;
; |
; |   (%larr)[i1] = %tmp.priv.0.lcssa;
; + END LOOP

; CHECK: |      %tgu = (%n2)/u8;
; CHECK: |      %call = undef;
; CHECK: |
; CHECK: |      + DO i2 = 0, %tgu + -1, 1   <DO_LOOP> <nounroll>
; CHECK: |
; CHECK: |      + DO i2 = 8 * %tgu, %n2 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>   <LEGAL_MAX_TC = 7> <nounroll> <max_trip_count = 7>

; Checl that there is only ony phi in the outer loop header after SROA.
; CHECK: loop{{.*}}:
; CHECK: %i1{{.*}} = phi i64
; CHECK-NEXT:  {{%.*}} = icmp sgt i64 %n2, 0


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64 %n1, i64 %n2, i64* nocapture %larr) {
entry:
  %l1.linear.iv = alloca i64, align 8
  %cmp = icmp sgt i64 %n1, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %cmp726 = icmp sgt i64 %n2, 0
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %for.end, %DIR.OMP.SIMD.2
  %.omp.iv.local.020 = phi i64 [ %add8, %for.end ], [ 0, %DIR.OMP.SIMD.2 ]
  br i1 %cmp726, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %omp.inner.for.body
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l2.027 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %call = call i64 (...) @baz() #1
  %inc = add nuw nsw i64 %l2.027, 1
  %exitcond = icmp eq i64 %inc, %n2
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %call.lcssa = phi i64 [ %call, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %omp.inner.for.body
  %tmp.priv.0.lcssa = phi i64 [ 1111, %omp.inner.for.body ], [ %call.lcssa, %for.end.loopexit ]
  %arrayidx = getelementptr inbounds i64, i64* %larr, i64 %.omp.iv.local.020
  store i64 %tmp.priv.0.lcssa, i64* %arrayidx, align 8
  %add8 = add nuw nsw i64 %.omp.iv.local.020, 1
  %exitcond28 = icmp eq i64 %add8, %n1
  br i1 %exitcond28, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %for.end
  store i64 %n1, i64* %l1.linear.iv, align 8
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

declare dso_local i64 @baz(...)

