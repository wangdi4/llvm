; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s
; RUN: opt < %s -analyze -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE

; Verify that in scoped mode we can propagate nowrap flags to SCEV of %add by
; assuming that IR will not change during analysis.

; %n is only used in one SCEVable instruction(icmp is not counted) which makes
; it safe to propagate the no-wrap flags.


; CHECK: -->  (1 + %n) U: full-set S: full-set

; SCOPED-MODE: -->  (1 + %n)<nuw><nsw> U: [1,0) S: [-2147483647,-2147483648)


define i32 @foo(i32 %n) "intel-lang"="fortran" {
entry:
  %cmp17 = icmp slt i32 %n, 5
  %add = add nsw nuw i32 %n, 1
  ret i32 %add
}
