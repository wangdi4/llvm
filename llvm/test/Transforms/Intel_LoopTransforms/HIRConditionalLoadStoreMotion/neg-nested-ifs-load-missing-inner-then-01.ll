; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output -debug-only=hir-cond-ldst-motion < %s 2>&1 | FileCheck %s

; This test case checks that the transformation didn't hoist the loads of %a[i]
; in the nested Ifs because the reference to %a[i] wasn't found in all branches.
; It was created from the following example in C++:

; int foo(int *a, int n, int m) {
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       res += a[i];
;     } else if (m * i == 5) {
;       res += 2 + a[m];
;     } else {
;       res += 1 + a[i];
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
;       |         %3 = (%a)[%m];
;       |         %.pn = %3 + 2;
;       |      }
;       |      else
;       |      {
;       |         %4 = (%a)[i1];
;       |         %.pn = %4 + 1;
;       |      }
;       |   }
;       |   %res.028 = %.pn  +  %res.028;
;       + END LOOP
; END REGION

; Check the debug information

; CHECK: Removing conditional set: (<9>(%a)[i1] | <25>(%a)[i1])

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      %.pn = (%a)[i1];
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %3 = (%a)[%m];
; CHECK:       |         %.pn = %3 + 2;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %4 = (%a)[i1];
; CHECK:       |         %.pn = %4 + 1;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   %res.028 = %.pn  +  %res.028;
; CHECK:       + END LOOP
; CHECK: END REGION

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp27 = icmp sgt i32 %n, 0
  br i1 %cmp27, label %for.body.lr.ph, label %for.cond.cleanup

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
  %i.029 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %res.028 = phi i32 [ 0, %for.body.lr.ph ], [ %res.1, %for.inc ]
  %0 = add nsw i64 %indvars.iv, %idxprom5
  %1 = icmp eq i64 %0, 2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.029, %m
  %cmp3 = icmp eq i32 %mul, 5
  br i1 %cmp3, label %if.then4, label %if.else9

if.then4:                                         ; preds = %if.else
  %3 = load i32, ptr %arrayidx6
  %add7 = add nsw i32 %3, 2
  br label %for.inc

if.else9:                                         ; preds = %if.else
  %arrayidx11 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx11
  %add12 = add nsw i32 %4, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else9, %if.then4
  %.pn = phi i32 [ %2, %if.then ], [ %add7, %if.then4 ], [ %add12, %if.else9 ]
  %res.1 = add nsw i32 %.pn, %res.028
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.029, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}