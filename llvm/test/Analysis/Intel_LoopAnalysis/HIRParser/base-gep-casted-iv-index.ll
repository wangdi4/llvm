; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that we suppress traceback of %arrayidx and %arrayidx2 to %Aoffset and %Boffset to parse %idxprom index as linear.

;CHECK: + DO i1 = 0, -1 * %s + %n + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4294967295>
;CHECK: |   %inc3.out = i1 + %s + 1;
;CHECK: |   %0 = (%Aoffset)[i1 + %s + 1];
;CHECK: |   %1 = (%Boffset)[i1 + %s + 1];
;CHECK: |   if (%0 != %1)
;CHECK: |   {
;CHECK: |      goto while.body.while.end_crit_edge;
;CHECK: |   }
;CHECK: |   %inc = i1 + %s + 1  +  1;
;CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i8* nocapture readonly %A, i8* nocapture readonly %B, i32 %s, i32 %n, i64 %t) {
entry:
  %Aoffset = getelementptr inbounds i8, i8* %A, i64 %t
  %Boffset = getelementptr inbounds i8, i8* %B, i64 %t
  %inc1 = add i32 %s, 1
  %cmp2 = icmp eq i32 %s, %n
  br i1 %cmp2, label %while.end, label %while.body.lr.ph

while.body.lr.ph:                                 ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %while.cond
  %inc3 = phi i32 [ %inc1, %while.body.lr.ph ], [ %inc, %while.cond ]
  %idxprom = zext i32 %inc3 to i64
  %arrayidx = getelementptr inbounds i8, i8* %Aoffset, i64 %idxprom
  %0 = load i8, i8* %arrayidx, align 1
  %arrayidx2 = getelementptr inbounds i8, i8* %Boffset, i64 %idxprom
  %1 = load i8, i8* %arrayidx2, align 1
  %cmp4 = icmp eq i8 %0, %1
  br i1 %cmp4, label %while.cond, label %while.body.while.end_crit_edge

while.cond:                                       ; preds = %while.body
  %i.0 = phi i32 [ %inc3, %while.body ]
  %inc = add i32 %i.0, 1
  %cmp = icmp eq i32 %i.0, %n
  br i1 %cmp, label %while.cond.while.end_crit_edge, label %while.body

while.body.while.end_crit_edge:                   ; preds = %while.body
  %split = phi i32 [ %inc3, %while.body ]
  br label %while.end

while.cond.while.end_crit_edge:                   ; preds = %while.cond
  %split4 = phi i32 [ %inc, %while.cond ]
  br label %while.end

while.end:                                        ; preds = %while.cond.while.end_crit_edge, %while.body.while.end_crit_edge, %entry
  %inc.lcssa = phi i32 [ %split, %while.body.while.end_crit_edge ], [ %split4, %while.cond.while.end_crit_edge ], [ %inc1, %entry ]
  ret i32 %inc.lcssa
}

