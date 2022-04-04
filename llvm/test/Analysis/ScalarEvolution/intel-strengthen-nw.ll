; RUN: opt < %s -scalar-evolution -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Check that ScalarEvolution can determine that the entire addition tree does
; not signed wrap. E.g., (v0 + 7 + 13) summed in any order will not wrap.
; CHECK-LABEL: @nosignedwrap
; CHECK: %a1 =
; INTEL_CUSTOMIZATION
; refined from [16,24): srem is [-3,4) + 7 + 13 => [17,24)
; CHECK: -->  (20 + %v0)<nsw> U: [17,24) S: [17,24)
; end INTEL_CUSTOMIZATION
define i8 @nosignedwrap(i8 %v) {
        %v0 = srem i8 %v, 4
        %a0 = add i8 %v0, 7
        %a1 = add i8 %a0, 13
        ret i8 %a1
}

; This demonstrates a case where it is provable that these additions form a
; particular sum tree which can be shown to not wrap only if the flag ;
; strengthening happens after constants in the sum are folded.
; E.g, %a1 = ((%v0 + -4) + 127) doesn't wrap, but ((%v0 + 127) + -4) could, so
; the 3-way sum cannot have NSW. However, with constant folding we have a
; binary sum, (123 + %v0), and this can be shown to NSW.
; CHECK-LABEL: @foldednsw
; CHECK: %a1 =
; INTEL_CUSTOMIZATION
; refined from [119,127): srem is [-3,3] - 4 + 127 => [120,127)
; CHECK: --> (123 + %v0)<nsw> U: [120,127) S: [120,127)
; end INTEL_CUSTOMIZATION
define i8 @foldednsw(i8 %v) {
        %v0 = srem i8 %v, 4
        %a0 = add i8 %v0, -4
        %a1 = add i8 %a0, 127
        ret i8 %a1
}
