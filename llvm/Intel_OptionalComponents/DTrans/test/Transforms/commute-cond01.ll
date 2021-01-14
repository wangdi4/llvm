; This test verifies that CommuteCond transformation is triggered for
; this test when -dtrans-commute-cond-ignore-heuristic is passed.
; Verified that the operands of %and1 are swapped.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-commutecond -dtrans-commute-cond-ignore-heuristic -S 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-commutecond -dtrans-commute-cond-ignore-heuristic  -S 2>&1 | FileCheck %s

; CHECK: %and1 = and i1 %cmp2, %cmp1
; CHECK-NOT: %and1 = and i1 %cmp1, %cmp2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @bea_is_dual_infeasible(i16 %ident, i64 %red_cost) {
entry:
  %cmp = icmp slt i64 %red_cost, 0
  br i1 %cmp, label %land.lhs.true, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %cmp1 = icmp ne i64 %red_cost, 0
  %cmp2 = icmp eq i16 %ident, 2
  %and1 = and i1 %cmp1, %cmp2
  br i1 %and1, label %l.end.true, label %l.end.false

land.lhs.true:                                    ; preds = %entry
  %cmp3 = icmp eq i16 %ident, 1
  br i1 %cmp3, label %l.end.true, label %l.end.false

l.end.true:                              ; preds = %lor.rhs, %land.lhs.true
  ret i32 1

l.end.false:                             ; preds = %lor.rhs, %land.lhs.true
  ret i32 0
}
