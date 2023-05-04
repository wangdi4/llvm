; RUN: opt < %s -S -passes="globalopt" | FileCheck %s

@x = internal global [10 x i16] zeroinitializer, align 16
@w = internal global ptr null, align 8
@z = internal global ptr null, align 8

; CHECK-LABEL: @foo
; Check that GlobalOpt doesn't assert while trying to bitcast
; an i64 ptrtoint to ptr.
define void @foo() {
entry:
  store i64 ptrtoint (ptr @x to i64), ptr bitcast (ptr @z to ptr), align 8
  ret void
}

define void @bar() {
entry:
  %z = load ptr, ptr @z, align 8
  store ptr %z, ptr @w, align 8
  ret void
}
