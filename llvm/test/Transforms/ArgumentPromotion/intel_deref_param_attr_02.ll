; This test verifies that dereferenceable attribute of argument is utilized
; to promote arguments. %b, which is a pointer to struct, will be promoted to
; i64 and i32 arguments because %b is marked with "dereferenceable(16)"
; attribute.

; RUN: opt < %s -argpromotion -S | FileCheck %s
; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

%struct.ss = type { i64, i32 }

; CHECK-LABEL: define {{[^@]+}}@f
; CHECK-SAME: (i64 [[B_0:%.*]], i32 [[B_1:%.*]], i32* %X, i32 %i)
; CHECK: b1:
; CHECK-NEXT: %tmp5 = trunc i64 [[B_0]] to i32
; CHECK-NEXT: %tmp4 = add i32 %tmp5, [[B_1]]
; CHECK-NEXT: store i32 %tmp4, i32* %X, align 4
; CHECK-NEXT: br label %b2

define internal void @f(%struct.ss* align 8 dereferenceable(16) %b, i32* %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i64, i64* %tmp, align 8
  %tmp2 = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 1
  %tmp3 = load i32, i32* %tmp2, align 4
  %tmp5 = trunc i64 %tmp1 to i32
  %tmp4 = add i32 %tmp5, %tmp3
  store i32 %tmp4, i32* %X, align 4
  br label %b2

b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@test(i32* %X, %struct.ss* %S) {
; CHECK: store i32 2, i32* %tmp4, align 4
; CHECK-NEXT: [[G_1:%.*]] = getelementptr %struct.ss, %struct.ss* %S, i64 0, i32 0
; CHECK-NEXT: [[S_1:%.*]] = load i64, i64* [[G_1]], align 8
; CHECK-NEXT: [[G_2:%.*]] = getelementptr %struct.ss, %struct.ss* %S, i64 0, i32 1
; CHECK-NEXT: [[S_2:%.*]] = load i32, i32* [[G_2]], align 4
; CHECK-NEXT:  call void @f(i64 [[S_1]], i32 [[S_2]], i32* %X, i32 zeroext 0)
define i32 @test(i32* %X, %struct.ss* %S) {
entry:
  %tmp1 = getelementptr %struct.ss, %struct.ss* %S, i32 0, i32 0
  store i64 1, i64* %tmp1, align 8
  %tmp4 = getelementptr %struct.ss, %struct.ss* %S, i32 0, i32 1
  store i32 2, i32* %tmp4, align 4

  call void @f(%struct.ss* %S, i32* %X, i32 zeroext 0)
  ret i32 0
}
