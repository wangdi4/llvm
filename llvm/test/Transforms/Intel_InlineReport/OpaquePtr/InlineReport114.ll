; Classic inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Test that if inlining a recursive function removes a recursive call, that
; the inlining report is produced without dying.

; CHECK: COMPILE FUNC: test_recursive_inlining_remapping
; CHECK: INLINE: test_recursive_inlining_remapping
; CHECK: test_recursive_inlining_remapping

define i32 @test_recursive_inlining_remapping(i1 %init, ptr %addr) {
bb:
  %n = alloca i32, align 4
  br i1 %init, label %store, label %load

store:                                            ; preds = %bb
  store i32 0, ptr %n, align 4
  %v = call i32 @test_recursive_inlining_remapping(i1 false, ptr %n)
  ret i32 %v

load:                                             ; preds = %bb
  %n.load = load i32, ptr %addr, align 4
  ret i32 %n.load
}
