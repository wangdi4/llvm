; RUN: opt < %s -scalar-evolution -analyze | FileCheck %s

; Check that ScalarEvolution can determine that the entire addition tree does
; not signed wrap. E.g., (v0 + 7 + 13) summed in any order will not wrap.
; CHECK-LABEL: @nosignedwrap
; CHECK: %a1 =
; CHECK: -->  (20 + %v0)<nsw> U: [-108,-128) S: [16,24)
define i8 @nosignedwrap(i8 %v) {
        %v0 = srem i8 %v, 4
        %a0 = add i8 %v0, 7
        %a1 = add i8 %a0, 13
        ret i8 %a1
}

; This demonstrates a case where it is provable that these additions form a
; particular sum tree which can't sign wrap, but ScalarEvolution doesn't mark
; the sum as no-wrap because another sum tree with the same operands could
; wrap. E.g, ((%v0 + -4) + 127) doesn't wrap, but ((%v0 + 127) + -4) could.
; CHECK-LABEL: @falsewrap
; CHECK: %a1 =
; CHECK: --> (123 + %v0) U: full-set S: [119,127)
define i8 @falsewrap(i8 %v) {
        %v0 = srem i8 %v, 4
        %a0 = add i8 %v0, -4
        %a1 = add i8 %a0, 127
        ret i8 %a1
}
