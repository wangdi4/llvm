; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s

; Check that the side effect analysis identifies that hoisting the condition
; will increase code size. Both branches won't produce a side effect, but the
; store after the condition will produce a side effect. This test case was
; created from the following C++ code:

; void foo(int *a, int n, int m) {
;   int temp1 = 0;
;   for (int i = 0; i < n; i++) {
;     if (m == 5) {
;       temp1++;
;     } else {
;       temp1+=2;
;     }
;     a[0] = temp1;
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (%m == 5)
;       |   {
;       |      %temp1.010 = %temp1.010  +  1;
;       |   }
;       |   else
;       |   {
;       |      %temp1.010 = %temp1.010  +  2;
;       |   }
;       |   (%a)[0] = %temp1.010;
;       + END LOOP
; END REGION

; Debug trace

; CHECK: Opportunity: <2>          if (%m == 5) --> Level 0, Candidate: 1
; CHECK-NEXT:   - Code size will increase, thresholds needed

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%m == 5)
; CHECK:       {
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   %temp1.010 = %temp1.010  +  1;
; CHECK:          |   (%a)[0] = %temp1.010;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   %temp1.010 = %temp1.010  +  2;
; CHECK:          |   (%a)[0] = %temp1.010;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION



;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_ii(ptr nocapture noundef writeonly %a, ptr nocapture noundef writeonly %b, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp eq i32 %m, 5
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %temp1.010 = phi i32 [ 0, %for.body.lr.ph ], [ %temp1.1, %for.inc ]
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %temp1.010, 1
  br label %for.inc

if.else:                                          ; preds = %for.body
  %inc2 = add nsw i32 %temp1.010, 2
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %temp1.1 = phi i32 [ %inc, %if.then ], [ %inc2, %if.else ]
  store i32 %temp1.1, ptr %a
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}