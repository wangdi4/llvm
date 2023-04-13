; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that creating the stores for sinking doesn't produce
; a backward dependency. It was created from the following C++ code:

; void foo(int *a, int n, int m) {
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       a[i] = 0;
;       a[i + 3] = 5;
;       a[i + 1] = 2;
;     } else {
;       a[i + 1] = 4;
;       a[i + 3] = 9;
;       a[i] = 1;
;       if (m * i == 40) {
;         a[i + 3] = 7;
;         a[i] = -1;
;         a[i + 1] = 3;
;       }
;     }
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (i1 + sext.i32.i64(%m) == 2)
;       |   {
;       |      (%a)[i1] = 0;
;       |      (%a)[i1 + 3] = 5;
;       |      (%a)[i1 + 1] = 2;
;       |   }
;       |   else
;       |   {
;       |      (%a)[i1 + 1] = 4;
;       |      (%a)[i1 + 3] = 9;
;       |      (%a)[i1] = 1;
;       |      if (%m * i1 == 40)
;       |      {
;       |         (%a)[i1 + 3] = 7;
;       |         (%a)[i1] = -1;
;       |         (%a)[i1 + 1] = 3;
;       |      }
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      %cldst.sunk = 0;
; CHECK:       |      %cldst.sunk9 = 5;
; CHECK:       |      %cldst.sunk5 = 2;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %cldst.sunk5 = 4;
; CHECK:       |      %cldst.sunk9 = 9;
; CHECK:       |      %cldst.sunk = 1;
; CHECK:       |      if (%m * i1 == 40)
; CHECK:       |      {
; CHECK:       |         %cldst.sunk9 = 7;
; CHECK:       |         %cldst.sunk = -1;
; CHECK:       |         %cldst.sunk5 = 3;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   (%a)[i1 + 3] = %cldst.sunk9;
; CHECK:       |   (%a)[i1 + 1] = %cldst.sunk5;
; CHECK:       |   (%a)[i1] = %cldst.sunk;
; CHECK:       + END LOOP
; CHECK: END REGION


; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z3fooPiii(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp48 = icmp sgt i32 %n, 0
  br i1 %cmp48, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %m to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next.pre-phi, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %2 = icmp eq i64 %1, 2
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 0, ptr %arrayidx
  %3 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx4 = getelementptr inbounds i32, ptr %a, i64 %3
  store i32 5, ptr %arrayidx4
  %4 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %4
  store i32 2, ptr %arrayidx7
  br label %for.inc

if.else:                                          ; preds = %for.body
  %5 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 %5
  store i32 4, ptr %arrayidx10
  %6 = add i64 %indvars.iv, 3
  %idxprom12 = and i64 %6, 4294967295
  %arrayidx13 = getelementptr inbounds i32, ptr %a, i64 %idxprom12
  store i32 9, ptr %arrayidx13
  %arrayidx15 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 1, ptr %arrayidx15
  %7 = trunc i64 %indvars.iv to i32
  %8 = mul i32 %7, %m
  %cmp16 = icmp eq i32 %8, 40
  br i1 %cmp16, label %if.then17, label %for.inc

if.then17:                                        ; preds = %if.else
  store i32 7, ptr %arrayidx13
  store i32 -1, ptr %arrayidx15
  store i32 3, ptr %arrayidx10
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.then17, %if.else
  %indvars.iv.next.pre-phi = phi i64 [ %4, %if.then ], [ %5, %if.then17 ], [ %5, %if.else ]
  %exitcond.not = icmp eq i64 %indvars.iv.next.pre-phi, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}