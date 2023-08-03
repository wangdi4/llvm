; Don't promote arguments if any call is marked with "ippredopt-callsite"
; attribute.

; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

%T = type { i32, i32, i32, i32 }
@G = constant %T { i32 0, i32 0, i32 17, i32 25 }

define internal i32 @test(ptr %p) {
; CHECK-LABEL: define {{[^@]+}}@test(ptr %p) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %a.gep = getelementptr %T, ptr %p, i64 0, i32 3
; CHECK-NEXT:    %b.gep = getelementptr %T, ptr %p, i64 0, i32 2
; CHECK-NEXT:    %a = load i32, ptr %a.gep
; CHECK-NEXT:    %b = load i32, ptr %b.gep
; CHECK-NEXT:    %v = add i32 %a, %b
; CHECK-NEXT:    ret i32 %v
;
entry:
  %a.gep = getelementptr %T, ptr %p, i64 0, i32 3
  %b.gep = getelementptr %T, ptr %p, i64 0, i32 2
  %a = load i32, ptr %a.gep
  %b = load i32, ptr %b.gep
  %v = add i32 %a, %b
  ret i32 %v
}

define i32 @caller() {
; CHECK-LABEL: define {{[^@]+}}@caller() {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %v = call i32 @test(ptr @G)
; CHECK-NEXT:    ret i32 %v
;
entry:
  %v = call i32 @test(ptr @G) #0
  ret i32 %v
}

attributes #0 = { "ippredopt-callsite" }
