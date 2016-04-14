; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the (for.body568) loop's upper is parsed correctly. The initial value for the loop upper is coming from unknown sibling loop(land.rhs532).
; CHECK: DO i1 = 0, %k.41017 + -1
; CHECK-NEXT: END LOOP

; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind
define void @DiffInt() {
entry:
  br i1 undef, label %land.lhs.true578, label %land.rhs532

land.rhs532:                                      ; preds = %for.body537, %entry
  %k.41017 = phi i32 [ %dec563, %for.body537 ], [ 10, %entry ]
  br i1 undef, label %for.body568.preheader, label %for.body537

for.body537:                                      ; preds = %land.rhs532
  %dec563 = add i32 %k.41017, -1
  br i1 undef, label %land.lhs.true578, label %land.rhs532

for.body568.preheader:                            ; preds = %land.rhs532
  br label %for.body568

for.body568:                                      ; preds = %for.body568, %for.body568.preheader
  %k.51013 = phi i32 [ %dec574, %for.body568 ], [ %k.41017, %for.body568.preheader ]
  %dec574 = add i32 %k.51013, -1
  %cmp566 = icmp eq i32 %dec574, 0
  br i1 %cmp566, label %for.end575, label %for.body568

for.end575:                                       ; preds = %for.body568
  br i1 undef, label %land.lhs.true578, label %cleanup

land.lhs.true578:                                 ; preds = %for.end575, %for.body537, %entry
  br label %cleanup

cleanup:                                          ; preds = %land.lhs.true578, %for.end575
  ret void
}

