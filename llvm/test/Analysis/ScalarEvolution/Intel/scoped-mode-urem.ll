; RUN: opt < %s -analyze -bugpoint-enable-legacy-pm -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s
; RUN: opt < %s -analyze -bugpoint-enable-legacy-pm -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE

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
