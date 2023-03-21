; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation sunk the stores of %a[i]
; because there is reference in the outer else, which makes the reference
; in the inner else unconditional. It was created from the following
; example in C++:

; int foo(int *a, int n, int m) {
;
;   int res = 0
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       a[i] = 0;
;     } else {
;       a[i] = 1;
;       if (m * i == 5) {
;         res += 2;
;       } else {
;         a[i] = 2;
;       }
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
;       |      (%a)[i1] = 0;
;       |   }
;       |   else
;       |   {
;       |      (%a)[i1] = 1;
;       |      if (%m * i1 == 5)
;       |      {
;       |         %res.018 = %res.018  +  2;
;       |      }
;       |      else
;       |      {
;       |         (%a)[i1] = 2;
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
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %cldst.sunk = 1;
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %res.018 = %res.018  +  2;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %cldst.sunk = 2;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   (%a)[i1] = %cldst.sunk;
; CHECK:       + END LOOP
; CHECK: END REGION



;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp17 = icmp sgt i32 %n, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

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
  %i.019 = phi i32 [ 0, %for.body.preheader ], [ %inc, %for.inc ]
  %res.018 = phi i32 [ 0, %for.body.preheader ], [ %res.1, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %2 = icmp eq i64 %1, 2
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 0, ptr %arrayidx
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 1, ptr %arrayidx5
  %mul = mul nsw i32 %i.019, %m
  %cmp2 = icmp eq i32 %mul, 5
  br i1 %cmp2, label %if.then3, label %if.else6

if.then3:                                         ; preds = %if.else
  %add5 = add nsw i32 %res.018, 2
  br label %for.inc

if.else6:                                         ; preds = %if.else
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 2, ptr %arrayidx7
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else6, %if.then3
  %res.1 = phi i32 [ %res.018, %if.then ], [ %add5, %if.then3 ], [ %res.018, %if.else6 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.019, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
