; RUN: opt -passes="hir-cg" -force-hir-cg -S < %s | FileCheck %s

; Check that GEP DDRefs with bitcast in the base are CG'd properly.
; TODO: Consider removing this lit-test as bitcast doesn't exists anymore.

; HIR-
; + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   %1 = (i32*)(@B)[0][i1];
; |   (i32*)(@A)[0][i1] = %1;
; + END LOOP


; CHECK: region.0:
; CHECK: [[B_ADDR:%.*]] = getelementptr inbounds [100 x float], ptr @B
; CHECK-NEXT: load i32, ptr [[B_ADDR]]

; CHECK: [[A_ADDR:%.*]] = getelementptr inbounds [100 x float], ptr @A
; CHECK: store i32 {{.*}} ptr [[A_ADDR]]

; ModuleID = 'float2.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = external global [100 x float], align 16
@A = external global [100 x float], align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n) {
entry:
  %cmp.1 = icmp sgt i32 %n, 0
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv
  store i32 %0, ptr %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp ne i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void

; uselistorder directives
  uselistorder i64 %indvars.iv, { 2, 1, 0 }
  uselistorder i64 0, { 1, 2, 0 }
  uselistorder label %for.body, { 1, 0 }
}
