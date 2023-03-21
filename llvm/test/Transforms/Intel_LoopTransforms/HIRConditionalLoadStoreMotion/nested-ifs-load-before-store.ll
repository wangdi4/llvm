; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation hoisted the loads of %a[i]
; outside the nested Ifs since the store to %a[m] happens after the load in one
; of the branches. It was created from the following example in C++:

; int foo(int *a, int n, int m) {
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       res += a[i];
;     } else if (m * i == 5) {
;       res += a[i] + 1;
;       a[m] = 0;
;     } else {
;       res += a[i] + 2;
;     }
;   }
; 
;   return res;
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (i1 + sext.i32.i64(%m) == 2)
;       |   {
;       |      %.pn = (%a)[i1];
;       |   }
;       |   else
;       |   {
;       |      if (%m * i1 == 5)
;       |      {
;       |         %3 = (%a)[i1];
;       |         (%a)[%m] = 0;
;       |         %.pn = %3 + 1;
;       |      }
;       |      else
;       |      {
;       |         %4 = (%a)[i1];
;       |         %.pn = %4 + 2;
;       |      }
;       |   }
;       |   %res.032 = %.pn  +  %res.032;
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %cldst.hoisted = (%a)[i1];
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      %.pn = %cldst.hoisted;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %3 = %cldst.hoisted;
; CHECK:       |         (%a)[%m] = 0;
; CHECK:       |         %.pn = %3 + 1;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %4 = %cldst.hoisted;
; CHECK:       |         %.pn = %4 + 2;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   %res.032 = %.pn  +  %res.032;
; CHECK:       + END LOOP
; CHECK: END REGION



; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp31 = icmp sgt i32 %n, 0
  br i1 %cmp31, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %idxprom5 = sext i32 %m to i64
  %arrayidx6 = getelementptr inbounds i32, ptr %a, i64 %idxprom5
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %res.1.lcssa = phi i32 [ %res.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %i.033 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %res.032 = phi i32 [ 0, %for.body.lr.ph ], [ %res.1, %for.inc ]
  %0 = add nsw i64 %indvars.iv, %idxprom5
  %1 = icmp eq i64 %0, 2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.033, %m
  %cmp3 = icmp eq i32 %mul, 5
  br i1 %cmp3, label %if.then4, label %if.else11

if.then4:                                         ; preds = %if.else
  %arrayidx8 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx8, align 4
  store i32 0, ptr %arrayidx6, align 4
  %add9 = add nsw i32 %3, 1
  br label %for.inc

if.else11:                                        ; preds = %if.else
  %arrayidx13 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx13, align 4
  %add14 = add nsw i32 %4, 2
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else11, %if.then4
  %.pn = phi i32 [ %2, %if.then ], [ %add9, %if.then4 ], [ %add14, %if.else11 ]
  %res.1 = add nsw i32 %.pn, %res.032
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.033, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
