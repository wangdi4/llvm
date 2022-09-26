; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt < %s -enable-npm-dtrans -passes='lto-pre-link<O3>' -S 2>&1 | FileCheck %s

; CMPLRLLVM-27767: Makes sure InstCombine doesn't do the following
; transformation when both PrepareForLTO and EnableDTrans are true.
; ((X - Y) - Z)  -->  X - (Y + Z)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: %d = sub i64 %p1, %p2
; CHECK: %r = sub i64 %d, %z
; CHECK-NOT: = add

define i64 @t0(i8* %x, i8* %y, i64 %z) {
  %p1 = ptrtoint i8* %x to i64
  %p2 = ptrtoint i8* %y to i64
  %d = sub i64 %p1, %p2
  %r = sub i64 %d, %z
  ret i64 %r
}
; end INTEL_FEATURE_SW_ADVANCED
