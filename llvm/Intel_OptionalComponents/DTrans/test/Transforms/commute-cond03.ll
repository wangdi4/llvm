; UNSUPPORTED: enable-opaque-pointers

; This test verifies that CommuteCond transformation is triggered
; using DTrans heuristic, which checks that %ident is a field of a struct
; and number of possible constant values for the field is less than 4.
; Verified that operands of %and1 are swapped by the transformation.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-commutecond 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-commutecond 2>&1 | FileCheck %s

; CHECK: define i32 @proc2
; CHECK: %and1 = and i1 %cmp2, %cmp1
; CHECK-NOT: %and1 = and i1 %cmp1, %cmp2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

define i32 @main() {
entry:
  call void @proc1();
  ret i32 0
}

define i32 @proc2(%struct.test.01* %arc, i64 %red_cost) {
entry:
  %F4 = getelementptr %struct.test.01, %struct.test.01* %arc, i32 0, i32 4
  %ident = load i16, i16* %F4
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

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F4 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 4
  store i16 0, i16* %F4
  %S1 = select i1 undef, i16 1, i16 2
  store i16 %S1, i16* %F4
  %call2 = call i32 @proc2(%struct.test.01* %tp2, i64 20);
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
