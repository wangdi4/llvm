; RUN: opt < %s -analyze -bugpoint-enable-legacy-pm -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s
; RUN: opt < %s -analyze -bugpoint-enable-legacy-pm -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE

; Verify that in scoped mode we conservatively parse 'and i1' as SCEVUnknown.

; CHECK: %a = and i1 %n, %m
; CHECK: -->  (%n umin %m)

; SCOPED-MODE: %a = and i1 %n, %m
; SCOPED-MODE: -->  %a

define i1 @foo(i1 %n, i1 %m) {
entry:
  %a = and i1 %n, %m
  ret i1 %a
}
