; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the If in the nested loops was identified that
; will increase code size since there is a store in the outer loop and the
; hoist level is 0. It was created from the following test in C++:

; void foo(int *a, int n, int m, int l, int *b) {
;   for (int i = 0; i < n; i++) {
;     b[i] = i;
;     int temp1 = 0;
;     for(int j = 0; j < m; j++) {
;       if (l == 5) {
;         a[i * n + j] = temp1;
;       } else {
;         temp1++;
;       }
;     }
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   (%b)[i1] = i1;
;       |   
;       |      %temp1.021 = 0;
;       |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   if (%l == 5)
;       |   |   {
;       |   |      (%a)[zext.i32.i64(%n) * i1 + i2] = %temp1.021;
;       |   |   }
;       |   |   else
;       |   |   {
;       |   |      %temp1.021 = %temp1.021  +  1;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; Debug trace

; CHECK: Opportunity: <15>         if (%l == 5) --> Level 0, Candidate: Yes
; CHECK-NEXT:   - Code size will increase, thresholds needed

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%l == 5)
; CHECK:       {
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   (%b)[i1] = i1;
; CHECK:          |   
; CHECK:          |      %temp1.021 = 0;
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   (%a)[zext.i32.i64(%n) * i1 + i2] = %temp1.021;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   (%b)[i1] = i1;
; CHECK:          |   if (%m > 0)
; CHECK:          |   {
; CHECK:          |      %temp1.021 = 0;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiiiiS_(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %m, i32 noundef %l, ptr nocapture noundef writeonly %b) {
entry:
  %cmp23 = icmp sgt i32 %n, 0
  br i1 %cmp23, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp220 = icmp sgt i32 %m, 0
  %cmp5 = icmp eq i32 %l, 5
  %0 = zext i32 %n to i64
  %wide.trip.count = zext i32 %m to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.cond.cleanup3
  %indvars.iv26 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next27, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv26
  %1 = trunc i64 %indvars.iv26 to i32
  store i32 %1, ptr %arrayidx, align 4
  br i1 %cmp220, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.body
  %2 = mul nsw i64 %indvars.iv26, %0
  br label %for.body4

for.cond.cleanup3.loopexit:                       ; preds = %for.inc
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.body
  %indvars.iv.next27 = add nuw nsw i64 %indvars.iv26, 1
  %exitcond30.not = icmp eq i64 %indvars.iv.next27, %0
  br i1 %exitcond30.not, label %for.cond.cleanup.loopexit, label %for.body

for.body4:                                        ; preds = %for.body4.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %temp1.021 = phi i32 [ 0, %for.body4.lr.ph ], [ %temp1.1, %for.inc ]
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %for.body4
  %3 = add nuw nsw i64 %indvars.iv, %2
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %3
  store i32 %temp1.021, ptr %arrayidx7, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body4
  %inc = add nsw i32 %temp1.021, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %temp1.1 = phi i32 [ %temp1.021, %if.then ], [ %inc, %if.else ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup3.loopexit, label %for.body4
}
