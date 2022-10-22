; This test verifies that the CommuteCond transformation is NOT triggered for
; this test because the FalseValue of %sel1 is true.
; Verify that Condition and Truevalue operands of %sel1 are NOT swapped.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-commutecondop -dtrans-commute-cond-ignore-heuristic  -S 2>&1 | FileCheck %s

; CHECK: %sel1 = select i1 %cmp1, i1 %cmp2, i1 true
; CHECK-NOT: %sel1 = select i1 %cmp2, i1 %cmp1, i1 true

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @bea_is_dual_infeasible(i16 %ident, i64 %red_cost) {
entry:
  %cmp = icmp slt i64 %red_cost, 0
  br i1 %cmp, label %land.lhs.true, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %cmp1 = icmp ne i64 %red_cost, 0
  %cmp2 = icmp eq i16 %ident, 2
  %sel1 = select i1 %cmp1, i1 %cmp2, i1 true
  br i1 %sel1, label %l.end.true, label %l.end.false

land.lhs.true:                                    ; preds = %entry
  %cmp3 = icmp eq i16 %ident, 1
  br i1 %cmp3, label %l.end.true, label %l.end.false

l.end.true:                              ; preds = %lor.rhs, %land.lhs.true
  ret i32 1

l.end.false:                             ; preds = %lor.rhs, %land.lhs.true
  ret i32 0
}

!intel.dtrans.types = !{}
