; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; Verify no reroll happens. %add (non reduction var) is live out of Loop.


; CHECK:Function: foo

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK:               |   %add1833 = (@c)[0][i1];
; CHECK:               |
; CHECK:               |   + DO i2 = 0, 199, 1   <DO_LOOP>
; CHECK:               |   |   %mul = (@y)[0][2 * i2]  *  (@x)[0][2 * i2];
; CHECK:               |   |   %add = %add1833  +  %mul;
; CHECK:               |   |   %mul15 = (@y)[0][2 * i2 + 1]  *  (@x)[0][2 * i2 + 1];
; CHECK:               |   |   %add1833 = %add  +  %mul15;
; CHECK:               |   + END LOOP
; CHECK:               |
; CHECK:               |   (@c)[0][i1] = %add;
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK:Function: foo

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK:               |   %add1833 = (@c)[0][i1];
; CHECK:               |
; CHECK:               |   + DO i2 = 0, 199, 1   <DO_LOOP>
; CHECK:               |   |   %mul = (@y)[0][2 * i2]  *  (@x)[0][2 * i2];
; CHECK:               |   |   %add = %add1833  +  %mul;
; CHECK:               |   |   %mul15 = (@y)[0][2 * i2 + 1]  *  (@x)[0][2 * i2 + 1];
; CHECK:               |   |   %add1833 = %add  +  %mul15;
; CHECK:               |   + END LOOP
; CHECK:               |
; CHECK:               |   (@c)[0][i1] = %add;
; CHECK:               + END LOOP
; CHECK:         END REGION


;Module Before HIR
; ModuleID = 'daxpy-red.c'
source_filename = "daxpy-red.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = common dso_local local_unnamed_addr global [10 x double] zeroinitializer, align 16
@x = common dso_local local_unnamed_addr global [10 x double] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [10 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(double %a, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp34 = icmp sgt i32 %n, 0
  br i1 %cmp34, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %for.cond1.preheader.preheader
  %indvars.iv37 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next38, %for.cond.cleanup3 ]
  %arrayidx8 = getelementptr inbounds [10 x double], ptr @c, i64 0, i64 %indvars.iv37
  %arrayidx8.promoted = load double, ptr %arrayidx8, align 8
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %add18.lcssa = phi double [ %add, %for.body4 ]
  store double %add18.lcssa, ptr %arrayidx8, align 8
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond = icmp eq i64 %indvars.iv.next38, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %add1833 = phi double [ %arrayidx8.promoted, %for.cond1.preheader ], [ %add18, %for.body4 ]
  %arrayidx = getelementptr inbounds [10 x double], ptr @y, i64 0, i64 %indvars.iv
  %0 = load double, ptr %arrayidx, align 16
  %arrayidx6 = getelementptr inbounds [10 x double], ptr @x, i64 0, i64 %indvars.iv
  %1 = load double, ptr %arrayidx6, align 16
  %mul = fmul double %0, %1
  %add = fadd double %add1833, %mul
  %2 = or i64 %indvars.iv, 1
  %arrayidx11 = getelementptr inbounds [10 x double], ptr @y, i64 0, i64 %2
  %3 = load double, ptr %arrayidx11, align 8
  %arrayidx14 = getelementptr inbounds [10 x double], ptr @x, i64 0, i64 %2
  %4 = load double, ptr %arrayidx14, align 8
  %mul15 = fmul double %3, %4
  %add18 = fadd double %add, %mul15
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp2 = icmp ult i64 %indvars.iv.next, 400
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3
}



