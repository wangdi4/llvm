; This testcase causes an assertion in inst combine where
; target instructions has vector types.
;
; This happens during optimizations:
; ((X << C) + Y) >> C  ->  (X + (Y >> C)) & (~0 >> C)
; (Y + (X << C)) >> C  ->  ((Y >> C) + X) & (~0 >> C)
;
; RUN: opt < %s -instcombine -S | FileCheck %s

define <2 x i64> @test1(<2 x i64> %X, <2 x i64> %Y) {             
  %x = mul <2 x i64> <i64 8, i64 8>, %X
  %y = add <2 x i64> %x, %Y
  %z = lshr <2 x i64> %y, <i64 3, i64 3>

;CHECK: %y1 = lshr <2 x i64> %Y, <i64 3, i64 3>
;CHECK: %x2 = add <2 x i64> %y1, %X
;CHECK: %z = and <2 x i64> %x2, <i64 2305843009213693951, i64 2305843009213693951>

  ret <2 x i64> %z
}

define <2 x i64> @test2(<2 x i64> %X, <2 x i64> %Y) {
  %x = mul <2 x i64> <i64 8, i64 8>, %X
  %y = add <2 x i64> %Y, %x
  %z = lshr <2 x i64> %y, <i64 3, i64 3>

;CHECK: %y1 = lshr <2 x i64> %Y, <i64 3, i64 3>
;CHECK: %x2 = add <2 x i64> %y1, %X
;CHECK: %z = and <2 x i64> %x2, <i64 2305843009213693951, i64 2305843009213693951>

  ret <2 x i64> %z
}

