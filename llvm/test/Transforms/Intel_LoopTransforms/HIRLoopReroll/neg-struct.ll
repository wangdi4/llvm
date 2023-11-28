; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Invalid to reroll due to different trailing offsets.

; CHECK: Function: Vsub

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, (sext.i32.i64(%m) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 1073741824>
; CHECK:              |   %mul1 = %mul  *  (%a)[2 * i1];
; CHECK:              |   (%b)[2 * i1].0 = %mul1;
; CHECK:              |   %mul7 = %mul  *  (%a)[2 * i1 + 1];
; CHECK:              |   (%b)[2 * i1 + 1].1 = %mul7;
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK: Function: Vsub

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, (sext.i32.i64(%m) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 1073741824>
; CHECK:              |   %mul1 = %mul  *  (%a)[2 * i1];
; CHECK:              |   (%b)[2 * i1].0 = %mul1;
; CHECK:              |   %mul7 = %mul  *  (%a)[2 * i1 + 1];
; CHECK:              |   (%b)[2 * i1 + 1].1 = %mul7;
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'struct.c'
source_filename = "struct.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { double, double }

; Function Attrs: norecurse nounwind uwtable
define dso_local void @Vsub(ptr nocapture readonly %a, ptr nocapture %b, double %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %m, 0
  br i1 %cmp23, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %mul = fmul double %n, %n
  %0 = sext i32 %m to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, ptr %a, i64 %indvars.iv
  %1 = load double, ptr %arrayidx, align 8
  %mul1 = fmul double %mul, %1
  %X = getelementptr inbounds %struct.S, ptr %b, i64 %indvars.iv, i32 0
  store double %mul1, ptr %X, align 8
  %2 = or i64 %indvars.iv, 1
  %arrayidx6 = getelementptr inbounds double, ptr %a, i64 %2
  %3 = load double, ptr %arrayidx6, align 8
  %mul7 = fmul double %mul, %3
  %Y = getelementptr inbounds %struct.S, ptr %b, i64 %2, i32 1
  store double %mul7, ptr %Y, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



