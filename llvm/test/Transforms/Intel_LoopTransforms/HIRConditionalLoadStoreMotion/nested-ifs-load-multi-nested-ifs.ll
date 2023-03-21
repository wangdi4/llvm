; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation hoist the loads of %a[i] in
; the nested Ifs. It was created from the following example in C++:

; int foo(int *a, int n, int m) {
; 
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2)
;       res += (0 + a[i]);
;     else if (m * i == 5)
;       res += (-1 + a[i]);
;     else if (m / i == 3)
;       res += (2 * a[i]);
;     else
;       res += (1 + a[i]);
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
;       |         %4 = (%a)[i1];
;       |         %.pn = %4 + -1;
;       |      }
;       |      else
;       |      {
;       |         %div = %m  /  i1;
;       |         if (%div == 3)
;       |         {
;       |            %6 = (%a)[i1];
;       |            %.pn = 2 * %6;
;       |         }
;       |         else
;       |         {
;       |            %7 = (%a)[i1];
;       |            %.pn = %7 + 1;
;       |         }
;       |      }
;       |   }
;       |   %res.042 = %.pn  +  %res.042;
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
; CHECK:       |         %4 = %cldst.hoisted;
; CHECK:       |         %.pn = %4 + -1;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %div = %m  /  i1;
; CHECK:       |         if (%div == 3)
; CHECK:       |         {
; CHECK:       |            %6 = %cldst.hoisted;
; CHECK:       |            %.pn = 2 * %6;
; CHECK:       |         }
; CHECK:       |         else
; CHECK:       |         {
; CHECK:       |            %7 = %cldst.hoisted;
; CHECK:       |            %.pn = %7 + 1;
; CHECK:       |         }
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   %res.042 = %.pn  +  %res.042;
; CHECK:       + END LOOP
; CHECK: END REGION

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp41 = icmp sgt i32 %n, 0
  br i1 %cmp41, label %for.body.preheader, label %for.cond.cleanup

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
  %i.043 = phi i32 [ 0, %for.body.preheader ], [ %inc, %for.inc ]
  %res.042 = phi i32 [ 0, %for.body.preheader ], [ %res.1, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %2 = icmp eq i64 %1, 2
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.043, %m
  %cmp4 = icmp eq i32 %mul, 5
  br i1 %cmp4, label %if.then5, label %if.else10

if.then5:                                         ; preds = %if.else
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx7
  %add8 = add nsw i32 %4, -1
  br label %for.inc

if.else10:                                        ; preds = %if.else
  %5 = trunc i64 %indvars.iv to i32
  %div = sdiv i32 %m, %5
  %cmp11 = icmp eq i32 %div, 3
  br i1 %cmp11, label %if.then12, label %if.else17

if.then12:                                        ; preds = %if.else10
  %arrayidx14 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %6 = load i32, ptr %arrayidx14
  %mul15 = shl nsw i32 %6, 1
  br label %for.inc

if.else17:                                        ; preds = %if.else10
  %arrayidx19 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %7 = load i32, ptr %arrayidx19
  %add20 = add nsw i32 %7, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.then12, %if.else17, %if.then5
  %.pn = phi i32 [ %3, %if.then ], [ %add8, %if.then5 ], [ %mul15, %if.then12 ], [ %add20, %if.else17 ]
  %res.1 = add nsw i32 %.pn, %res.042
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.043, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}