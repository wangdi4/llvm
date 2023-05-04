; RUN: opt -opaque-pointers -passes=inline < %s -S -o - | FileCheck %s

; Both stores after inlining should have the same !alias.scope metadata.
; Also check that loads are !noalias with stores.

define void @foo(ptr "ptrnoalias" %a) {
bb:
  %p1 = load ptr, ptr %a, align 8
  store i32 1, ptr %p1, align 4
  %p2 = load ptr, ptr %a, align 8
  store i32 2, ptr %p2, align 4
  ret void
}

define void @bar(ptr %a) {
; CHECK-LABEL: @bar

; CHECK: load
; CHECK-SAME: !noalias !0
; CHECK: store
; CHECK-SAME: !alias.scope !0

; CHECK: load
; CHECK-SAME: !noalias !0
; CHECK: store
; CHECK-SAME: !alias.scope !0
bb:
  call void @foo(ptr %a)
  ret void
}
