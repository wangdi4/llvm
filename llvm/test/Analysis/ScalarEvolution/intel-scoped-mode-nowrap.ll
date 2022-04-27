; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s
; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE
; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution -scalar-evolution-print-scoped-mode -scalar-evolution-nowrap-user-tracking-threshold=1 | FileCheck %s --check-prefix=SCOPED-MODE-THRESHOLD
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode -scalar-evolution-nowrap-user-tracking-threshold=1 "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE-THRESHOLD

; Verify that in scoped mode we can propagate nsw flag to SCEV of %add by
; assuming that IR will not change during analysis. The nuw flag is removed
; because the other instruction %add1 with the same operands doesn't have it.

; %n's use in icmp is ignored which it is not considered SCEVable.
; %n's use in sdiv is deemed safe because the SCEV form of the
; instruction is SCEVUnknown so %n does not 'escape' through it.


; CHECK: -->  (3 + %n) U: full-set S: full-set

; SCOPED-MODE: %add = add nuw nsw i32 %n, 3
; SCOPED-MODE:   -->  (3 + %n)<nsw> U: [-2147483645,-2147483648) S: [-2147483645,-2147483648)
; SCOPED-MODE: %div = sdiv i32 %n, 8
; SCOPED-MODE:   -->  %div U: [-268435456,268435456) S: [-268435456,268435456)
; SCOPED-MODE: %add1 = add nsw i32 %n, 3
; SCOPED-MODE:   -->  (3 + %n)<nsw> U: [-2147483645,-2147483648) S: [-2147483645,-2147483648)

; Verify that partial adds which constant fold to the same addition are recognized by nowrap analysis.
; SCOPED-MODE: %partial.add1 = add nsw i32 %partial.add, 2
; SCOPED-MODE:   -->  (3 + %n)<nsw> U: [-2147483645,-2147483648) S: [-2147483645,-2147483648)


; Verify that the nowrap analysis gives up if the threshold is too low.

; SCOPED-MODE-THRESHOLD: %add = add nuw nsw i32 %n, 3
; SCOPED-MODE-THRESHOLD-NEXT: -->  (3 + %n) U: full-set S: full-set

define i32 @foo(i32 %n) "intel-lang"="fortran" {
entry:
  %cmp17 = icmp slt i32 %n, 5
  %add = add nsw nuw i32 %n, 3
  %div = sdiv i32 %n, 8
  %add1 = add nsw i32 %n, 3
  %partial.add = add nsw i32 %n, 1
  %partial.add1 = add nsw i32 %partial.add, 2
  ret i32 %add
}
