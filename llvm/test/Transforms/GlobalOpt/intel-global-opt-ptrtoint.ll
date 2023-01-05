; RUN: opt < %s -S -passes="globalopt" | FileCheck %s

@x = internal global [10 x i16] zeroinitializer, align 16
@w = internal global i16* null, align 8
@z = internal global i16* null, align 8

; CHECK-LABEL: @foo
; Check that GlobalOpt doesn't assert while trying to bitcast
; an i64 ptrtoint to i16*.
define void @foo() {
entry:
  store i64 ptrtoint ([10 x i16]* @x to i64), i64* bitcast (i16** @z to i64*), align 8
  ret void
}

define void @bar() {
entry:
  %z = load i16*, i16** @z, align 8
  store i16* %z, i16** @w, align 8
  ret void
}
