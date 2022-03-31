; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE

; Verify that even in scoped mode we do not propagate nsw flag to SCEV of %add
; because they are invalid for %add2 which has identical SCEV.

; The constant folding logic recognizes that the addition of constants 1 and
; 127 (in %add1 and %add2 respectively) overflows signed range and gives up.



; SCOPED-MODE: %add = add nsw i8 %n, -128
; SCOPED-MODE:   -->  (-128 + %n) U: full-set S: full-set
; SCOPED-MODE: %add2 = add nsw i8 %add1, 127
; SCOPED-MODE:   -->  (-128 + %n) U: full-set S: full-set


define void @foo(i8 %n) "intel-lang"="fortran" {
entry:
  %cmp = icmp sgt i8 %n, 0
  br i1 %cmp, label %bb1, label %bb2

bb1:
  %add = add nsw i8 %n, -128
  ret void

bb2:
  %add1 = add nsw i8 %n, 1
  %add2 = add nsw i8 %add1, 127
  ret void
}


