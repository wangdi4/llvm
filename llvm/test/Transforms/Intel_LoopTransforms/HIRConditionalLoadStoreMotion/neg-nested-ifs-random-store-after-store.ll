; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation didn't sunk the stores of
; %a[i] in the nested Ifs due to the write of %a[m]. There is a chance that
; m is equal to i, and it may overwrite the value of a[i]. It was created from
; the following example in C++:

; void foo(int *a, int n, int m) {
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       a[i] = 0;
;     } else if (m * i == 5) {
;       a[i] = 1;
;       a[m] = 2;
;     } else {
;       a[i] = -1;
;     }
;   }
; }

; HIR before:

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (i1 + sext.i32.i64(%m) == 2)
;       |   {
;       |      (%a)[i1] = 0;
;       |   }
;       |   else
;       |   {
;       |      if (%m * i1 == 5)
;       |      {
;       |         (%a)[i1] = 1;
;       |         (%a)[%m] = 2;
;       |      }
;       |      else
;       |      {
;       |         (%a)[i1] = -1;
;       |      }
;       |   }
;       + END LOOP
; END REGION

; HIR after:

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      (%a)[i1] = 0;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         (%a)[i1] = 1;
; CHECK:       |         (%a)[%m] = 2;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         (%a)[i1] = -1;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiii(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp23 = icmp sgt i32 %n, 0
  br i1 %cmp23, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %idxprom6 = sext i32 %m to i64
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %idxprom6
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %i.024 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %0 = add nsw i64 %indvars.iv, %idxprom6
  %1 = icmp eq i64 %0, 2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 0, ptr %arrayidx
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.024, %m
  %cmp2 = icmp eq i32 %mul, 5
  br i1 %cmp2, label %if.then3, label %if.else8

if.then3:                                         ; preds = %if.else
  %arrayidx5 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 1, ptr %arrayidx5
  store i32 2, ptr %arrayidx7
  br label %for.inc

if.else8:                                         ; preds = %if.else
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 -1, ptr %arrayidx10
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else8, %if.then3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.024, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}