; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-before=hir-complete-unroll -print-after=hir-complete-unroll -hir-details 2>&1 < %s | FileCheck %s

; Source code-
;  for(i=0; i<60; i++) {
;    a = A[i];
;    for(j=0; j<40; j++) {
;      b = A[j];
;      for(k=0; k<6; k++) {
;        A[k] = i + k*b;
;        A[2*k] = a + k*b;
;        A[3*k] = b;
;      }
;    }
;  }

; Check the following updates to canon expr def level of rvals in the first iteration (when k is 0) after complete unrolling of innermost loop-
; i + k*b {def@2} -> i {linear}
; a + k*b {def@2} -> a {def@1}
; b {def@2} -> b {non-linear}

; CHECK: Dump Before
; CHECK: (%A)[i3] = i1 + sext.i32.i64(%2) * i3
; CHECK: <RVAL-REG> LINEAR trunc.i64.i32(i1 + sext.i32.i64(%2) * i3){def@2}

; CHECK: (%A)[2 * i3] = sext.i32.i64(%2) * i3 + zext.i32.i64(%0)
; CHECK: <RVAL-REG> LINEAR trunc.i64.i32(sext.i32.i64(%2) * i3 + zext.i32.i64(%0)){def@2}

; CHECK: (%A)[3 * i3] = %2
; CHECK: <RVAL-REG> LINEAR i32 %2{def@2}


; CHECK: Dump After
; CHECK: (%A)[0] = i1
; CHECK: <RVAL-REG> LINEAR trunc.i64.i32(i1)

; Check that there isn't any def level for the above rval.
; CHECK-NOT: def

; CHECK: (%A)[0] = zext.i32.i64(%0)
; CHECK: <RVAL-REG> LINEAR trunc.i64.i32(zext.i32.i64(%0)){def@1}

; CHECK: (%A)[0] = %2
; CHECK: <RVAL-REG> NON-LINEAR i32 %2


; ModuleID = 'canon-def-level1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc.22, %entry
  %indvars.iv58 = phi i64 [ 0, %entry ], [ %indvars.iv.next59, %for.inc.22 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv58
  %0 = load i32, i32* %arrayidx, align 4
  %1 = zext i32 %0 to i64
  br label %for.body.3

for.body.3:                                       ; preds = %for.inc.19, %for.body
  %indvars.iv55 = phi i64 [ 0, %for.body ], [ %indvars.iv.next56, %for.inc.19 ]
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv55
  %2 = load i32, i32* %arrayidx5, align 4
  %3 = sext i32 %2 to i64
  br label %for.body.8

for.body.8:                                       ; preds = %for.body.8, %for.body.3
  %indvars.iv = phi i64 [ 0, %for.body.3 ], [ %indvars.iv.next, %for.body.8 ]
  %4 = mul nsw i64 %indvars.iv, %3
  %5 = add nsw i64 %4, %indvars.iv58
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %6 = trunc i64 %5 to i32
  store i32 %6, i32* %arrayidx10, align 4
  %7 = add i64 %4, %1
  %8 = shl nsw i64 %indvars.iv, 1
  %arrayidx15 = getelementptr inbounds i32, i32* %A, i64 %8
  %9 = trunc i64 %7 to i32
  store i32 %9, i32* %arrayidx15, align 4
  %10 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx18 = getelementptr inbounds i32, i32* %A, i64 %10
  store i32 %2, i32* %arrayidx18, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %for.inc.19, label %for.body.8

for.inc.19:                                       ; preds = %for.body.8
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next56, 40
  br i1 %exitcond57, label %for.inc.22, label %for.body.3

for.inc.22:                                       ; preds = %for.inc.19
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond60 = icmp eq i64 %indvars.iv.next59, 60
  br i1 %exitcond60, label %for.end.24, label %for.body

for.end.24:                                       ; preds = %for.inc.22
  ret void
}

