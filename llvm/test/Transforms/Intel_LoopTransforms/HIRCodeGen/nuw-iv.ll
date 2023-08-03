; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 -hir-details | FileCheck %s

; for(i=-5; i<n; ++i) {
;   A[i+5] = i;
; }

; Check that parser does not set NSW for this loop as the normalized IV can overflow signed range.
; CHECK: HasSignedIV: No

; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck --check-prefix=CG %s

; Check that CG generatres nuw only IV for this loop
; CG: [[IV_UPDATE:%nextiv.*]] = add nuw i64 {{%.*}}, 1

; ModuleID = 'nuw-iv.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %A, i64 %n) {
entry:
  %cmp5 = icmp sgt i64 %n, -5
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i64 [ %inc, %for.body ], [ -5, %for.body.preheader ]
  %add = add nsw i64 %i.06, 5
  %arrayidx = getelementptr inbounds i64, ptr %A, i64 %add
  store i64 %i.06, ptr %arrayidx, align 8
  %inc = add nsw i64 %i.06, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
