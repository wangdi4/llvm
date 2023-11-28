; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-lmm -disable-output < %s 2>&1 | FileCheck %s

; This test case checks if the store happens before a goto and does not
; dominate then we can't apply the transformation.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, -1 * %g + 5, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483654>  <LEGAL_MAX_TC = 2147483654>
;       |   %0 = 0;
;       |   if (%h != 0)
;       |   {
;       |      (@b)[0] = i1 + %g;
;       |      %tobool1 = i1 + %g != 0;
;       |      %0 = %tobool1;
;       |   }
;       |   if (%0 + 1 != -1 * i1 + -1 * trunc.i32.i16(%g))
;       |   {
;       |      goto for.end.loopexit;
;       |   }
;       |   (@b)[0] = 0;
;       + END LOOP
; END REGION

; Print debug information

; CHECK: HIR LIMM on Function : f
; CHECK:  Legality failure: Ref (@b)[0] in node <8> doesn't dominate goto <19>
; CHECK:    Node: <8>          (@b)[0] = i1 + %g;
; CHECK:    Goto: <19>         goto for.end.loopexit;
; CHECK: HIRLMM: failed legal test

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, -1 * %g + 5, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483654>  <LEGAL_MAX_TC = 2147483654>
; CHECK:       |   %0 = 0;
; CHECK:       |   if (%h != 0)
; CHECK:       |   {
; CHECK:       |      (@b)[0] = i1 + %g;
; CHECK:       |      %tobool1 = i1 + %g != 0;
; CHECK:       |      %0 = %tobool1;
; CHECK:       |   }
; CHECK:       |   if (%0 + 1 != -1 * i1 + -1 * trunc.i32.i16(%g))
; CHECK:       |   {
; CHECK:       |      goto for.end.loopexit;
; CHECK:       |   }
; CHECK:       |   (@b)[0] = 0;
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local local_unnamed_addr global i32 8

define dso_local void @f(i32 noundef %g, i64 noundef %h, i32 %m, i32 %n) {
entry:
  %cmp7 = icmp slt i32 %g, 6
  br i1 %cmp7, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %tobool.not = icmp eq i64 %h, 0
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %g.addr.08 = phi i32 [ %g, %for.body.lr.ph ], [ %inc, %for.inc ]
  %conv = trunc i32 %g.addr.08 to i16
  br i1 %tobool.not, label %land.end, label %land.rhs

land.rhs:                                         ; preds = %for.body
  store i32 %g.addr.08, ptr @b
  %tobool1 = icmp ne i32 %g.addr.08, 0
  br label %land.end

land.end:                                         ; preds = %land.rhs, %for.body
  %0 = phi i1 [ false, %for.body ], [ %tobool1, %land.rhs ]
  %1 = xor i1 %0, true
  %conv2.neg = sext i1 %1 to i16
  %sub.i = sub i16 0, %conv
  %tobool3.not.not = icmp eq i16 %conv2.neg, %sub.i
  br i1 %tobool3.not.not, label %for.inc, label %for.end.loopexit

for.inc:                                          ; preds = %land.end
  store i32 0, ptr @b
  %inc = add i32 %g.addr.08, 1
  %exitcond.not = icmp eq i32 %inc, 6
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %land.end, %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
