; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the Select instruction was converted into an
; If, and opt-predicate was applied even if the loop is SIMD. This case
; can be handled since the directives are in the preheader and postexit of
; the loop.

; Before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |      %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0]), 1) ]
;       |   + DO i2 = 0, %m + -1, 1   <DO_LOOP> <simd>
;       |   |   %tmp1 = (%n == 20) ? 0 : 1;
;       |   |   (%p)[i2] = i2 + %tmp1;
;       |   + END LOOP
;       |      @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       + END LOOP
; END REGION


; After transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n == 20)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |      %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0]), 1) ]
; CHECK:          |   + DO i2 = 0, %m + -1, 1   <DO_LOOP> <simd>
; CHECK:          |   |   %tmp1 = 0;
; CHECK:          |   |   (%p)[i2] = i2 + %tmp1;
; CHECK:          |   + END LOOP
; CHECK:          |      @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |      %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0]), 1) ]
; CHECK:          |   + DO i2 = 0, %m + -1, 1   <DO_LOOP> <simd>
; CHECK:          |   |   %tmp1 = 1;
; CHECK:          |   |   (%p)[i2] = i2 + %tmp1;
; CHECK:          |   + END LOOP
; CHECK:          |      @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %p, i32 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp eq i32 %n, 20
  %j.linear.iv = alloca i32, align 4
  br label %outer.loop.ph

outer.loop.ph:
  br label %outer.loop.for.body

outer.loop.for.body:
  %indvars.iv = phi i64 [ 0, %outer.loop.ph ], [ %inc, %outer.loop.end ]
  %cmp2 = icmp sgt i64 %m, 0
  br i1 %cmp2, label %omp.inner.for.body.lr.ph, label %outer.loop.end

omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV"(ptr %j.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv2 = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.lr.ph ]
  %tmp1 = select i1 %cmp1, i32 0, i32 1
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv2
  %tr = trunc i64 %indvars.iv2 to i32
  %a = add i32 %tmp1, %tr
  store i32 %a, ptr %arrayidx, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv2, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %m
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %outer.loop.end

outer.loop.end:
  %inc = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %inc, 100
  br i1 %cmp, label %exit, label %outer.loop.for.body

exit:
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)