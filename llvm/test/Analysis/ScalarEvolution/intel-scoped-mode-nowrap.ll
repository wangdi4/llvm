; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s
; RUN: opt < %s -analyze -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE

; Verify that in scoped mode we can propagate nsw flag to SCEV of %add by
; assuming that IR will not change during analysis. The nuw flag is removed 
; because the other instruction %add1 with the same operands doesn't have it.

; %n's use in icmp is ignored which it is not considered SCEVable.
; %n's use in sdiv is deemed safe because the SCEV form of the
; instruction is SCEVUnknown so %n does not 'escape' through it.


; CHECK: -->  (1 + %n) U: full-set S: full-set

; SCOPED-MODE: %add = add nuw nsw i32 %n, 1
; SCOPED-MODE:   -->  (1 + %n)<nsw> U: [-2147483647,-2147483648) S: [-2147483647,-2147483648)
; SCOPED-MODE: %div = sdiv i32 %n, 8
; SCOPED-MODE:   -->  %div U: full-set S: [-268435456,268435456)
; SCOPED-MODE: %add1 = add nsw i32 %n, 1
; SCOPED-MODE:   -->  (1 + %n)<nsw> U: [-2147483647,-2147483648) S: [-2147483647,-2147483648)

define i32 @foo(i32 %n) "intel-lang"="fortran" {
entry:
  %cmp17 = icmp slt i32 %n, 5
  %add = add nsw nuw i32 %n, 1
  %div = sdiv i32 %n, 8
  %add1 = add nsw i32 %n, 1
  ret i32 %add
}
