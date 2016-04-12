; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; for(i=-5; i<n; ++i) {
;   A[i+5] = i;
; }

; Check that parser does not set NSW for this loop as the normalized IV can overflow signed range.
; CHECK: NSW: No

; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck --check-prefix=CG %s

; Check that CG generatres nuw only IV for this loop
; CG: [[IV_UPDATE:%nextiv.*]] = add nuw i64 {{%.*}}, 1
; CG: icmp ule i64 [[IV_UPDATE]]

; ModuleID = 'nuw-iv.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64* nocapture %A, i64 %n) {
entry:
  %cmp5 = icmp sgt i64 %n, -5
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i64 [ %inc, %for.body ], [ -5, %for.body.preheader ]
  %add = add nsw i64 %i.06, 5
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %add
  store i64 %i.06, i64* %arrayidx, align 8
  %inc = add nsw i64 %i.06, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
