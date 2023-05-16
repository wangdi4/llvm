; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that presence of 'live.range.de.ssa' metadata on %shl prevents %shr
; from being parsed in terms of %in as it suppresses traceback.

; Absence of metadata allows traceback so %shr is parsed in terms of %in.

; CHECK-LABEL: range
; CHECK: %shr = ashr exact i64 %shl, 29
; CHECK-NEXT:  -->  ((((-1 * %shl) smax %shl) /u 536870912) * (1 smin (-1 smax %shl)))<nsw>


; CHECK-LABEL: no_range
; CHECK: %shr = ashr exact i64 %shl, 29
; CHECK-NEXT:  -->  (sext i35 (8 * (trunc i64 %in to i35)) to i64)


define i64 @range(i64 %in) {
entry:
  %shl = shl i64 %in, 32, !live.range.de.ssa !0
  %shr = ashr exact i64 %shl, 29
  ret i64 %shr
}

define i64 @no_range(i64 %in) {
entry:
  %shl = shl i64 %in, 32
  %shr = ashr exact i64 %shl, 29
  ret i64 %shr
}

!0 = !{}
