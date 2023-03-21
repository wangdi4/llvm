; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation sinks the stores of %a[i] in
; the nested Ifs, but it doesn't transforms the store outside the condition.
; This is an artificial test case, in reality the compiler will remove the
; all the stores inside the condition since the store outside will overwrite
; the value. The purpose of this test is to make sure that the analysis process
; works as expected.


; HIR before transformation

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
;       |         (%a)[i1] = -1;
;       |      }
;       |      else
;       |      {
;       |         (%a)[i1] = 1;
;       |      }
;       |   }
;       |   (%a)[i1] = 10;
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      %cldst.sunk = 0;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %cldst.sunk = -1;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %cldst.sunk = 1;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   (%a)[i1] = %cldst.sunk;
; CHECK:       |   (%a)[i1] = 10;
; CHECK:       + END LOOP
; CHECK: END REGION


; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiii(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %m to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %i.020 = phi i32 [ 0, %for.body.preheader ], [ %inc, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %2 = icmp eq i64 %1, 2
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 0, ptr %arrayidx
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.020, %m
  %cmp2 = icmp eq i32 %mul, 5
  br i1 %cmp2, label %if.then3, label %if.else6

if.then3:                                         ; preds = %if.else
  %arrayidx5 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 -1, ptr %arrayidx5
  br label %for.inc

if.else6:                                         ; preds = %if.else
  %arrayidx8 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 1, ptr %arrayidx8
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else6, %if.then3
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 10, ptr %arrayidx10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.020, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
