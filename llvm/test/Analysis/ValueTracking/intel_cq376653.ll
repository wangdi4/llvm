; RUN: opt -passes="instcombine" < %s -S | FileCheck %s
; 
; This test checks to make sure that value tracking can tell that
; (n * (n [+-] 1)) is always non-negative.  It checks for a number
; of permutations of this pattern.  It checks to be sure the signed
; division can be understood to be implementable by a single logical
; shift right by 1, which can only occur if the expression being divided
; is known to be non-negative.
;
; The same analysis applies whether the adds are nsw or not, so long as the
; multiply is nsw, so for this test, none of the adds are marked nsw.

define i32 @tst1(i32 %n) {
; CHECK-LABEL: @tst1(
  %1 = add i32 %n, 1
  %2 = mul nsw i32 %n, %1
  %3 = sdiv i32 %2, 2
; CHECK: %3 = lshr i32 %2, 1
  ret i32 %3
}

define i32 @tst2(i32 %n) {
; CHECK-LABEL: @tst2(
  %1 = add i32 %n, -1
  %2 = mul nsw i32 %n, %1
  %3 = sdiv i32 %2, 2
; CHECK: %3 = lshr i32 %2, 1
  ret i32 %3
}

define i32 @tst3(i32 %n) {
; CHECK-LABEL: @tst3(
  %1 = add i32 %n, 1
  %2 = mul nsw i32 %1, %n
  %3 = sdiv i32 %2, 2
; CHECK: %3 = lshr i32 %2, 1
  ret i32 %3
}

define i32 @tst4(i32 %n) {
; CHECK-LABEL: @tst4(
  %1 = add i32 %n, -1
  %2 = mul nsw i32 %1, %n
  %3 = sdiv i32 %2, 2
; CHECK: %3 = lshr i32 %2, 1
  ret i32 %3
}
