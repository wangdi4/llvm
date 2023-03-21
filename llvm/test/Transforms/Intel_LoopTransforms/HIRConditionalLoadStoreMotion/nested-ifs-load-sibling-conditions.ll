; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the optimization hoists the loads in sibling
; conditionals.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (i1 + sext.i32.i64(%m) < 4)
;       |   {
;       |      %.pn = (%a)[i1];
;       |   }
;       |   else
;       |   {
;       |      if (%m * i1 == 5)
;       |      {
;       |         %3 = (%a)[i1];
;       |         %.pnt = %3 + 2;
;       |      }
;       |      else
;       |      {
;       |         %4 = (%a)[i1];
;       |         %.pnt = %4 + 3;
;       |      }
;       |      if (%m * i1 == 10)
;       |      {
;       |         %6 = (%a)[i1];
;       |         %.pn = %6 + %.pnt + 1;
;       |      }
;       |      else
;       |      {
;       |         %7 = (%a)[i1];
;       |         %.pn = %7 + %.pnt + 10;
;       |      }
;       |   }
;       |   %res.042 = %.pn  +  %res.042;
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %cldst.hoisted = (%a)[i1];
; CHECK:       |   if (i1 + sext.i32.i64(%m) < 4)
; CHECK:       |   {
; CHECK:       |      %.pn = %cldst.hoisted;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %3 = %cldst.hoisted;
; CHECK:       |         %.pnt = %3 + 2;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %4 = %cldst.hoisted;
; CHECK:       |         %.pnt = %4 + 3;
; CHECK:       |      }
; CHECK:       |      if (%m * i1 == 10)
; CHECK:       |      {
; CHECK:       |         %6 = %cldst.hoisted;
; CHECK:       |         %.pn = %6 + %.pnt + 1;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %7 = %cldst.hoisted;
; CHECK:       |         %.pn = %7 + %.pnt + 10;
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
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %cmp41 = icmp sgt i32 %n, 0
  br i1 %cmp41, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               
  %0 = sext i32 %m to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        
  %res.1.lcssa = phi i32 [ %res.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %i.043 = phi i32 [ 0, %for.body.preheader ], [ %inc, %for.inc ]
  %res.042 = phi i32 [ 0, %for.body.preheader ], [ %res.1, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %cmp1 = icmp slt i64 %1, 4
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx
  br label %for.inc

if.else:
  %mul = mul nsw i32 %i.043, %m
  %cmp11 = icmp eq i32 %mul, 5
  br i1 %cmp11, label %if.then12, label %if.else12

if.then12:                                        
  %arrayidx14 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx14
  %add15 = add nsw i32 %3, 2
  br label %end.cmp

if.else12:                                          
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx7
  %add8 = add nsw i32 %4, 3
  br label %end.cmp

end.cmp:
  %.pnt = phi i32 [ %add15, %if.then12 ], [ %add8, %if.else12 ]
  %5 = icmp eq i32 %mul, 10
  br i1 %5, label %if.then17, label %if.else17

if.then17:                                        
  %arrayidx19 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %6 = load i32, ptr %arrayidx19
  %add20 = add nsw i32 %6, 1
  %add21 = add nsw i32 %add20, %.pnt
  br label %for.inc

if.else17:                                        
  %arrayidx20 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %7 = load i32, ptr %arrayidx20
  %add22 = add nsw i32 %7, 10
  %add23 = add nsw i32 %add22, %.pnt
  br label %for.inc

for.inc:                                          
  %.pn = phi i32 [ %2, %if.then ], [ %add21, %if.then17 ], [ %add23, %if.else17 ]
  %res.1 = add nsw i32 %.pn, %res.042
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.043, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}