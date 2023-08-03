; This test verifies that dereferenceable attribute of argument is utilized
; to promote arguments. %b, which is a pointer to struct, will be promoted to
; i64 and i32 arguments because %b is marked with "dereferenceable(16)"
; attribute.

; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; CHECK-LABEL: define {{[^@]+}}@f
; CHECK-SAME: (i64 [[B_0:%.*]], i32 [[B_1:%.*]], ptr %X, i32 %i)
; CHECK: b1:
; CHECK-NEXT: %tmp5 = trunc i64 [[B_0]] to i32
; CHECK-NEXT: %tmp4 = add i32 %tmp5, [[B_1]]
; CHECK-NEXT: store i32 %tmp4, ptr %X, align 4
; CHECK-NEXT: br label %b2

%struct.ss = type { i64, i32 }

define internal void @f(i64 %b.0.val, i32 %b.8.val, ptr %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

b1:                                               ; preds = %entry
  %tmp5 = trunc i64 %b.0.val to i32
  %tmp4 = add i32 %tmp5, %b.8.val
  store i32 %tmp4, ptr %X, align 4
  br label %b2

b2:                                               ; preds = %b1, %entry
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@test(ptr %X, ptr %S) {
; CHECK: store i32 2, ptr %tmp4, align 4
; CHECK-NEXT: [[S_1:%.*]] = load i64, ptr %S, align 8
; CHECK-NEXT: [[G_2:%.*]] = getelementptr i8, ptr %S, i64 8
; CHECK-NEXT: [[S_2:%.*]] = load i32, ptr [[G_2]], align 4
; CHECK-NEXT:  call void @f(i64 [[S_1]], i32 [[S_2]], ptr %X, i32 zeroext 0)
define i32 @test(ptr %X, ptr %S) {
entry:
  %tmp1 = getelementptr %struct.ss, ptr %S, i32 0, i32 0
  store i64 1, ptr %tmp1, align 8
  %tmp4 = getelementptr %struct.ss, ptr %S, i32 0, i32 1
  store i32 2, ptr %tmp4, align 4
  %S.val = load i64, ptr %S, align 8
  %0 = getelementptr i8, ptr %S, i64 8
  %S.val1 = load i32, ptr %0, align 4
  call void @f(i64 %S.val, i32 %S.val1, ptr %X, i32 zeroext 0)
  ret i32 0
}
