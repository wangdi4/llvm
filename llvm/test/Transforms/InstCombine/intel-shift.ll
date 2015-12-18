; This test makes sure that this optimization is applied:
; ((X << C) + Y) >> C  ->  (X + (Y >> C)) & (~0 >> C) 
;
; RUN: opt < %s -instcombine -S | FileCheck %s

define i32 @test1(i32 %a, i32 %b) {             
; CHECK-LABEL: @test1(
; CHECK-NEXT: %c11 = lshr i32 %b, 10
; CHECK-NEXT: %c3 = add i32 %c11, %a
; CHECK-NEXT: and i32 %c3, 4194303
  %c = shl i32 %a, 10
  %c1 = add i32 %c, %b
  %c2 = lshr i32 %c1, 10 
  ret i32 %c2
}                                  
