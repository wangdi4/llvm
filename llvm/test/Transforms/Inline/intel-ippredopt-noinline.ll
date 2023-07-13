; INTEL_FEATURE_SW_ADVANCED

; This test verifies that calls to simpleFunction are not inlined
; since those calls are marked with "ippredopt-callsite".

; RUN: opt < %s -passes=inline -S | FileCheck %s

define i32 @simpleFunction(i32 %a) {
entry:
  %add = add i32 %a, %a
  ret i32 %add
}

define i32 @bar(i32 %a) {
; CHECK-LABEL: @bar
; CHECK: call i32 @simpleFunction(i32 5)
; CHECK: call i32 @simpleFunction(i32 6)
entry:
  %i0 = tail call i32 @simpleFunction(i32 5) #0
  %i1 = tail call i32 @simpleFunction(i32 6) #0
  %add = add i32 %i0, %i1
  ret i32 %add
}

attributes #0 = { "ippredopt-callsite" }
; end INTEL_FEATURE_SW_ADVANCED
