; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s
; RUN: opt < %s -analyze -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE

; Verify that in scoped mode we conservatively parse urem.

; CHECK: %rem = urem i32 %n, 255
; CHECK: -->  ((-255 * (%n /u 255)) + %n)

; SCOPED-MODE: %rem = urem i32 %n, 255
; SCOPED-MODE: -->  %rem

define i32 @foo(i32 %n) {
entry:
  %rem = urem i32 %n, 255
  ret i32 %rem
}
