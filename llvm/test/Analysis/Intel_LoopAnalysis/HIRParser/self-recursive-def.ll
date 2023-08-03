; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loopnest verifying that there is not self-recursive definition.
; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %.pre = (%A)[i1];
; CHECK: |   %0 = %.pre;
; CHECK: |
; CHECK: |   + DO i2 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   |   %1 = (%B)[i2];
; CHECK: |   |   %0 = %0  +  %1;
; CHECK: |   |   (%A)[i1] = %0;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Check that every instance of %0 is parsed as a non-linear self blob.
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output -hir-details 2>&1 | FileCheck %s -check-prefix=DETAIL

; DETAIL: HasSignedIV: Yes
; DETAIL: HasSignedIV: Yes

; DETAIL: %0 = %0  +  %1
; DETAIL: NON-LINEAR i32 %0
; DETAIL: NON-LINEAR i32 %0
; DETAIL: (%A)[i1] = %0
; DETAIL: NON-LINEAR i32 %0


; ModuleID = 'self-recursive-def.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %A, ptr nocapture readonly %B, i32 %n) {
entry:
  %cmp.18 = icmp sgt i32 %n, 0
  br i1 %cmp.18, label %for.body.3.lr.ph.preheader, label %for.end.8

for.body.3.lr.ph.preheader:                       ; preds = %entry
  br label %for.body.3.lr.ph

for.body.3.lr.ph:                                 ; preds = %for.body.3.lr.ph.preheader, %for.inc.6
  %indvars.iv20 = phi i64 [ %indvars.iv.next21, %for.inc.6 ], [ 0, %for.body.3.lr.ph.preheader ]
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv20
  %.pre = load i32, ptr %arrayidx5, align 4
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.body.3.lr.ph
  %0 = phi i32 [ %.pre, %for.body.3.lr.ph ], [ %add, %for.body.3 ]
  %indvars.iv = phi i64 [ 0, %for.body.3.lr.ph ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, %1
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc.6, label %for.body.3

for.inc.6:                                        ; preds = %for.body.3
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %lftr.wideiv22 = trunc i64 %indvars.iv.next21 to i32
  %exitcond23 = icmp eq i32 %lftr.wideiv22, %n
  br i1 %exitcond23, label %for.end.8.loopexit, label %for.body.3.lr.ph

for.end.8.loopexit:                               ; preds = %for.inc.6
  br label %for.end.8

for.end.8:                                        ; preds = %for.end.8.loopexit, %entry
  ret void
}
