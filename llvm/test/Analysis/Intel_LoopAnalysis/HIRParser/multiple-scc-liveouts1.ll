; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that we do not construct SCC (%mul26.lcssa.lcssa99 -> %inc -> %mul26.lcssa.lcssa98) containing multiple liveout values.

; CHECK: LiveOuts
; CHECK-DAG: mul26.lcssa.lcssa98
; CHECK-DAG: inc

; CHECK: DO i1 = 0, 58
; CHECK-NEXT: %inc = %mul26.lcssa.lcssa99  +  1;
; CHECK-NEXT: %mul26.lcssa.lcssa98 = %mul26.lcssa.lcssa99 + 1;
; CHECK: %mul26.lcssa95.in2 = %mul26.lcssa.lcssa99 + 1;
; CHECK-NEXT: DO i2 = 0, i1 + -49
; CHECK-NEXT: %mul2692 = %mul26.lcssa95.in2;
; CHECK-NEXT: DO i3 = 0, 9
; CHECK-NEXT: %0 = (%g)[0][i3 + 1];
; CHECK-NEXT: %mul2692 = %mul2692  *  %0;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: %mul26.lcssa95.in2 = %mul2692;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: (%j0)[0] = 11;
; CHECK-NEXT: %mul26.lcssa.lcssa98 = %mul2692;
; CHECK: %mul26.lcssa.lcssa99 = %mul26.lcssa.lcssa98;
; CHECK-NEXT: END LOOP


;Module Before HIR; ModuleID = 'tester2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main(i32 %add17) {
entry:
  %j0 = alloca i32, align 4
  %g = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc.31
  %dec32102 = phi i32 [ 60, %entry ], [ %dec32, %for.inc.31 ]
  %mul26.lcssa.lcssa99 = phi i32 [ %add17, %entry ], [ %mul26.lcssa.lcssa98, %for.inc.31 ]
  %inc = add i32 %mul26.lcssa.lcssa99, 1
  %cmp20.91 = icmp ult i32 %dec32102, 12
  br i1 %cmp20.91, label %for.cond.22.preheader.preheader, label %for.inc.31

for.cond.22.preheader.preheader:                  ; preds = %for.body
  br label %for.cond.22.preheader

for.cond.22.preheader:                            ; preds = %for.cond.22.preheader.preheader, %for.inc.28
  %inc2997 = phi i32 [ %inc29, %for.inc.28 ], [ %dec32102, %for.cond.22.preheader.preheader ]
  %mul26.lcssa95 = phi i32 [ %mul26.lcssa, %for.inc.28 ], [ %inc, %for.cond.22.preheader.preheader ]
  br label %for.body.24

for.body.24:                                      ; preds = %for.body.24, %for.cond.22.preheader
  %indvars.iv = phi i64 [ 1, %for.cond.22.preheader ], [ %indvars.iv.next, %for.body.24 ]
  %mul2692 = phi i32 [ %mul26.lcssa95, %for.cond.22.preheader ], [ %mul26, %for.body.24 ]
  %arrayidx25 = getelementptr inbounds [100 x i32], [100 x i32]* %g, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx25, align 4
  %mul26 = mul i32 %mul2692, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.inc.28, label %for.body.24

for.inc.28:                                       ; preds = %for.body.24
  %mul26.lcssa = phi i32 [ %mul26, %for.body.24 ]
  %inc29 = add nuw nsw i32 %inc2997, 1
  %exitcond103 = icmp eq i32 %inc29, 12
  br i1 %exitcond103, label %for.cond.19.for.inc.31_crit_edge, label %for.cond.22.preheader

for.cond.19.for.inc.31_crit_edge:                 ; preds = %for.inc.28
  %mul26.lcssa.lcssa = phi i32 [ %mul26.lcssa, %for.inc.28 ]
  store i32 11, i32* %j0, align 4
  br label %for.inc.31

for.inc.31:                                       ; preds = %for.cond.19.for.inc.31_crit_edge, %for.body
  %mul26.lcssa.lcssa98 = phi i32 [ %mul26.lcssa.lcssa, %for.cond.19.for.inc.31_crit_edge ], [ %inc, %for.body ]
  %dec32 = add nsw i32 %dec32102, -1
  %cmp18 = icmp ugt i32 %dec32, 1
  br i1 %cmp18, label %for.body, label %if.end.34.loopexit

if.end.34.loopexit:                               ; preds = %for.inc.31
  %mul26.lcssa.lcssa98.lcssa = phi i32 [ %mul26.lcssa.lcssa98, %for.inc.31 ]
  %inc.lcssa = phi i32 [ %inc, %for.inc.31 ]
  ret i32 0
}

