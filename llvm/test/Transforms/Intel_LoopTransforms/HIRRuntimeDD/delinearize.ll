; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check that loop can be multiversioned after reference delinearization.
;
; Strides:
; (sext i32 %n to i64)
;
; Sizes:
; (sext i32 %n to i64)
; 1
;
; Delinearized refs:
; (%p)[sext.i32.i64(%n) * i1 + i2] -> (%p)[i1][i2]
; (%p)[sext.i32.i64(%n) * i1 + i2 + sext.i32.i64(%n)] -> (%p)[i1 + 1][i2]
;
; Group 0 contains (2) refs:
; (%q)[i2]
; (%q)[i2]
; Group 1 contains (2) refs:
; (%p)[i1][i2]
; (%p)[i1 + 1][i2]
;
; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(%k) + -1, 1   <DO_LOOP>
;       |   |   %2 = (%q)[i2];
;       |   |   (%p)[sext.i32.i64(%n) * i1 + i2] = %2;
;       |   |   %4 = (%q)[i2];
;       |   |   (%p)[sext.i32.i64(%n) * i1 + i2 + sext.i32.i64(%n)] = %4;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK-DAG: &((%q)[sext.i32.i64(%k) + -1]) >=u &((%p)[0][0]);
; CHECK-DAG: &((%p)[100][sext.i32.i64(%k) + -1]) >=u &((%q)[0]);
; CHECK: sext.i32.i64(%k) + -1 < sext.i32.i64(%n)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %p, i32* nocapture readonly %q, i32 %n, i32 %k) {
entry:
  %cmp231 = icmp sgt i32 %k, 0
  br i1 %cmp231, label %for.cond1.preheader.us.preheader, label %for.cond.cleanup

for.cond1.preheader.us.preheader:                 ; preds = %entry
  %0 = sext i32 %n to i64
  %wide.trip.count = sext i32 %k to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us, %for.cond1.preheader.us.preheader
  %indvar = phi i64 [ 0, %for.cond1.preheader.us.preheader ], [ %indvar.next, %for.cond1.for.cond.cleanup3_crit_edge.us ]
  %1 = mul nsw i64 %indvar, %0
  br label %for.body4.us

for.body4.us:                                     ; preds = %for.body4.us, %for.cond1.preheader.us
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next, %for.body4.us ]
  %arrayidx.us = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx.us, align 4
  %3 = add nsw i64 %indvars.iv, %1
  %arrayidx6.us = getelementptr inbounds i32, i32* %p, i64 %3
  store i32 %2, i32* %arrayidx6.us, align 4
  %4 = load i32, i32* %arrayidx.us, align 4
  %5 = add nsw i64 %3, %0
  %arrayidx13.us = getelementptr inbounds i32, i32* %p, i64 %5
  store i32 %4, i32* %arrayidx13.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond1.for.cond.cleanup3_crit_edge.us, label %for.body4.us

for.cond1.for.cond.cleanup3_crit_edge.us:         ; preds = %for.body4.us
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond41 = icmp eq i64 %indvar.next, 100
  br i1 %exitcond41, label %for.cond.cleanup.loopexit, label %for.cond1.preheader.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}

