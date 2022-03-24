; This test verifies that dereferenceable attribute of argument is utilized
; to promote arguments. %b will be promoted because the argument is marked
; with "dereferenceable(8)" attribute. %c will not be promoted because
; it is not marked with dereferenceable.

; RUN: opt < %s -argpromotion -S | FileCheck %s
; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; CHECK-LABEL: define {{[^@]+}}@f
; CHECK-SAME: (i64 [[B_0:%.*]], i32* %X, i32 %i)
; CHECK: b1:
; CHECK: %tmp2 = add i64 [[B_0]], 1
; CHECK: store i32 0, i32* %X, align 4
; CHECK: br label %b2
define internal void @f(i64* align 8 dereferenceable(8) %b, i32* %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp1 = load i64, i64* %b, align 8
  %tmp2 = add i64 %tmp1, 1
  store i32 0, i32* %X
  br label %b2

 b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@g
; CHECK-SAME: (i64* %c, i32* %X, i32 %i)
; CHECK: b1:
; CHECK: %tmp1 = load i64, i64* %c, align 8
; CHECK: %tmp2 = add i64 %tmp1, 1
define internal void @g(i64* %c, i32* %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp1 = load i64, i64* %c, align 8
  %tmp2 = add i64 %tmp1, 1
  store i32 0, i32* %X
  br label %b2

 b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@test(i32* %X, i64* %S) {
; CHECK: store i64 1, i64* %S, align 8
; CHECK: [[S_1_VAL:%.*]] = load i64, i64* %S, align 8
; CHECK: call void @f(i64 [[S_1_VAL]], i32* %X, i32 zeroext 0)
; CHECK: call void @g(i64* %S, i32* %X, i32 zeroext 0)
define i32 @test(i32* %X, i64* %S) {
entry:
  store i64 1, i64* %S, align 8
  call void @f(i64* %S, i32* %X, i32 zeroext 0)
  call void @g(i64* %S, i32* %X, i32 zeroext 0)
  ret i32 0
}
