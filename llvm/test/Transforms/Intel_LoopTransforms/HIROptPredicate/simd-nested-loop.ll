; RUN: opt -disable-hir-opt-predicate-region-simd=false -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the If condition is hoisted out of the loopnest
; and the SIMD directives are inside the condition.

; HIR before transformation

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV:TYPED(null, 0),  QUAL.OMP.NORMALIZED.UB:TYPED(null, 0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0]), 0, 1, 1) ]
;
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd>
;       |   %1 = trunc.i64.i32(i1);
;       |   %2 = (%b)[i1];
;       |
;       |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   (%a)[i1 + zext.i32.i64(%1) * i2] = i1 + i2 + %2;
;       |   |
;       |   |   + DO i3 = 0, zext.i32.i64(%p1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   |   if (%p == 8)
;       |   |   |   {
;       |   |   |      (%a)[zext.i32.i64(%1) * i2 + i3] = i1 + %2;
;       |   |   |   }
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%p == 8)
; CHECK:       {
; CHECK:          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV:TYPED(null, 0),  QUAL.OMP.NORMALIZED.UB:TYPED(null, 0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0]), 0, 1, 1) ]
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd>
; CHECK:          |   %1 = trunc.i64.i32(i1);
; CHECK:          |   %2 = (%b)[i1];
; CHECK:          |
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   (%a)[i1 + zext.i32.i64(%1) * i2] = i1 + i2 + %2;
; CHECK:          |   |
; CHECK:          |   |   + DO i3 = 0, zext.i32.i64(%p1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   |   (%a)[zext.i32.i64(%1) * i2 + i3] = i1 + %2;
; CHECK:          |   |   + END LOOP
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV:TYPED(null, 0),  QUAL.OMP.NORMALIZED.UB:TYPED(null, 0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0]), 0, 1, 1) ]
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd>
; CHECK:          |   %1 = trunc.i64.i32(i1);
; CHECK:          |   %2 = (%b)[i1];
; CHECK:          |
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   (%a)[i1 + zext.i32.i64(%1) * i2] = i1 + i2 + %2;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       }
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture noundef writeonly %a, ptr nocapture noundef readonly %b, i32 noundef %n, i32 noundef %m, i32 noundef %p, i32 noundef %p1) local_unnamed_addr  {
entry:
  %i.linear.iv = alloca i32, align 4
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.158

DIR.OMP.SIMD.158:                                 ; preds = %DIR.OMP.SIMD.1
  %cmp644 = icmp sgt i32 %m, 0
  %cmp1342 = icmp sgt i32 %p1, 0
  %wide.trip.count56 = zext i32 %n to i64
  %wide.trip.count52 = zext i32 %m to i64
  %wide.trip.count = zext i32 %p1 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.158, %for.cond.cleanup
  %indvars.iv54 = phi i64 [ 0, %DIR.OMP.SIMD.158 ], [ %indvars.iv.next55, %for.cond.cleanup ]
  %1 = trunc i64 %indvars.iv54 to i32
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv54
  %2 = load i32, ptr %arrayidx, align 4
  %add5 = add nsw i32 %2, %1
  br i1 %cmp644, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %omp.inner.for.body
  br label %for.body

for.cond.loopexit.loopexit:                       ; preds = %for.body15
  br label %for.cond.loopexit

for.cond.loopexit:                                ; preds = %for.cond.loopexit.loopexit, %for.body
  %exitcond53.not = icmp eq i64 %indvars.iv.next48, %wide.trip.count52
  br i1 %exitcond53.not, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.loopexit
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %omp.inner.for.body
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next55, %wide.trip.count56
  br i1 %exitcond57.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

for.body:                                         ; preds = %for.body.preheader, %for.cond.loopexit
  %indvars.iv47 = phi i64 [ %indvars.iv.next48, %for.cond.loopexit ], [ 0, %for.body.preheader ]
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %3 = mul nuw nsw i64 %indvars.iv.next48, %indvars.iv54
  %arrayidx11 = getelementptr inbounds i32, ptr %a, i64 %3
  %4 = trunc i64 %indvars.iv47 to i32
  %5 = add i32 %add5, %4
  store i32 %5, ptr %arrayidx11, align 4
  br i1 %cmp1342, label %for.body15.lr.ph, label %for.cond.loopexit

for.body15.lr.ph:                                 ; preds = %for.body
  %6 = mul nuw nsw i64 %indvars.iv47, %indvars.iv54
  br label %for.body15

for.body15:                 ; preds = %for.body15.lr.ph, %for.body15 - innermost
  %indvars.iv = phi i64 [ 0, %for.body15.lr.ph ], [ %indvars.iv.next, %for.inc]
  %mycmp = icmp eq i32 %p, 8
  br i1 %mycmp, label %if.then, label %for.inc

if.then:
  %7 = add nuw nsw i64 %indvars.iv, %6
  %arrayidx19 = getelementptr inbounds i32, ptr %a, i64 %7
  store i32 %add5, ptr %arrayidx19, align 4
  br label %for.inc

for.inc:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.loopexit.loopexit, label %for.body15

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, %entry
  ret void
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
