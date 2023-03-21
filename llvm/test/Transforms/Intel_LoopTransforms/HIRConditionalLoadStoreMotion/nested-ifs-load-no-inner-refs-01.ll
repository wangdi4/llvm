; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation hoist the loads of %a[i] in
; the nested Ifs. The goal of this test case is to check that the
; transformation works since the reference to %a[i] isn't used inside inner
; conditional. It was created from the following example in C++:

; int foo(int *a, int n, int m) {
; 
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       res += (a[i] + 7);
;     } else {
;       res += (a[i] + m);
;       if (m * i == 5)
;         res += (-3);
;       else
;         res += (1);
;     }
;   }
; 
;   return res;
; }

; HIR before transformation

;  BEGIN REGION { }
;        + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;        |   if (i1 + sext.i32.i64(%m) == 2)
;        |   {
;        |      %3 = (%a)[i1];
;        |      %.pn = %3 + 7;
;        |   }
;        |   else
;        |   {
;        |      %4 = (%a)[i1];
;        |      if (%m * i1 == 5)
;        |      {
;        |         %.pn = %m + %4 + -3;
;        |      }
;        |      else
;        |      {
;        |         %.pn = %m + %4 + 1;
;        |      }
;        |   }
;        |   %res.029 = %.pn  +  %res.029;
;        + END LOOP
;  END REGION


; HIR after transformation

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:        |   %cldst.hoisted = (%a)[i1];
; CHECK:        |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:        |   {
; CHECK:        |      %3 = %cldst.hoisted;
; CHECK:        |      %.pn = %3 + 7;
; CHECK:        |   }
; CHECK:        |   else
; CHECK:        |   {
; CHECK:        |      %4 = %cldst.hoisted;
; CHECK:        |      if (%m * i1 == 5)
; CHECK:        |      {
; CHECK:        |         %.pn = %m + %4 + -3;
; CHECK:        |      }
; CHECK:        |      else
; CHECK:        |      {
; CHECK:        |         %.pn = %m + %4 + 1;
; CHECK:        |      }
; CHECK:        |   }
; CHECK:        |   %res.029 = %.pn  +  %res.029;
; CHECK:        + END LOOP
; CHECK:  END REGION


; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp28 = icmp sgt i32 %n, 0
  br i1 %cmp28, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %m to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %res.1.lcssa = phi i32 [ %res.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %i.030 = phi i32 [ 0, %for.body.preheader ], [ %inc, %for.inc ]
  %res.029 = phi i32 [ 0, %for.body.preheader ], [ %res.1, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %2 = icmp eq i64 %1, 2
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx
  %add1 = add nsw i32 %3, 7
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx7
  %add8 = add nsw i32 %4, %m
  %mul = mul nsw i32 %i.030, %m
  %cmp4 = icmp eq i32 %mul, 5
  br i1 %cmp4, label %if.then5, label %if.else10

if.then5:                                         ; preds = %if.else
  %add9 = add nsw i32 %add8, -3
  br label %for.inc

if.else10:                                        ; preds = %if.else
  %add13 = add nsw i32 %add8, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else10, %if.then5
  %.pn = phi i32 [ %add1, %if.then ], [ %add9, %if.then5 ], [ %add13, %if.else10 ]
  %res.1 = add nsw i32 %.pn, %res.029
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.030, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}