; This testcase causes an assertion in inst combine where
; target instructions has vector types.
;
; This happens during optimizations:
; ((X << C) + Y) >> C  ->  (X + (Y >> C)) & (~0 >> C)
; (Y + (X << C)) >> C  ->  ((Y >> C) + X) & (~0 >> C)
;
; RUN: opt < %s -instcombine -S | FileCheck %s

define <2 x i64> @test1(i64 %X, i64 %Y) {             
  %X_v1 = insertelement <2 x i64> undef, i64 %X, i32 0
  %X_v2 = shufflevector <2 x i64> %X_v1, <2 x i64> undef, <2 x i32> zeroinitializer

  %Y_v1 = insertelement <2 x i64> undef, i64 %Y, i32 0
  %Y_v2 = shufflevector <2 x i64> %Y_v1, <2 x i64> undef, <2 x i32> zeroinitializer

  %x = mul <2 x i64> <i64 8, i64 8>, %X_v2
  %y = add <2 x i64> %x, %Y_v2
  %z = lshr <2 x i64> %y, <i64 3, i64 3>

;CHECK: %y1 = lshr <2 x i64> %Y_v2, <i64 3, i64 3>
;CHECK: %x2 = add <2 x i64> %X_v2, %y1
;CHECK: %z = and <2 x i64> %x2, <i64 2305843009213693951, i64 2305843009213693951>

  ret <2 x i64> %z
}

define <2 x i64> @test2(i64 %X, i64 %Y) {
  %X_v1 = insertelement <2 x i64> undef, i64 %X, i32 0
  %X_v2 = shufflevector <2 x i64> %X_v1, <2 x i64> undef, <2 x i32> zeroinitializer

  %Y_v1 = insertelement <2 x i64> undef, i64 %Y, i32 0
  %Y_v2 = shufflevector <2 x i64> %Y_v1, <2 x i64> undef, <2 x i32> zeroinitializer

  %x = mul <2 x i64> <i64 8, i64 8>, %X_v2
  %y = add <2 x i64> %Y_v2, %x
  %z = lshr <2 x i64> %y, <i64 3, i64 3>

;CHECK: %y1 = lshr <2 x i64> %Y_v2, <i64 3, i64 3>
;CHECK: %Y_v22 = add <2 x i64> %y1, %X_v2
;CHECK: %z = and <2 x i64> %Y_v22, <i64 2305843009213693951, i64 2305843009213693951>

  ret <2 x i64> %z
}

