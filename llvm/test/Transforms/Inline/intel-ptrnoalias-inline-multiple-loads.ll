; RUN: opt -passes=inline < %s -S -o - | FileCheck %s

; Both stores after inlining should have the same !alias.scope metadata.
; Also check that loads are !noalias with stores.

define void @foo(i32** "ptrnoalias" %a) {
  %p1 = load i32*, i32** %a, align 8
  store i32 1, i32* %p1
  %p2 = load i32*, i32** %a, align 8
  store i32 2, i32* %p2
  ret void
}

define void @bar(i32** %a) {
; CHECK-LABEL: @bar

; CHECK: load
; CHECK-SAME: !noalias !0
; CHECK: store
; CHECK-SAME: !alias.scope !0

; CHECK: load
; CHECK-SAME: !noalias !0
; CHECK: store
; CHECK-SAME: !alias.scope !0

  call void @foo(i32** %a)
  ret void
}

