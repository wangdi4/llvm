; RUN: opt -hir-details -disable-output -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-details -disable-output -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; Check that %p.addr.022 def level is updated after opt-predicate.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (%n == 50)
;       |   {
;       |      %0 = (%q)[i1];
;       |      %p.addr.09 = &((%0)[0]);
;       |   }
;       |   (%p.addr.09)[i1] = i1;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: if (%n == 50)
; CHECK: {
; CHECK:    + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   %0 = (%q)[i1];
; CHECK:    |   %p.addr.09 = &((%0)[0]);
; CHECK:    |   <LVAL-REG> NON-LINEAR i32* %p.addr.09
; CHECK:    |
; CHECK:    |   (%p.addr.09)[i1] = i1;
; CHECK:    |   <LVAL-REG> {al:4}(NON-LINEAR i32* %p.addr.09)[LINEAR i64 i1]
; CHECK:    |      <BLOB> NON-LINEAR i32* %p.addr.09
; CHECK:    + END LOOP
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   (%p.addr.09)[i1] = i1;
; CHECK:    |   <LVAL-REG> {al:4}(LINEAR i32* %p.addr.09)[LINEAR i64 i1]
; CHECK:    |   <BLOB> LINEAR i32* %p.addr.09
; CHECK:    + END LOOP
; CHECK: }
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %p, i32** nocapture readonly %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp eq i32 %n, 50
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %p.addr.09 = phi i32* [ %p, %entry ], [ %p.addr.1, %if.end ]
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32*, i32** %q, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %p.addr.1 = phi i32* [ %0, %if.then ], [ %p.addr.09, %for.body ]
  %arrayidx3 = getelementptr inbounds i32, i32* %p.addr.1, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

