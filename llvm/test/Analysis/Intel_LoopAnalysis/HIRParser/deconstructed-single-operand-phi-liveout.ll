; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that deconstructed single operand phi %.lcssa is marked liveout of i2 loop.

; HIR-
; + DO i1 = 0, (-1 * %g.027.ph + umax(9, (2 + %g.027.ph)) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 6>
; |   + DO i2 = 0, 2, 1   <DO_LOOP>
; |   |   (%t1)[0] = 0;
; |   |   (@a)[0] = 1;
; |   |   %.lcssa = 1;
; |   + END LOOP
; + END LOOP


; CHECK: DO i32 i1
; CHECK: LiveOut symbases: [[LIVEOUTSB:[0-9]+]]
; CHECK: DO i32 i2

; CHECK: %.lcssa = 1;
; CHECK: <LVAL-REG> i32 1 {sb:[[LIVEOUTSB]]}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global i32 0, align 4

define dso_local void @e(i32 %g.027.ph, ptr %t1) {
entry:
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %entry, %for.end9
  %g.124 = phi i32 [ %add11, %for.end9 ], [ %g.027.ph, %entry ]
  br label %for.body8

for.body8:                                        ; preds = %for.cond6.preheader, %for.body8
  %f.222 = phi i32 [ 0, %for.cond6.preheader ], [ %add, %for.body8 ]
  store i32 0, ptr %t1, align 4
  store i32 1, ptr @a, align 4
  %add = add nuw nsw i32 %f.222, 3
  %cmp7 = icmp ult i32 %add, 8
  br i1 %cmp7, label %for.body8, label %for.end9

for.end9:                                         ; preds = %for.body8
  %.lcssa = phi i32 [ 1, %for.body8 ]
  %add11 = add nuw i32 %g.124, 2
  %cmp4 = icmp ult i32 %add11, 9
  br i1 %cmp4, label %for.cond6.preheader, label %for.cond.loopexit.thread

for.cond.loopexit.thread:                         ; preds = %for.end9
  %.lcssa.lcssa = phi i32 [ %.lcssa, %for.end9 ]
  ret void
}

