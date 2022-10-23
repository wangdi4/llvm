; Classic inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Test that if inlining a recursive function removes a recursive call, that
; the inlining report is produced without dying.

; CHECK: COMPILE FUNC: test_recursive_inlining_remapping
; CHECK: INLINE: test_recursive_inlining_remapping
; CHECK: test_recursive_inlining_remapping

define i32 @test_recursive_inlining_remapping(i1 %init, i8* %addr) {
bb:
  %n = alloca i32
  br i1 %init, label %store, label %load
store:
  store i32 0, i32* %n
  %cast = bitcast i32* %n to i8*
  %v = call i32 @test_recursive_inlining_remapping(i1 false, i8* %cast)
  ret i32 %v
load:
  %castback = bitcast i8* %addr to i32*
  %n.load = load i32, i32* %castback
  ret i32 %n.load
}
