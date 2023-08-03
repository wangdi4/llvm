; This test verifies that dereferenceable_or_null or dereferenceable with
; insufficient reference byte attributes will not help to promote arguments.
; %b won't be promoted because it is marked with dereferenceable_or_null.
; %c won't be promoted because it is marked with dereferenceable(8) attribute
; that confirms only the 8 bytes (out of 16) are dereferenceable.

; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

%struct.ss = type { i64, i32 }

; CHECK-LABEL: define {{[^@]+}}@f
; CHECK-SAME: (ptr align 8 dereferenceable_or_null(16) %b, ptr %X, i32 %i)
define internal void @f(ptr align 8 dereferenceable_or_null(16) %b, ptr %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i64, ptr %tmp, align 8
  %tmp2 = getelementptr %struct.ss, ptr %b, i32 0, i32 1
  %tmp3 = load i32, ptr %tmp2, align 4
  %tmp5 = trunc i64 %tmp1 to i32
  %tmp4 = add i32 %tmp5, %tmp3
  store i32 %tmp4, ptr %X, align 4
  br label %b2

b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@g
; CHECK-SAME: (ptr dereferenceable(8) %c, ptr %X, i32 %i)
define internal void @g(ptr dereferenceable(8) %c, ptr %X, i32 %i) {
entry:
  br i1 undef, label %b1, label %b2

 b1:
  %tmp = getelementptr %struct.ss, ptr %c, i32 0, i32 0
  %tmp1 = load i64, ptr %tmp, align 8
  %tmp2 = getelementptr %struct.ss, ptr %c, i32 0, i32 1
  %tmp3 = load i32, ptr %tmp2, align 4
  %tmp5 = trunc i64 %tmp1 to i32
  %tmp4 = add i32 %tmp5, %tmp3
  store i32 %tmp4, ptr %X, align 4
  br label %b2

b2:
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@test(ptr %X, ptr %S)
; CHECK: call void @f(ptr %S, ptr %X, i32 zeroext 0)
; CHECK: call void @g(ptr %S, ptr %X, i32 zeroext 0)
define i32 @test(ptr %X, ptr %S) {
entry:
  %tmp1 = getelementptr %struct.ss, ptr %S, i32 0, i32 0
  store i64 1, ptr %tmp1, align 8
  %tmp4 = getelementptr %struct.ss, ptr %S, i32 0, i32 1
  store i32 2, ptr %tmp4, align 4

  call void @f(ptr %S, ptr %X, i32 zeroext 0)
  call void @g(ptr %S, ptr %X, i32 zeroext 0)

  ret i32 0
}
