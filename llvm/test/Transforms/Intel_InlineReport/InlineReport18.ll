; Inline report
; RUN: opt -inline -inline-report=7 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-OLD %s
; RUN: opt -passes='cgscc(inline)' -inline-report=7 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=134 < %s -S | opt -inline -inline-report=134 -S | opt -inlinereportemitter -inline-report=134 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD
; RUN: opt -passes='inlinereportsetup' -inline-report=134 < %s -S | opt -passes='cgscc(inline)' -inline-report=134 -S | opt -passes='inlinereportemitter' -inline-report=134 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

; Check that the inline report reports that inlining is inhibited when a
; caller/callee pair has mismatched "null-pointer-is-valid" attributes,
; unless the callee also has the attribute "alwaysinline".

define i32 @null-pointer-is-valid_callee0(i32 %i) null_pointer_is_valid {
  ret i32 %i
}

define i32 @null-pointer-is-valid_callee1(i32 %i) alwaysinline null_pointer_is_valid {
  ret i32 %i
}

define i32 @test_null-pointer-is-valid0(i32 %i) {
  %1 = call i32 @null-pointer-is-valid_callee0(i32 %i)
  ret i32 %1
}

define i32 @test_null-pointer-is-valid1(i32 %i) {
  %1 = call i32 @null-pointer-is-valid_callee1(i32 %i)
  ret i32 %1
}

; Old pass manager checks
; CHECK-OLD: -> null-pointer-is-valid_callee0 {{\[\[}}Caller/callee null pointer mismatch{{\]\]}}
; CHECK-OLD: -> INLINE: null-pointer-is-valid_callee1 <<Callee is always inline>>

; New pass manager checks
; CHECK-NEW: -> INLINE: null-pointer-is-valid_callee1 <<Callee is always inline>>
; CHECK-NEW: -> null-pointer-is-valid_callee0 {{\[\[}}Caller/callee null pointer mismatch{{\]\]}}

