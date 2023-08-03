; This test verifies that dereferenceable attribute of argument is utilized
; to promote arguments. %b will be promoted because the argument is marked
; with "dereferenceable(8)" attribute. %c will not be promoted because
; it is not marked with dereferenceable.

; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; CHECK-LABEL: define {{[^@]+}}@f
; CHECK-SAME: (i64 [[B_0:%.*]], ptr %X, i32 %i)
; CHECK: b1:
; CHECK: %tmp2 = add i64 [[B_0]], 1
; CHECK: store i32 0, ptr %X, align 4
; CHECK: br label %b2
define internal void @f(ptr align 8 dereferenceable(8) %b, ptr %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp1 = load i64, ptr %b, align 8
  %tmp2 = add i64 %tmp1, 1
  store i32 0, ptr %X
  br label %b2

 b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@g
; CHECK-SAME: (ptr %c, ptr %X, i32 %i)
; CHECK: b1:
; CHECK: %tmp1 = load i64, ptr %c, align 8
; CHECK: %tmp2 = add i64 %tmp1, 1
define internal void @g(ptr %c, ptr %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp1 = load i64, ptr %c, align 8
  %tmp2 = add i64 %tmp1, 1
  store i32 0, ptr %X
  br label %b2

 b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@test(ptr %X, ptr %S) {
; CHECK: store i64 1, ptr %S, align 8
; CHECK: [[S_1_VAL:%.*]] = load i64, ptr %S, align 8
; CHECK: call void @f(i64 [[S_1_VAL]], ptr %X, i32 zeroext 0)
; CHECK: call void @g(ptr %S, ptr %X, i32 zeroext 0)
define i32 @test(ptr %X, ptr %S) {
entry:
  store i64 1, ptr %S, align 8
  call void @f(ptr %S, ptr %X, i32 zeroext 0)
  call void @g(ptr %S, ptr %X, i32 zeroext 0)
  ret i32 0
}
