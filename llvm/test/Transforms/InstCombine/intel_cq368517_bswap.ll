; Test that the bswap recognition and conversion works correctly.
;
; RUN: opt < %s -instcombine -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; inputs to OR are shifts.
; Function Attrs: nounwind uwtable
define i64 @_Z17byteswap_64b_logn_shiftsm(i64 %result) {
; CHECK: call i64 @llvm.bswap.i64(i64 %result)
  %1 = and i64 %result, -4294967296
  %2 = lshr i64 %1, 32
  %3 = and i64 %result, 4294967295
  %4 = shl i64 %3, 32
  %5 = or i64 %2, %4
  %6 = and i64 %5, -281470681808896
  %7 = lshr i64 %6, 16
  %8 = and i64 %5, 281470681808895
  %9 = shl i64 %8, 16
  %10 = or i64 %7, %9
  %11 = and i64 %10, -71777214294589696
  %12 = lshr i64 %11, 8
  %13 = and i64 %10, 71777214294589695
  %14 = shl i64 %13, 8
  %15 = or i64 %12, %14
  ret i64 %15
}

; inputs to OR are masks.
; Function Attrs: nounwind uwtable
define i64 @_Z23byteswap_64b_logn_masksm(i64 %result) #0 {
; CHECK: call i64 @llvm.bswap.i64(i64 %result)
  %1 = lshr i64 %result, 32
  %2 = and i64 %1, 4294967295
  %3 = shl i64 %result, 32
  %4 = and i64 %3, -4294967296
  %5 = or i64 %2, %4
  %6 = lshr i64 %5, 16
  %7 = and i64 %6, 281470681808895
  %8 = shl i64 %5, 16
  %9 = and i64 %8, -281470681808896
  %10 = or i64 %7, %9
  %11 = lshr i64 %10, 8
  %12 = and i64 %11, 71777214294589695
  %13 = shl i64 %10, 8
  %14 = and i64 %13, -71777214294589696
  %15 = or i64 %12, %14
  ret i64 %15
}

; 1 of the inputs to OR is OR
; Function Attrs: nounwind uwtable
define i64 @_Z14byteswap_64b_nm(i64 %result) {
; CHECK: call i64 @llvm.bswap.i64(i64 %result)
  %1 = and i64 %result, 255
  %2 = shl i64 %1, 56
  %3 = and i64 %result, -72057594037927936
  %4 = lshr i64 %3, 56
  %5 = or i64 %2, %4
  %6 = and i64 %result, 65280
  %7 = shl i64 %6, 40
  %8 = or i64 %5, %7
  %9 = and i64 %result, 71776119061217280
  %10 = lshr i64 %9, 40
  %11 = or i64 %8, %10
  %12 = and i64 %result, 16711680
  %13 = shl i64 %12, 24
  %14 = or i64 %11, %13
  %15 = and i64 %result, 280375465082880
  %16 = lshr i64 %15, 24
  %17 = or i64 %14, %16
  %18 = and i64 %result, 1095216660480
  %19 = lshr i64 %18, 8
  %20 = or i64 %17, %19
  %21 = and i64 %result, 4278190080
  %22 = shl i64 %21, 8
  %23 = or i64 %20, %22
  ret i64 %23
}

; THIS SHOULD NOT BE CONVERTED INTO BSWAP INTRINSICS. 
; as one of the constant is wrong. this is not a bswap
; operation.
; inputs to OR are masks.
; Function Attrs: nounwind uwtable
define i64 @_Z23not_byteswap_64b_logn_masksm(i64 %result) #0 {
; CHECK: and i64 %11, -61777214294589696 
  %1 = lshr i64 %result, 32
  %2 = and i64 %1, 4294967295
  %3 = shl i64 %result, 32
  %4 = and i64 %3, -4294967296
  %5 = or i64 %2, %4
  %6 = lshr i64 %5, 16
  %7 = and i64 %6, 281470681808895
  %8 = shl i64 %5, 16
  %9 = and i64 %8, -281470681808896
  %10 = or i64 %7, %9
  %11 = lshr i64 %10, 8
  %12 = and i64 %11, 71777214294589695 
  %13 = shl i64 %10, 8
  %14 = and i64 %13, -61777214294589696; -71777214294589696 completes a bswap. 
  %15 = or i64 %12, %14
  ret i64 %15
}


