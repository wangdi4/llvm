; This test verifies that ArgumentPromotion doesn't depend on AA
; when an argument is marked with noalias. %p1 argument is promoted
; in "fn" function by ignoring AA since the argument is marked as
; noalias. %p2 argument is not promoted as it is not marked as noalias.

; RUN: opt -opaque-pointers=0 < %s -passes=argpromotion -S | FileCheck %s

@a = global i32 0, align 4
@b = global i32 0, align 4
@c = global i32 0, align 4
@d = global i32 0, align 4
@g = global i32 0, align 4

; CHECK-LABEL: define {{[^@]+}}@fn
; CHECK-SAME: (i32 [[P1_VAL:%.*]], i32* %p2)
; CHECK-NEXT:  entry:
; CHECK-NEXT: store i32 2, i32* @g, align 4
; CHECK-NEXT: %ld2 = load i32, i32* %p2, align 4
; CHECK-NEXT: store i32 [[P1_VAL]], i32* @c, align 4
; CHECK-NEXT: store i32 %ld2, i32* @b, align 4
; CHECK-NEXT:  ret void
define internal fastcc void @fn(i32* noalias %p1, i32* %p2) {
entry:
  store i32 2, i32* @g, align 4
  %ld1 = load i32, i32* %p1, align 4
  %ld2 = load i32, i32* %p2, align 4
  store i32 %ld1, i32* @c, align 4
  store i32 %ld2, i32* @b, align 4
  ret void
}

; CHECK-LABEL: define {{[^@]+}}@main()
; CHECK-NEXT:  entry:
; CHECK-NEXT:  [[D_VAL:%.*]] = load i32, i32* @d, align 4
; CHECK-NEXT:  call fastcc void @fn(i32 [[D_VAL]], i32* @a)
; CHECK-NEXT:  ret i32 0

define i32 @main() {
entry:
  call fastcc void @fn(i32* @d, i32* @a)
  ret i32 0
}
